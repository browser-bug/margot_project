suppressMessages(suppressPackageStartupMessages(library("mda")))
suppressMessages(suppressPackageStartupMessages(library("polspline")))
suppressMessages(suppressPackageStartupMessages(library("dplyr")))
suppressMessages(suppressPackageStartupMessages(library("magrittr")))
suppressMessages(suppressPackageStartupMessages(library("quadprog")))

options(scipen = 100)
map_to_input = TRUE

######################## GET THE ARGUMENTS ############################

args = commandArgs(trailingOnly = TRUE)
if (length(args) < 10)
{
  stop("Error: Number of input parameters is less than 10 (Please input the storage_type, storage_address, application_name, metric_name and root_path)", call. = FALSE)
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
  print(paste("Warning: the following program option are ignored:", args[11:nrow(args)], collapse = ", "))
}

print(paste("Started plugin", metric_name))

######################## SET WORKSPACE PATH AND VARIABLES #######################

setwd(root_path)
source("create_discrete_doe.R")
source("get_knobs_config_list.R")
source("fit_models_agora_cor.R")

########################### LOAD DATA #######################################

if (storage_type == "CASSANDRA")
{
  suppressMessages(suppressPackageStartupMessages(library("RJDBC")))  # connect to database using JDBC codecs
  
  application_name <- gsub("/", "_", application_name)
  
  knobs_container_name <- paste("margot.", application_name, "_knobs", sep = "")
  features_container_name <- paste("margot.", application_name, "_features", sep = "")
  observation_container_name <- paste("margot.", application_name, "_trace", sep = "")
  model_container_name <- paste("margot.", application_name, "_model", sep = "")
  doe_container_name <- paste("margot.", application_name, "_doe", sep = "")
  
  driver <- JDBC("com.github.adejanovski.cassandra.jdbc.CassandraDriver", "cassandra-jdbc-wrapper-3.1.0.jar", identifier.quote = "'")
  full_address_string <- paste("jdbc:cassandra://", storage_address, ":9042", sep = "")
  conn <- dbConnect(driver, full_address_string)
  
  # READ CONFIGURATION FROM CASSANDRA
  knobs_names <- dbGetQuery(conn, paste("SELECT name FROM ", knobs_container_name, sep = ""))
  features_names <- dbGetQuery(conn, paste("SELECT name FROM ", features_container_name, sep = ""))
  
} else if (storage_type == "CSV")
{
  
} else
{
  stop(paste("Error: uknown $STORAGE_TYPE ", storage_type, ". Please, select $STORAGE_TYPE=CASSANDRA.", sep = ""), call. = FALSE)
}

################################# PREPARE DOE OPTIONS #################################

nknobs <- dim(knobs_names)[1]
if (nknobs == 0)
{
  stop("Error: no knobs found. Please specify the knobs.")
}
if (dim(features_names)[1] == 0)
{
  features_names <- NULL
}
cat("Number of KNOBS: ", nknobs, "\nNumber of FEATURES: ", dim(features_names)[1])

# MAKE NAMES LOWERCASE
knobs_names <- sapply(knobs_names, tolower)

# GET COLUMN NAMES OF DSE AND INPUT
dse_columns <- c(knobs_names, features_names, metric_name)
input_columns <- c(knobs_names, features_names)
# Check if there are any results already

# LOAD OBSERVATION DATA FRAME
if (storage_type == "CASSANDRA")
{
  observation_df <- dbGetQuery(conn, paste("SELECT ", paste(dse_columns, collapse = ","), " FROM ", observation_container_name, sep = ""))
} else if (storage_type == "CSV")
{
  
}

# GET BOOLEAN VECTOR OF ROWS WITHOUT NA VALUES
ind_complete <- complete.cases(observation_df)

# DISCARD ROWS WITH NA VALUES
if (any(!ind_complete))
{
  warning("Some observations are incomplete and are discarded for the model learning.")
  print("I discarded following observations: ")
  print(observation_df[!ind_complete, ])
  observation_df <- observation_df[ind_complete, ]
}

# BEWARE does not account for the features !!!!!!  GET GRID CONFIGURATION
knobs_config_list <- get_knobs_config_list(conn, knobs_container_name)
if (map_to_input == FALSE)
{
  knobs_config_list <- lapply(knobs_config_list, function(x) seq(min(x), max(x), by = 0.1))
}
# CREATE KNOB TRANSFORM LIST
knob_transform <- sapply(knobs_config_list, function(knob_config)
{
  knob_adjusted_max <- max(knob_config) - min(knob_config)
  knob_min <- min(knob_config)
  return(c(knob_min, knob_adjusted_max))
})

# CREATE DESIGN SPACE GRID
design_space_grid <- expand.grid(knobs_config_list)
colnames(design_space_grid) <- knobs_names

if (map_to_input == TRUE)
{
  nobserved_orig <- dim(observation_df)[1]
  # DISCARD OBSERVATION OUTSIDE THE DESIGN SPACE GRID
  observation_df <- inner_join(observation_df, design_space_grid, by = input_columns)
  if (nobserved_orig > dim(observation_df)[1])
  {
    warning("There were some observation out of the design grid and were discarded.")
  }
} else
{
  for (i in length(knobs_config_list))
  {
    observation_df <- observation_df[observation_df[, i] > min(knobs_config_list[[i]]) & observation_df[, i] < max(knobs_config_list[[i]]), ]
  }
}

observation_df <- unique(observation_df)
nobserved <- dim(observation_df)[1]

# STOP IF THERE ARE NO OBSERVATIONS
if (nobserved == 0)
{
  stop("No DOE results found!")
}

print(paste("Number of observations in the TRACE table: ", nobserved))

##################################### FIT MODELS ######################################################
print("I fit models now.")

# Permute observation_df should prevent same observation being in the test fold for cross validation
observation_df <- observation_df[sample(1:nobserved, nobserved), ]

validation <- fit_models_agora(observation_df, input_columns, metric_name, nobserved)

#################################### SET R2 AND MAE ###################################################
model_selection_features <- c("R2", "MAE")
normalization_obs_df <- abs(diff(range(observation_df[, metric_name])))
if (normalization_obs_df == 0)
{
  stop("All the values of the response variables are constant. There might be a bug in Your model.")
}

if ("R2" %in% model_selection_features)
{
  CV_R2 <- sapply(validation, function(model)
  {
    mean(model$R2[2:length(model$R2)], na.rm = TRUE)
  })
  
  CV_R2[is.na(CV_R2)] <- Inf
  
  # CV_R2_avg <- sapply(validation,
  #                 function(model)
  #                 {mean(model$R2_avg[2:length(model$R2_avg)], na.rm = TRUE)}
  # )
  # 
  # CV_R2_avg[is.na(CV_R2_avg)] <- Inf
}

if ("MAE" %in% model_selection_features)
{
  CV_MAE <- sapply(validation, function(model)
  {
    mean(model$MAE[2:length(model$MAE)])
  })
  CV_MAE <- CV_MAE/normalization_obs_df
}

############################### BAGGING ###################################

bagged_fit <- sapply(validation, function(type)
{
  # Go over all the model fits, model + cross-validation models and take their fit means
  temp_fit <- sapply(type$models, function(temp_model)
  {
    model_type <- class(temp_model)[1]
    switch(model_type, lm = {
      Y_hat <- predict(temp_model, observation_df[, input_columns])
    }, mars = {
      Y_hat <- predict(temp_model, observation_df[, input_columns])
    }, polymars = {
      Y_hat <- predict(temp_model, observation_df[, input_columns])
    }, km = {
      Y_hat <- predict(temp_model, newdata = observation_df[, input_columns], type = "UK")$mean
    })
    return(Y_hat)
  })
  return(rowMeans(temp_fit))
})
bagged_R2 <- c(cor(observation_df[, metric_name], bagged_fit)^2)
bagged_MAE <- colMeans(apply(bagged_fit, 2, function(x)
{
  abs(x - matrix(observation_df[, metric_name], ncol = 1))
}))
bagged_MAE <- bagged_MAE/normalization_obs_df

# Append bagged to the colnames, for the clear distinction between CV and bagged R2 and MAE
names(bagged_R2) <- paste(names(CV_R2), "_bagged", sep = "")
names(bagged_MAE) <- paste(names(bagged_MAE), "_bagged", sep = "")

############################### STACKING ###################################

stacking_data <- sapply(validation, function(type)
{
  type$stacking_data
})
stacking_columns <- colSums(is.na(stacking_data)) == 0
stacking_data %<>% subset(select = stacking_columns)
stack_col <- dim(stacking_data)[2]
tryCatch({
  # Prepare data for quadratic optimization stacking_data <- unique(stacking_data)
  d_temp <- observation_df
  Rinv <- solve(chol(t(stacking_data) %*% stacking_data))
  C <- cbind(rep(1, stack_col), diag(stack_col))
  b <- c(1, rep(0, stack_col))
  d <- t(d_temp[, metric_name]) %*% stacking_data
  # Do quadratic optimization and extract solution (weights for each model)
  stacking_weights <- solve.QP(Dmat = Rinv, factorized = TRUE, dvec = d, Amat = C, bvec = b, meq = 1)$solution
}, error = function(e) print(e))
if (!exists("stacking_weights"))
{
  stacking_weights <- rep(1/stack_col, stack_col)
}

print(stacking_weights)
model_fit_data <- sapply(validation[stacking_columns], function(type)
{
  type$model_fit
})

stacked_fit <- model_fit_data %*% stacking_weights

stacked_R2 <- c(cor(observation_df[, metric_name], stacked_fit)^2)
stacked_MAE <- colMeans(apply(stacked_fit, 2, function(x)
{
  abs(x - matrix(observation_df[, metric_name], ncol = 1))
}))
stacked_MAE <- stacked_MAE/normalization_obs_df

# Name the R2 and MAE
names(stacked_R2) <- "stacked_model"
names(stacked_MAE) <- "stacked_model"

############## COMBINE BASE, BAGGING, AND STACKING R2 AND MAE ################

# Append R2 and MAE for CV, bagged models and stacked model
complete_R2 <- c(CV_R2, bagged_R2, stacked_R2)
complete_MAE <- c(CV_MAE, bagged_MAE, stacked_MAE)
# Remove kriging bagged
complete_R2 <- complete_R2[names(complete_R2) != "kriging_bagged"]
complete_MAE <- complete_MAE[names(complete_MAE) != "kriging_bagged"]

# Check which models have acceptable R2 and MAE
complete_R2_ind <- complete_R2 > 0
complete_MAE_ind <- complete_MAE < 0.2

model_selection <- rep(TRUE, length(complete_R2_ind))
for (selection_feature in model_selection_features) switch(selection_feature, R2 = {
  model_selection <- model_selection & complete_R2_ind
}, MAE = {
  model_selection <- model_selection & complete_MAE_ind
})

chosen_model <- which.min(complete_MAE[model_selection])

# Write the MAE and R2 to file models.log 
# Check the iteration of the run 
# Check if the models.log exists if not set iteration to 1, if yes read the file and set the iteration to the max iter + 1

if (file.exists("models.log"))
{
  old_models <- read.table(file = "models.log", header = TRUE, sep = ",", dec = ".")
  if (iteration == 1)
  {
    run <- max(old_models$run) + 1
  } else
  {
    run <- max(old_models$run)
  }
} else
{
  run <- 1
  write("R2, MAE, model, iteration, run", file = "models.log")
}

models_info <- as.data.frame(t(rbind(complete_R2, complete_MAE, names(complete_R2))))
models_info$iteration <- iteration
models_info$run <- run
colnames(models_info) <- c("R2", "MAE",  "model", "iteration, run")
write.table(models_info, file = "models.log", col.names = FALSE, row.names = FALSE, sep = ",", dec = ".", append = TRUE)

# If necessary increase the model space -------------------------------------------------------------------------------------------------------
if (length(chosen_model) == 0)
{
  stop("No model was chosen. DOE will be explored again.")
}

# Write the model results for the grid created by the values set in the knobs definition ------------------------------------------------------

# Make predictions and write results ----------------------------------------------------------------------------------------------------------
print("I will make predictions now.")

Y_final <- list()

switch(names(chosen_model), linear = {
  Y_hat <- predict(validation$linear$models$full, design_space_grid, se.fit = TRUE, interval = "confidence")
  
  Y_final$mean <- Y_hat$fit[, 1]
  Y_final$sd <- Y_hat$se.fit
}, linear2 = {
  Y_hat <- predict(validation$linear2$models$full, design_space_grid, se.fit = TRUE, interval = "confidence")
  
  Y_final$mean <- Y_hat$fit[, 1]
  Y_final$sd <- Y_hat$se.fit
}, mars = {
  Y_hat <- predict(validation$mars$models$full, design_space_grid)
  
  Y_final$mean <- Y_hat
  Y_final$sd <- -1
}, polymars = {
  Y_hat <- predict(validation$polymars$models$full, design_space_grid)
  
  Y_final$mean <- Y_hat
  Y_final$sd <- -1
}, kriging = {
  Y_hat <- predict(validation$kriging$models$full, newdata = design_space_grid, type = "UK")
  
  Y_final$mean <- Y_hat$mean
  Y_final$sd <- Y_hat$sd
}, linear_bagged = {
  chosen_model2 <- gsub("_bagged", "", names(chosen_model))
  Y_hat <- sapply(validation[[chosen_model2]]$models, function(temp_model)
  {
    Y_hat <- predict(temp_model, design_space_grid)
    return(Y_hat)
  })
  
  Y_final$mean <- rowMeans(Y_hat)
  Y_final$sd <- -1
}, linear2_bagged = {
  chosen_model2 <- gsub("_bagged", "", names(chosen_model))
  Y_hat <- sapply(validation[[chosen_model2]]$models, function(temp_model)
  {
    Y_hat <- predict(temp_model, design_space_grid)
    return(Y_hat)
  })
  
  Y_final$mean <- rowMeans(Y_hat)
  Y_final$sd <- -1
}, mars_bagged = {
  chosen_model2 <- gsub("_bagged", "", names(chosen_model))
  Y_hat <- sapply(validation[[chosen_model2]]$models, function(temp_model)
  {
    Y_hat <- predict(temp_model, design_space_grid)
    return(Y_hat)
  })
  
  Y_final$mean <- rowMeans(Y_hat)
  Y_final$sd <- -1
}, polymars_bagged = {
  chosen_model2 <- gsub("_bagged", "", names(chosen_model))
  Y_hat <- sapply(validation[[chosen_model2]]$models, function(temp_model)
  {
    Y_hat <- predict(temp_model, design_space_grid)
    return(Y_hat)
  })
  
  Y_final$mean <- rowMeans(Y_hat)
  Y_final$sd <- -1
}, kriging_bagged = {
  chosen_model2 <- gsub("_bagged", "", names(chosen_model))
  Y_hat <- sapply(validation[[chosen_model2]]$models, function(temp_model)
  {
    Y_hat <- predict(temp_model, design_space_grid, type = "UK")$mean
    return(Y_hat)
  })
  
  Y_final$mean <- rowMeans(Y_hat)
  Y_final$sd <- -1
}, stacked_model = {
  stacked_fit2 <- sapply(validation[stacking_columns], function(type)
  {
    # Go over all the model fits, model + cross-validation models and take their fit means
    
    model_type <- class(type$models$full)[1]
    switch(model_type, lm = {
      Y_hat <- predict(type$models$full, design_space_grid)
    }, mars = {
      Y_hat <- predict(type$models$full, design_space_grid)
    }, polymars = {
      Y_hat <- predict(type$models$full, design_space_grid)
    }, km = {
      Y_hat <- predict(type$models$full, newdata = design_space_grid, type = "UK")$mean
    })
    return(Y_hat)
  })
  Y_final$mean <- stacked_fit2 %*% stacking_weights
  Y_final$sd <- -1
})

print("I will write results now")

# Write results of the model, for the whole grid
if (storage_type == "CASSANDRA")
{
  for (row.ind in 1:nrow(design_space_grid))
  {
    set_columns <- paste(c(knobs_names, paste(metric_name, "_avg", sep = ""), paste(metric_name, "_std", sep = "")), sep = "", collapse = ", ")
    set_values <- paste(cbind(design_space_grid, Y_final$mean, Y_final$sd)[row.ind, ], sep = "", collapse = ", ")
    # set_statement <- paste(doe_names, ' = ', doe_design[row.ind, ], sep = '', collapse = ', ')
    dbSendUpdate(conn, paste("INSERT INTO ", model_container_name, "(", set_columns, ") VALUES (", set_values, ")", sep = ""))
  }
} else if (storage_type == "CSV")
{
  write.table(design_space_grid_res, file = model_container_name, col.names = TRUE, row.names = FALSE, sep = ",", dec = ".")  
}
print(paste("Quiting [HTH] plugin", metric_name))
q(save = "no")
