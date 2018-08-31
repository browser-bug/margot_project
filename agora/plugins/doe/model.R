# library(crs, quietly = TRUE)    # to get the response surface models
library("RJDBC") # connect to database using JDBC codecs
library("mda")
library("polspline")
library("dplyr")
# source("R/create_discrete_doe.R")
# source("R/CV_mod.R")
# source("R/get_knobs_config_list.R")


options(scipen = 100)

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
if (length(args) < 5)
{
  stop("Error: Number of input parameters is less than 5 (Please input the storage_type, storage_address, application_name, metric_name and root_path)", call.=FALSE)
} else if (length(args) == 5)
{
  storage_type <- args[1]
  storage_address <- args[2]
  application_name <- args[3]
  metric_name <- args[4]
  root_path <- args[5]
} else
{
  storage_type <- args[1 ]
  storage_address <- args[2]
  application_name <- args[3]
  metric_name <- args[4]
  root_path <- args[5]
  print(paste("Warning: the following program option are ignored:", args[6:nrow(args)], collapse = ", "))
}

setwd(root_path)
source("create_discrete_doe.R")
# source("CV_mod.R")
# source("R2_adj.R")
source("get_knobs_config_list.R")
source("fit_models_agora.R")

application_name <- gsub("/", "_", application_name)

knobs_container_name <- paste("margot.", application_name, "_knobs", sep = "")
features_container_name <- paste("margot.", application_name, "_features", sep = "")
observation_container_name <- paste("margot.", application_name, "_trace", sep = "")
model_container_name <- paste("margot.", application_name, "_model", sep = "")
doe_container_name <- paste("margot.", application_name, "_doe", sep = "")

# connect to the database

if(storage_type == "CASSANDRA")
{
  driver <- JDBC("com.github.adejanovski.cassandra.jdbc.CassandraDriver", "/home/martinovic/SynologyDrive/IT4I/margot/java/cassandra-jdbc-wrapper-3.1.0.jar", identifier.quote = "'")
  full_address_string <- paste("jdbc:cassandra://", storage_address, ":9042", sep = "")
  conn <- dbConnect(driver, full_address_string)
} else
{
  stop(paste("Error: uknown $STORAGE_TYPE ", storage_type, ". Please, select $STORAGE_TYPE=CASSANDRA.", sep = ""), call.=FALSE)
}

# read the remainder of the configuration from cassandra
knobs_names <- dbGetQuery(conn, paste("SELECT name FROM ", knobs_container_name, sep = ""))
features_names <- dbGetQuery(conn, paste("SELECT name FROM ", features_container_name, sep = ""))

nknobs <- dim(knobs_names)[1]

if(nknobs == 0)
{
  stop("Error: no knobs found. Please specify the knobs.")
}

if(dim(features_names)[1] == 0)
{
  features_names <- NULL
}
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

#Forcefully set map_to_input = TRUE for now
map_to_input = TRUE
if(map_to_input == TRUE)
{
  nobserved_orig <- dim(observation_df)[1]
  # Discard observation which are not on the grid
  observation_df <- inner_join(observation_df, design_space_grid, by = input_columns)
  if(nobserved_orig > dim(observation_df)[1])
  {
    warning("There were some observation were out of the design grid and were discarded.")
  }
}

# Set nobserved the number of observation in the obsercation_df
nobserved <- dim(observation_df)[1]

if(nobserved == 0)
{
  # Need to add this options to margot/agora ...
  doe_options <- list(nobs = nknobs * 5, eps = 0.2)
  
  # Propose DOE design points
  doe_design <-  create_doe(knobs_config_list, doe_options, map_to_input = TRUE, algorithm = "lhs")
  doe_names <- c(knobs_names, "counter")
  
  # Add values for counter column
  if(is.null(doe_design))
  {
    doe_design <- matrix(c(doe_design, rep(1, length(doe_design))), ncol = 2)
  } else
  {
    doe_design <- cbind(doe_design, 1)
  }
  
  for(row.ind in 1:nrow(doe_design))
  {
    set_columns <- paste(doe_names, sep = "", collapse = ", ")
    set_values <- paste(doe_design[row.ind, ], sep = "", collapse = ", ")
    # set_statement <- paste(doe_names, " = ", doe_design[row.ind, ], sep = "", collapse = ", ")
    dbSendUpdate(conn, paste("INSERT INTO ", doe_container_name, "(", set_columns, ") VALUES (", set_values, ")", sep = ""))
  }
  
  # Quit to let the AGORA ask for the results at the specific points
  q(save = "no")
}


# Create models ---------------------------------------------------------------------------------------------------------------
print("I fit models now.")

# Permute observation_df should prevent same observation being in the test fold for cross validation
observation_df <- observation_df[sample(1:nobserved, nobserved), ]

validation <- fit_models_agora(observation_df, input_columns, metric_name, nobserved)



# Check if there is a good model ----------------------------------------------------------------------------------------
model_selection_features <- c("R2", "MAE")

if( "R2" %in% model_selection_features )
{
  model_R2 <-sapply(validation,
                    function(model)
                     {median(model$R2[2:length(model$R2)], na.rm = TRUE)}
  )
  
  
  model_R2_ind <- rep(NA, length(model_R2))
  
  model_R2_ind[model_R2 > 0.7] <- 2
  model_R2_ind[model_R2 >= 0.5 & model_R2 <= 0.7] <- 1
  model_R2_ind[model_R2 < 0.5] <- 0
}

if( "MAE" %in% model_selection_features )
{
  model_MAE <-sapply(validation,
                    function(model)
                    {median(model$MAE[2:length(model$MAE)])}
  )
  model_MAE <- model_MAE / max(observation_df[, metric_name])
  
  model_MAE_ind <- rep(NA, length(model_MAE))
  
  model_MAE_ind[model_MAE < 0.1] <- -2
  model_MAE_ind[model_MAE >= 0.1 & model_MAE <= 0.3] <- -1
  model_MAE_ind[model_MAE > 0.3] <- 0
  model_MAE_ind <- abs(model_MAE_ind)
}

models_for_selection <- rep(0, length(validation))

for(selection_feature in model_selection_features)
  switch(selection_feature,
         "R2" = {models_for_selection <- models_for_selection + (model_R2_ind == 2)},
         "MAE" = {models_for_selection <- models_for_selection + (model_MAE_ind == 2)})

models_for_selection <- models_for_selection == length(model_selection_features)

if(any(models_for_selection))
{
  chosen_model <- which.min(model_MAE[models_for_selection])
} else
{
  models_for_ensemble <- rep(0, length(validation))
  
  for(selection_feature in model_selection_features)
    switch(selection_feature,
           "R2" = {models_for_ensemble <- models_for_ensemble + (model_R2_ind >= 1)},
           "MAE" = {models_for_ensemble <- models_for_ensemble + (model_MAE_ind >= 1)})
  
  models_for_ensemble <- models_for_ensemble == length(model_selection_features)
  
  if(sum(models_for_ensemble >= 2))
  {
    # Do ensemble
    chosen_model <- 0
  } else
  {
    # If there is no good model for prediction or the ensemble set model selected to 0
    chosen_model <- 0
  }
}



# If necessary increase the model space -------------------------------------------------------------------------------------------------------
if(chosen_model == 0)
{
  # Get knobs config list
  knobs_config_list <- get_knobs_config_list(conn, knobs_container_name)
  
  print("Model is not good enough. I will ask for more points to explore.")
  # 0.2 could be substituted by parameter DOE update rate.
  updated_nobs <- floor(nobserved * 1.2)
  doe_options <- list(nobs = updated_nobs, eps = 0.2)
  
  # Propose DOE design points
  doe_design <-  create_doe(knobs_config_list, doe_options, map_to_input = TRUE, algorithm = "dmax")
  doe_names <- c(knobs_names, "counter")
  
  # Add values for counter column
  if(is.null(doe_design))
  {
    doe_design <- matrix(c(doe_design, rep(1, length(doe_design))), ncol = 2)
  } else
  {
    doe_design <- cbind(doe_design, 1)
  }
  
  for(row.ind in 1:nrow(doe_design))
  {
    set_columns <- paste(doe_names, sep = "", collapse = ", ")
    set_values <- paste(doe_design[row.ind, ], sep = "", collapse = ", ")
    # set_statement <- paste(doe_names, " = ", doe_design[row.ind, ], sep = "", collapse = ", ")
    dbSendUpdate(conn, paste("INSERT INTO ", doe_container_name, "(", set_columns, ") VALUES (", set_values, ")", sep = ""))
  }
  
  # Quit to let the AGORA ask for the results at the specific points
  q(save = "no")
}

# Write the model results for the grid created by the values set in the knobs definition ------------------------------------------------------

# Make predictions and write results ----------------------------------------------------------------------------------------------------------
print("I will make predictions now.")

Y_final <- list()

switch(names(chosen_model),
  "linear" = {
    Y_hat <- predict(validation$linear$model, design_space_grid, se.fit = TRUE, interval = "confidence") 
    
    Y_final$mean <- Y_hat$fit[,1]
    Y_final$sd   <- Y_hat$se.fit
  },
  "linear2" = {
    Y_hat <- predict(validation$linear2$model, design_space_grid, se.fit = TRUE, interval = "confidence")
 
      Y_final$mean <- Y_hat$fit[,1]
      Y_final$sd   <- Y_hat$se.fit
  },
  "mars" = {
    Y_hat <- predict(validation$mars$model, design_space_grid)
    
    Y_final$mean <- Y_hat
    Y_final$sd   <- -1
  },
  "polymars" = {
    Y_hat <- predict(validation$polymars$model, design_space_grid)
    
    Y_final$mean <- Y_hat
    Y_final$sd   <- -1
  },
  "kriging" = {
    Y_hat <- predict(validation$kriging$model, newdata = design_space_grid, type = "UK")
    
    Y_final$mean <- Y_hat$mean
    Y_final$sd   <- Y_hat$sd
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


q(save = "no")