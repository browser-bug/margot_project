# library(crs, quietly = TRUE)    # to get the response surface models
suppressMessages(suppressPackageStartupMessages(library("RJDBC")))  # connect to database using JDBC codecs
suppressMessages(suppressPackageStartupMessages(library("mda")))
suppressMessages(suppressPackageStartupMessages(library("polspline")))
suppressMessages(suppressPackageStartupMessages(library("dplyr")))
suppressMessages(suppressPackageStartupMessages(library("quadprog")))

options(scipen = 100)
map_to_input = TRUE

#######################################################################
############ GENERAL CONFIGURATION SECTION OF THE SCRIPT ##############
#######################################################################
# This part of the application describes the geometry of an Operating
# Point and configures the generation of the model.
#
# OUTPUT:
# ---------------------------------------------------------------------
# knobs :                 Vector with all the names of the software knobs
# features :              Vector with all the names of the input features
# target_meetric:         The name of the metric to predict
# correlation_threshold : Numeric value which define the usefulness of a
#                         predictor, wrt the target metric (default 0.2)
#
# NOTE:
# ---------------------------------------------------------------------
# The target metric and the correlation_threshold are taken in input as
# program option of the R script



# get the command arguments to figure out which is the metric to predict
args = commandArgs(trailingOnly=TRUE)
if (length(args) < 10)
{
  stop("Error: Number of input parameters is less than 10 (Please input the storage_type, storage_address, application_name, metric_name and root_path)", call.=FALSE)
} else if (length(args) == 10)
{
  storage_type <- args[1]
  storage_address <- args[2]
  application_name <- args[3]
  metric_name <- args[4]
  root_path <- args[5]
  iteration <- args[6]
  algorithm <- args[7]
  doe_eps <- as.numeric(args[8])
  doe_obs_per_dim <- as.numeric(args[9])
  doe_obs_per_point <- as.numeric(args[10])
} else
{
  storage_type <- args[1 ]
  storage_address <- args[2]
  application_name <- args[3]
  metric_name <- args[4]
  root_path <- args[5]
  iteration <- args[6]
  algorithm <- args[7]
  doe_eps <- as.numeric(args[8])
  doe_obs_per_dim <- as.numeric(args[9])
  doe_obs_per_point <- as.numeric(args[10])
  print(paste("Warning: the following program option are ignored:", args[11:nrow(args)], collapse = ", "))
}

print(paste("Started plugin", metric_name))

setwd(root_path)
source("create_discrete_doe.R")
source("get_knobs_config_list.R")
source("fit_models_agora_cor.R")

application_name <- gsub("/", "_", application_name)

knobs_container_name <- paste("margot.", application_name, "_knobs", sep = "")
features_container_name <- paste("margot.", application_name, "_features", sep = "")
observation_container_name <- paste("margot.", application_name, "_trace", sep = "")
model_container_name <- paste("margot.", application_name, "_model", sep = "")
doe_container_name <- paste("margot.", application_name, "_doe", sep = "")

# connect to the database -----------------------------------------------

if(storage_type == "CASSANDRA")
{
  driver <- JDBC("com.github.adejanovski.cassandra.jdbc.CassandraDriver", "/home/martinovic/SynologyDrive/IT4I/margot/java/cassandra-jdbc-wrapper-3.1.0.jar", identifier.quote = "'")
  full_address_string <- paste("jdbc:cassandra://", storage_address, ":9042", sep = "")
  conn <- dbConnect(driver, full_address_string)
} else
{
  stop(paste("Error: uknown $STORAGE_TYPE ", storage_type, ". Please, select $STORAGE_TYPE=CASSANDRA.", sep = ""), call.=FALSE)
}

# read the remainder of the configuration from cassandra -------------------------------------
knobs_names <- dbGetQuery(conn, paste("SELECT name FROM ", knobs_container_name, sep = ""))
features_names <- dbGetQuery(conn, paste("SELECT name FROM ", features_container_name, sep = ""))

nknobs <- dim(knobs_names)[1]

if(nknobs == 0)
{
  stop("Error: no knobs found. Please specify the knobs.")
}

print(paste("Number of KNOBS: ", nknobs) )

if(dim(features_names)[1] == 0)
{
  features_names <- NULL
}

print(paste("Number of FEATURES: ", dim(features_names)[1]) )

# predictor_names <- rbind(predictor_names, features)

# make sure that everything is in lowercase
knobs_names <- sapply( knobs_names , tolower )

#######################################################################
#################### INPUT PART OF THE SCRIPT #########################
#######################################################################
# This part of the application is in charge of retrieving the data
# of the profiled application to generate the model
#
# OUTPUT:
# ---------------------------------------------------------------------
# profiling_df : A dataframe filled with the porfile data, each row
#                represents a run of the application (except for the
#                first one, which is the header of the table); each
#                column represents a field of the Operating Point.
#                NOTE: The column must be named and related to the OP
#                      geometry defined later, e.g. the knob names
# prediction_df : A dataframe filled with the requests of prediction,
#                 each row represents an Operating Point to be predicted
#                 (except for the first one, which is the header of the
#                 table); each column represents the target software
#                 knobs and input features to predict

# get filter for columns in the dse table

dse_columns <- c(knobs_names, features_names, metric_name)
input_columns <- c(knobs_names, features_names)
# Check if there are any results already

# load the profile information from the dse
observation_df <- dbGetQuery(conn, paste("SELECT ", paste(dse_columns, collapse = ","), " FROM ", observation_container_name, sep = ""))

# get the boolean vector of complete rows (no NAs)
ind_complete <- complete.cases(observation_df)

# discard incomplete cases if any
if(any(!ind_complete))
{
  warning("Some observations are incomplete and are discarded for the model learning.")
  print("I discarded following observations: ")
  print(observation_df[!ind_complete,])
  observation_df <- observation_df[ind_complete,]
}

# BEWARE does not account for the features !!!!!!
# Get the grid configuration
knobs_config_list <- get_knobs_config_list(conn, knobs_container_name)
if(map_to_input == FALSE)
{
  knobs_config_list <- lapply(knobs_config_list, function(x)seq(min(x), max(x),by = 0.1))
}
# Transform information from the knobs_config_list into matrix
knob_transform <- sapply(knobs_config_list,
                         function(knob_config)
                         {
                           knob_adjusted_max <- max(knob_config) - min(knob_config)
                           knob_min <- min(knob_config)
                           return(c(knob_min, knob_adjusted_max))
                         })

# Create matrix of all the possible knob design points
design_space_grid <- expand.grid(knobs_config_list)

# design_space_grid <- as.data.frame(design_space_grid)
colnames(design_space_grid) <- knobs_names

if(map_to_input == TRUE)
{
  nobserved_orig <- dim(observation_df)[1]
  # Discard observation which are not on the grid
  observation_df <- inner_join(observation_df, design_space_grid, by = input_columns)
  if(nobserved_orig > dim(observation_df)[1])
  {
    warning("There were some observation out of the design grid and were discarded.")
  }
} else
{
  for(i in length(knobs_config_list))
  {
    observation_df <- observation_df[observation_df[, i] > min(knobs_config_list[[i]]) & observation_df[, i] < max(knobs_config_list[[i]]), ]
  }
}

# Set nobserved the number of observation in the obsercation_df
nobserved <- dim(observation_df)[1]

print(paste("Number of observations in the TRACE table: ", nobserved ) )

if(nobserved == 0)
{
  # Need to add this options to margot/agora ...
  # Set the minimal number of knobs to be 10 for each dimension
  doe_options <- list(nobs = nknobs * doe_obs_per_dim, eps = doe_eps)

  # Propose DOE design points
  doe_design <-  create_doe(knobs_config_list, doe_options, map_to_input, algorithm = algorithm)
  doe_names <- c(knobs_names, "counter")

  # Add values for counter column
  if(is.null(doe_design))
  {
    doe_design <- matrix(c(doe_design, rep(doe_obs_per_point, length(doe_design))), ncol = 2)
  } else
  {
    doe_design <- cbind(doe_design, doe_obs_per_point)
  }

  # Write DOE into database
  for(row.ind in 1:nrow(doe_design))
  {
    set_columns <- paste(doe_names, sep = "", collapse = ", ")
    set_values <- paste(doe_design[row.ind, ], sep = "", collapse = ", ")
    # set_statement <- paste(doe_names, " = ", doe_design[row.ind, ], sep = "", collapse = ", ")
    dbSendUpdate(conn, paste("INSERT INTO ", doe_container_name, "(", set_columns, ") VALUES (", set_values, ")", sep = ""))
  }

  # Quit to let the AGORA ask for the results at the specific points
  print("Wrote new DOE configurations")
  print(paste("Quiting plugin", metric_name))
  q(save = "no")
}


# Create models --------------------------------------------------------------------------------------------------------------------
print("I fit models now.")

# Permute observation_df should prevent same observation being in the test fold for cross validation
observation_df <- observation_df[sample(1:nobserved, nobserved), ]

validation <- fit_models_agora(observation_df, input_columns, metric_name, nobserved)

# Save R2 and MAE from the cross validation ----------------------------------------------------------------------------------------
model_selection_features <- c("R2", "MAE")
normalization_obs_df <- abs(diff(range(observation_df[, metric_name])))
if(normalization_obs_df == 0){stop("All the values of the response variables are constant. There might be a bug in Your model.")}

if( "R2" %in% model_selection_features )
{
  CV_R2 <- sapply(validation,
                    function(model)
                     {mean(model$R2[2:length(model$R2)], na.rm = TRUE)}
  )

  CV_R2[is.na(CV_R2)] <- Inf
}

if( "MAE" %in% model_selection_features )
{
  CV_MAE <- sapply(validation,
                    function(model)
                    {mean(model$MAE[2:length(model$MAE)])}
  )
  CV_MAE <- CV_MAE / normalization_obs_df
}

  # Do bagging -------------------------------------------------------------------------------
  # Go over all the models types (linear, mars, etc.)
  bagged_fit <- sapply(validation,
                       function(type)
                         {
                           # Go over all the model fits, model + cross-validation models and take their fit means
                           temp_fit <- sapply(type$models, function(temp_model)
                           {
                             model_type <- class(temp_model)[1]
                             switch(model_type,
                                    "lm" = {
                                      Y_hat <- predict(temp_model, observation_df[, input_columns])
                                    },
                                    "mars" = {
                                      Y_hat <- predict(temp_model, observation_df[, input_columns])
                                    },
                                    "polymars" = {
                                      Y_hat <- predict(temp_model, observation_df[, input_columns])
                                    },
                                    "km" = {
                                      Y_hat <- predict(temp_model, newdata = observation_df[, input_columns], type = "UK")$mean
                                    })
                             return(Y_hat)
                           })
                           return(rowMeans(temp_fit))
                         }
  )
  bagged_R2 <- c(cor(observation_df[, metric_name], bagged_fit)^2)
  bagged_MAE <- colMeans(apply(bagged_fit, 2, function(x){abs(x - matrix(observation_df[, metric_name], ncol = 1))}))
  bagged_MAE <- bagged_MAE / normalization_obs_df
 
  # Append bagged to the colnames, for the clear distinction between CV and bagged R2 and MAE
  names(bagged_R2) <- paste(names(CV_R2), "_bagged", sep = "") 
  names(bagged_MAE) <- paste(names(bagged_MAE), "_bagged", sep = "") 
  
  # Do stacking --------------------------------------------------------------------
  # get cross validation predictions
  stacking_data <- sapply(validation,
         function(type)
           {
           type$stacking_data
         })
  
  tryCatch({
  # Prepare data for quadratic optimization
  stacking_data <- unique(stacking_data)
  d_temp <- unique(observation_df)
  Rinv <- solve(chol(t(stacking_data) %*% stacking_data));
  C <- cbind(rep(1,length(validation)), diag(length(validation)))
  b <- c(1,rep(0,length(validation)))
  d <- t(d_temp[, metric_name]) %*% stacking_data
  # Do quadratic optimization and extract solution (weights for each model)
  stacking_weights <- solve.QP(Dmat = Rinv, factorized = TRUE, dvec = d, Amat = C, bvec = b, meq = 1)$solution
    },
  error = function(e) print(e))
  if(!exists("stacking_weights")){stacking_weights <- rep(1/length(validation), length(validation))}
  
  print(stacking_weights)
  model_fit_data <- sapply(validation,
                          function(type)
                          {
                            type$model_fit
                          })
  
  stacked_fit <- model_fit_data %*% stacking_weights
  
  stacked_R2 <- c(cor(observation_df[, metric_name], stacked_fit)^2)
  stacked_MAE <- colMeans(apply(stacked_fit, 2, function(x){abs(x - matrix(observation_df[, metric_name], ncol = 1))}))
  stacked_MAE <- stacked_MAE / normalization_obs_df
  
  # Name the R2 and MAE
  names(stacked_R2) <- "stacked_model"
  names(stacked_MAE) <- "stacked_model"
  
  # Append R2 and MAE for CV, bagged models and stacked model
  complete_R2 <- c(CV_R2, bagged_R2, stacked_R2)
  complete_MAE <- c(CV_MAE, bagged_MAE, stacked_MAE)
  
  # Check which models have acceptable R2 and MAE
  complete_R2_ind <- complete_R2 > 0.5
  complete_MAE_ind <- complete_MAE < 0.5
  
  model_selection <- rep(TRUE, length(complete_R2_ind))
  for(selection_feature in model_selection_features)
    switch(selection_feature,
           "R2" = {model_selection <- model_selection & complete_R2_ind},
           "MAE" = {model_selection <- model_selection & complete_MAE_ind})

  chosen_model <- which.min(complete_MAE[model_selection])

# Write the MAE and R2 to file models.log ----------------------------------------------------------------------------------------------------
# Check the iteration of the run
# Check if the models.log exists if not set iteration to 1, if yes read the file and set the iteration to the max iter + 1
if(file.exists("models.log"))
{
  old_models <- read.table(file = "models.log", header = TRUE, sep = ",", dec = ".")
  if(iteration == 1){
    run <- max(old_models$run) + 1
  } else {
    run <- max(old_models$run)
  }
} else
{
  run <- 1
  write("R2, MAE, model, iteration, run",
        file = "models.log")
}
  
models_info <- as.data.frame(t(rbind(complete_R2, complete_MAE, names(complete_R2))))
models_info$iteration <- iteration
models_info$run <- run
colnames(models_info) <- c("R2", "MAE", "model", "iteration, run")
write.table(models_info, file = "models.log", col.names = FALSE, row.names = FALSE, sep = ",", dec = ".", append = TRUE)

# If necessary increase the model space -------------------------------------------------------------------------------------------------------
if(length(chosen_model) == 0)
{
  if(grepl("factorial", algorithm))
  {
    algorithm <- "dmax"
  }
  # Get knobs config list
  knobs_config_list <- get_knobs_config_list(conn, knobs_container_name)

  print("Model is not good enough. I will ask for more points to explore.")
  # 0.2 could be substituted by parameter DOE update rate.
  updated_nobs <- dim(unique(observation_df[,input_columns]))[1] + 10
  doe_options <- list(nobs = updated_nobs, eps = doe_eps)

  # Propose DOE design points
  doe_design <-  create_doe(knobs_config_list, doe_options, map_to_input, algorithm = algorithm)
  doe_names <- c(knobs_names, "counter")

  # Add values for counter column
  if(is.null(doe_design))
  {
    doe_design <- matrix(c(doe_design, rep(doe_obs_per_point, length(doe_design))), ncol = 2)
  } else
  {
    doe_design <- cbind(doe_design, doe_obs_per_point)
  }

  for(row.ind in 1:nrow(doe_design))
  {
    set_columns <- paste(doe_names, sep = "", collapse = ", ")
    set_values <- paste(doe_design[row.ind, ], sep = "", collapse = ", ")
    # set_statement <- paste(doe_names, " = ", doe_design[row.ind, ], sep = "", collapse = ", ")
    dbSendUpdate(conn, paste("INSERT INTO ", doe_container_name, "(", set_columns, ") VALUES (", set_values, ")", sep = ""))
  }

  # Quit to let the AGORA ask for the results at the specific points
  print("Wrote new DOE configurations")
  print(paste("Quiting plugin", metric_name))
  q(save = "no")
}

# Write the model results for the grid created by the values set in the knobs definition ------------------------------------------------------

# Make predictions and write results ----------------------------------------------------------------------------------------------------------
print("I will make predictions now.")

Y_final <- list()

switch(names(chosen_model),
  "linear" = {
    Y_hat <- predict(validation$linear$models$full, design_space_grid, se.fit = TRUE, interval = "confidence")

    Y_final$mean <- Y_hat$fit[,1]
    Y_final$sd   <- Y_hat$se.fit 
  },
  "linear2" = {
    Y_hat <- predict(validation$linear2$models$full, design_space_grid, se.fit = TRUE, interval = "confidence")

      Y_final$mean <- Y_hat$fit[,1]
      Y_final$sd   <- Y_hat$se.fit
  },
  "mars" = {
    Y_hat <- predict(validation$mars$models$full, design_space_grid)

    Y_final$mean <- Y_hat
    Y_final$sd   <- -1
  },
  "polymars" = {
    Y_hat <- predict(validation$polymars$models$full, design_space_grid)

    Y_final$mean <- Y_hat
    Y_final$sd   <- -1
  },
  "kriging" = {
    Y_hat <- predict(validation$kriging$models$full, newdata = design_space_grid, type = "UK")

    Y_final$mean <- Y_hat$mean
    Y_final$sd   <- Y_hat$sd
  },
  "linear_bagged" = {
    chosen_model2 <- gsub("_bagged","", names(chosen_model))
    Y_hat <- sapply(validation[[chosen_model2]]$models, function(temp_model)
    {
      Y_hat <- predict(temp_model, design_space_grid)
      return(Y_hat)
    })
    
    Y_final$mean <- rowMeans(Y_hat)
    Y_final$sd   <- -1
  },
  "linear2_bagged" = {
    chosen_model2 <- gsub("_bagged","", names(chosen_model))
    Y_hat <- sapply(validation[[chosen_model2]]$models, function(temp_model)
    {
      Y_hat <- predict(temp_model, design_space_grid)
      return(Y_hat)
    })
    
    Y_final$mean <- rowMeans(Y_hat)
    Y_final$sd   <- -1
  },
  "mars_bagged" = {
    chosen_model2 <- gsub("_bagged","", names(chosen_model))
    Y_hat <- sapply(validation[[chosen_model2]]$models, function(temp_model)
    {
      Y_hat <- predict(temp_model, design_space_grid)
      return(Y_hat)
    })
    
    Y_final$mean <- rowMeans(Y_hat)
    Y_final$sd   <- -1
  },
  "polymars_bagged" = {
    chosen_model2 <- gsub("_bagged","", names(chosen_model))
    Y_hat <- sapply(validation[[chosen_model2]]$models, function(temp_model)
    {
      Y_hat <- predict(temp_model, design_space_grid)
      return(Y_hat)
    })
    
    Y_final$mean <- rowMeans(Y_hat)
    Y_final$sd   <- -1
  },
  "kriging_bagged" = {
    chosen_model2 <- gsub("_bagged","", names(chosen_model))
    Y_hat <- sapply(validation[[chosen_model2]]$models, function(temp_model)
    {
      Y_hat <- predict(temp_model, design_space_grid, type = "UK")$mean
      return(Y_hat)
    })
    
    Y_final$mean <- rowMeans(Y_hat)
    Y_final$sd   <- -1
  },
  "stacked_model" = {
    stacked_fit2 <- sapply(validation,
                         function(type)
                         {
                           # Go over all the model fits, model + cross-validation models and take their fit means
                           
                             model_type <- class(type$models$full)[1]
                             switch(model_type,
                                    "lm" = {
                                      Y_hat <- predict(type$models$full, design_space_grid)
                                    },
                                    "mars" = {
                                      Y_hat <- predict(type$models$full, design_space_grid)
                                    },
                                    "polymars" = {
                                      Y_hat <- predict(type$models$full, design_space_grid)
                                    },
                                    "km" = {
                                      Y_hat <- predict(type$models$full, newdata = design_space_grid, type = "UK")$mean
                                    })
                             return(Y_hat)
                         })
    Y_final$mean <- stacked_fit2 %*% stacking_weights
    Y_final$sd <- -1
  }
)

print("I will write results now")

# Write results of the model, for the whole grid
for(row.ind in 1:nrow(design_space_grid))
{
  set_columns <- paste(c(knobs_names, paste(metric_name, "_avg", sep = ""), paste(metric_name, "_std", sep = "")), sep = "", collapse = ", ")
  set_values <- paste(cbind(design_space_grid, Y_final$mean, Y_final$sd)[row.ind, ], sep = "", collapse = ", ")
  # set_statement <- paste(doe_names, " = ", doe_design[row.ind, ], sep = "", collapse = ", ")
  dbSendUpdate(conn, paste("INSERT INTO ", model_container_name, "(", set_columns, ") VALUES (", set_values, ")", sep = ""))
}

print(paste("Quiting plugin", metric_name))
q(save = "no")
