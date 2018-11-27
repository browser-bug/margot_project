# LOAD LIBRARIES ----------------------------------------------------------

suppressMessages(suppressPackageStartupMessages(library("mda")))
suppressMessages(suppressPackageStartupMessages(library("polspline")))
suppressMessages(suppressPackageStartupMessages(library("tidyverse")))
suppressMessages(suppressPackageStartupMessages(library("quadprog")))
suppressMessages(suppressPackageStartupMessages(library("DiceKriging")))
suppressMessages(suppressPackageStartupMessages(library("rlang")))

# SET OPTIONS -------------------------------------------------------------
# This ensures R does not use scientific notation on numbers
options(scipen = 100)

# GET THE ARGUMENTS -------------------------------------------------------

args = commandArgs(trailingOnly = TRUE)
if (length(args) < 1)
{
  stop("Error: Number of input parameters is less than 1 (Please input the storage_type, storage_address, application_name, metric_name and root_path)", call. = FALSE)
} else if (length(args) == 1)
{
  root_path <- args[1]
} else
{
  root_path <- args[1] 
  print(paste("Warning: the following program option are ignored:", args[2:nrow(args)], collapse = ", "))
}

# SET WORKSPACE PATH AND VARIABLES ----------------------------------------

setwd(root_path)
source("get_knobs_config_list.R")
source("models_functions.R")

configurations <- readLines(con = "agora_config.env")
configurations <- strsplit(configurations, '"')

storage_type <- configurations %>% .[grepl("STORAGE_TYPE",.)] %>% unlist %>% .[2]
storage_address <- configurations %>% .[grepl("STORAGE_ADDRESS",.)] %>% unlist %>% .[2]
application_name <- configurations %>% .[grepl("APPLICATION_NAME",.)] %>% unlist %>% .[2]
metric_name <- configurations %>% .[grepl("METRIC_NAME",.)] %>% unlist %>% .[2]
holdout_set_size <- configurations %>% .[grepl("VALIDATION_SPLIT",.)] %>% unlist %>% .[2] %>% as.numeric
nfolds <- configurations %>% .[grepl("K_VALUE",.)] %>% unlist %>% .[2] %>% as.numeric
minimal_R2 <- configurations %>% .[grepl("MIN_R2",.)] %>% unlist %>% .[2] %>% as.numeric
max_mae <- configurations %>% .[grepl("MAX_MAE",.)] %>% unlist %>% .[2] %>% as.numeric
iteration <- configurations %>% .[grepl("ITERATION_COUNTER",.)] %>% unlist %>% .[2] %>% as.numeric
max_iter <- configurations %>% .[grepl("MAX_NUMBER_ITERATION",.)] %>% unlist %>% .[2] %>% as.numeric
current_nconf <- configurations %>% .[grepl("NUMBER_CONFIGURATIONS_PER_ITERATION",.)] %>% unlist %>% .[2] %>% as.numeric

if(any(grepl("DOE_LIMITS", configurations))){
  limits <- configurations[grepl("DOE_LIMITS", configurations)]
  limits <- limits[[1]][2]
  limits <- strsplit(limits, ";")[[1]]
}

print(paste("Started DOE plugin. Metric:", metric_name))

if(iteration >= max_iter){
  minimal_R2 <- 0
  max_mae <- 1
}

# LOAD DATA ---------------------------------------------------------------

writeLines(paste("Started plugin", metric_name))

application_name <- gsub("/", "_", application_name)

if (storage_type == "CASSANDRA") {
  suppressMessages(suppressPackageStartupMessages(library("RJDBC")))  # connect to database using JDBC codecs
  # SET TABLE NAMES
  knobs_container_name <- paste("margot.", application_name, "_knobs", sep = "")
  features_container_name <- paste("margot.", application_name, "_features", sep = "")
  observation_container_name <- paste("margot.", application_name, "_trace", sep = "")
  model_container_name <- paste("margot.", application_name, "_model", sep = "")
  doe_container_name <- paste("margot.", application_name, "_doe", sep = "")
  # LOAD DRIVER AND ACCESS DATABASE
  driver <- JDBC("com.github.adejanovski.cassandra.jdbc.CassandraDriver", "cassandra-jdbc-wrapper-3.1.0.jar", identifier.quote = "'")
  full_address_string <- paste("jdbc:cassandra://", storage_address, ":9042", sep = "")
  conn <- dbConnect(driver, full_address_string)
  # GET KNOBS AND FEATURES NAMES
  knobs_names <- dbGetQuery(conn, paste("SELECT name FROM ", knobs_container_name, sep = ""))
  features_names <- dbGetQuery(conn, paste("SELECT name FROM ", features_container_name, sep = ""))
  # GET NUMBER OF KNOBS AND FEATURES
  nknobs <- length(knobs_names)
  if (nknobs == 0) {
    stop("Error: no knobs found. Please specify the knobs.")
  }
  if (length(features_names) == 0) {
    features_names <- NULL
  }
} else if (storage_type == "CSV") {
  # SET FILE PATHS
  knobs_container_name <- paste(storage_address, "/", application_name, "_knobs.csv", sep = "")
  features_container_name <- paste(storage_address, "/", application_name, "_features.csv", sep = "")
  observation_container_name <- paste(storage_address, "/", application_name, "_trace.csv", sep = "")
  model_container_name <- paste(storage_address, "/", application_name, "_model.csv", sep = "")
  doe_container_name <- paste(storage_address, "/", application_name, "_doe.csv", sep = "")
  # GET KNOBS AND FEATURES NAMES
  knobs_names <- read_csv(knobs_container_name) %>% pull(name)
  features_names <- read_csv(features_container_name) %>% pull(name)
  # GET NUMBER OF KNOBS AND FEATURES
  nknobs <- length(knobs_names)
  if (nknobs == 0) {
    stop("Error: no knobs found. Please specify the knobs.")
  }
  if (length(features_names) == 0) {
    features_names <- NULL
  }
  # SET DB CONNECTION TO NULL
  conn <- NULL
} else {
  stop(paste("Error: uknown $STORAGE_TYPE ", storage_type, ". Please, select $STORAGE_TYPE, CASANDRA or CSV", sep = ""), call. = FALSE)
}

# PREPARE DOE OPTIONS -----------------------------------------------------

writeLines(str_c("Number of KNOBS: ", nknobs, "\nNumber of FEATURES: ", dim(features_names)[1], if(is.null(features_names)){"0"}))
# MAKE NAMES LOWERCASE
knobs_names <- str_to_lower(knobs_names)
# GET COLUMN NAMES OF DSE AND INPUT
dse_columns <- c(knobs_names, features_names, metric_name)
input_columns <- c(knobs_names, features_names)
# CHECK IF THERE ARE ANY RESULTS IN OBSERVATION CONTAINER LOAD OBSERVATION DATA FRAME
if (storage_type == "CASSANDRA") {
  observation_df <- dbGetQuery(conn, paste("SELECT ", paste(dse_columns, collapse = ","), " FROM ", observation_container_name, sep = ""))
} else if (storage_type == "CSV") {
  observation_df <- read_csv(observation_container_name)
  observation_df <- observation_df %>% select(dse_columns)
}

# CREATE DESIGN SPACE GRID ------------------------------------------------

# BEWARE does not account for the features !!!!!!  GET GRID CONFIGURATION
knobs_config_list <- get_knobs_config_list(storage_type, knobs_container_name, conn)
# CREATE KNOB TRANSFORM LIST
knob_transform <- sapply(knobs_config_list, function(knob_config) {
  knob_adjusted_max <- max(knob_config) - min(knob_config)
  knob_min <- min(knob_config)
  return(c(knob_min, knob_adjusted_max))
})

# CREATE DESIGN SPACE GRID
design_space_grid <- expand.grid(knobs_config_list)
colnames(design_space_grid) <- knobs_names

if(exists("limits")){
  for(limits_iter in limits){
    design_space_grid <- design_space_grid %>% filter(!!parse_quo(limits_iter, env = environment()))
  }
}

# DISCARD 'BAD' OBSERVATIONS ----------------------------------------------

# GET BOOLEAN VECTOR OF ROWS WITHOUT NA VALUES
ind_complete <- complete.cases(observation_df)
# DISCARD ROWS WITH NA VALUES
if (any(!ind_complete)) {
  warning("Some observations are incomplete and are discarded for the model learning.")
  writeLines("I discarded following observations: ")
  print(observation_df %>% filter(!ind_complete))
  observation_df <- observation_df %>% filter(ind_complete)
}
nobserved_orig <- nrow(observation_df)
# DISCARD OBSERVATION OUTSIDE THE DESIGN SPACE GRID
observation_df <- inner_join(observation_df, design_space_grid, by = input_columns)
nobserved <- nrow(observation_df)
if (nobserved_orig > nobserved) {
  warning("There were some observation outside of the design grid and were discarded.")
}
rm(ind_complete, nobserved_orig)

# GET MAE NORMALIZATION VALUES --------------------------------------------

error_normalization_value <- abs(diff(range(observation_df %>% pull(metric_name))))
if (error_normalization_value == 0)
{
  stop("Error: All the values of the response variables are constant. There might be a bug in Your model.")
}

# CREATE UNIQUE CONFIGURATION DATA FRAME ----------------------------------

configuration_df <- unique(observation_df %>% select(input_columns))
nconfiguration <- nrow(configuration_df)

# STOP IF THERE ARE NO OBSERVATIONS
if (nconfiguration == 0) {
  stop("Error: No DOE results found!")
}

writeLines(str_c("Number of observations in the TRACE table: ", nobserved))
writeLines(str_c("Number of configurations in the TRACE table: ", nconfiguration))

writeLines("I fit models now.")

# FIT MODELS AND CHOSE THE BEST ONE ---------------------------------------

if (nconfiguration <= nfolds) {
  nfolds <- nconfiguration
  models_df <- model_list(length(input_columns))
  models_df <- models_df %>% mutate(fitted_models = map(.x = models, .f = fit_selected_model, observation_df, input_columns, metric_name))
  cv_df <- cross_validation(models_df$models, nfolds, observation_df, configuration_df, input_columns, metric_name)
  # THERE IS NO R2, BECAUSE OUTPUT WOULD BE A SINGULAR VALUE MOST OF THE TIME
  # AND THERE IS NO WAY TO COMPUTE CORRELATION BETWEEN SINGULAR VALUES
  models_df <- models_df %>% mutate(MAE = map_dbl(models, extract_measure_cv, "MAE", cv_df)) %>%
    mutate(MAE = MAE / error_normalization_value)
  chosen_model <- models_df %>% slice(which.min(MAE)) %>% select(models)
} else {
  holdout_set_size  <- min((nconfiguration - nfolds)/nconfiguration, holdout_set_size)
  holdout_nfolds <- floor(1/holdout_set_size)
  if(holdout_nfolds > nfolds)holdout_nfolds <- nfolds
  holdout_folds <- cut(seq(1, nrow(configuration_df)), breaks = holdout_nfolds, labels = FALSE)
  configuration_df <- configuration_df[sample(nrow(configuration_df)), ]

  holdout_results_df <- tibble(fold = seq(1, holdout_nfolds))
  holdout_results_df <- holdout_results_df %>%
    mutate(res = map(fold,
                     holdout_validation,
                     configuration_df,
                     holdout_folds,
                     observation_df,
                     metric_name,
                     input_columns,
                     nfolds,
                     error_normalization_value))
  holdout_results_df <- holdout_results_df %>% unnest(res) %>% group_by(models) %>% summarize(R2 = mean(R2, na.rm = TRUE), MAE = mean(MAE, na.rm = TRUE))

  # MODEL SELECTION AND LEARNING
  chosen_model <- holdout_results_df %>% filter(R2 > minimal_R2, MAE < max_mae) %>% slice(which.min(MAE)) %>% select(models)
}

# WRITE MODEL RESULTS -----------------------------------------------------

if (file.exists("models.log")) {
  old_models <- read_csv(file = "models.log")
  old_models <- old_models %>% filter(nconf == current_nconf)
  if(dim(old_models)[1] == 0){
    run <- 1
  } else if (iteration == 1) {
    run <- max(old_models$run) + 1
  } else {
    
    run <- max(old_models$run)
  }
} else {
  run <- 1
  write("model, R2, MAE, iteration, run, nconf, metric", file = "models.log")
}

if(exists("holdout_results_df")){
  models_info <- holdout_results_df
} else if(exists("models_df")){
  models_info <-  models_df %>% mutate(R2 = "NA") %>% select(c("models", "R2", "MAE"))
} else{
  error("No results from models fits found, after model fitting. This should not happen...")
}

models_info <- models_info %>% mutate(iteration = iteration, run = run, nconf = current_nconf, metric = metric_name)
write_csv(models_info, path = "models.log", append = TRUE)

# CHECK THE CHOSEN MODEL AND MAKE FITS IF NECESSARY -----------------------

if(dim(chosen_model)[1] == 0){
  print("No model was good enough, asking for more configurations to explore.")
  quit(save = "no")
}

if(!exists("models_df")){
  if(grepl("bagging", chosen_model)){
    models_df <- model_list(length(input_columns))
    models_df <- models_df %>% mutate(fitted_models = map(.x = models, .f = fit_selected_model, observation_df, input_columns, metric_name))
    cv_df <- cross_validation(models_df$models, nfolds, observation_df, configuration_df, input_columns, metric_name)
  } else if(chosen_model == "stacking"){
    models_df <- model_list(length(input_columns))
    models_df <- models_df %>% mutate(fitted_models = map(.x = models, .f = fit_selected_model, observation_df, input_columns, metric_name))
    cv_df <- cross_validation(models_df$models, nfolds, observation_df, configuration_df, input_columns, metric_name)

    stacking_df <- create_stacking_df(cv_df, configuration_df, observation_df, input_columns)
    d_temp <- stacking_df %>% pull(metric_name)
    stacking_df <- stacking_df %>% select(-metric_name) %>% as.matrix()
    stacking_weights <- compute_stacking_weights(stacking_df, d_temp)
    print(stacking_weights)
    rm(d_temp, stacking_df)
  } else{
    models_df <- chosen_model %>% mutate(fitted_models = map(.x = models, .f = fit_selected_model, observation_df, input_columns, metric_name))
  }
}

# COMPUTE PREDICTIONS FOR DESIGN SPACE GRID -------------------------------

if(grepl("bagging", chosen_model)){
  sub_cv_df <- create_sub_cv_df(chosen_model, cv_df)
  Y_final <- predict_selected_model(chosen_model, NA, sub_cv_df, design_space_grid)
} else if(chosen_model == "stacking"){
  Y_final <- predict_selected_model(chosen_model, NA, NA, design_space_grid, models_df, stacking_weights)
} else{
  chosen_model_fit <- models_df %>% filter(models == chosen_model$models) %>% pull(fitted_models)
  chosen_model_fit <- chosen_model_fit[[1]]
  Y_final <- predict_selected_model(chosen_model, chosen_model_fit, NA, design_space_grid)
}

# WRITE RESULTS -----------------------------------------------------------

if (storage_type == "CASSANDRA"){
  for (row.ind in 1:nrow(design_space_grid))
  {
    set_columns <- str_c(c(knobs_names, str_c(metric_name, "_avg"), str_c(metric_name, "_std")), collapse = ", ")
    set_values <- str_c(cbind(design_space_grid, Y_final$fit, Y_final$sd)[row.ind, ], collapse = ", ")
    dbSendUpdate(conn, str_c("INSERT INTO ", model_container_name, "(", set_columns, ") VALUES (", set_values, ")"))
  }
} else if (storage_type == "CSV"){
  model_csv <- read_csv(model_container_name)
  metric_avg <- str_c(metric_name, "_avg")
  metric_std <- str_c(metric_name, "_std")
  temp_df <- design_space_grid %>% mutate(!!metric_avg := as.numeric(Y_final$fit))
  temp_df <- temp_df %>% mutate(!!metric_std := as.numeric(Y_final$sd))
  model_csv <- model_csv %>% select(-!!str_c(metric_name, "_std"), -!!str_c(metric_name, "_avg"))
  model_csv <- inner_join(temp_df, model_csv)
  write.table(model_csv, file = model_container_name, col.names = TRUE, row.names = FALSE, sep = ",", dec = ".")
}

print(paste("Quiting [HTH] plugin", metric_name))
q(save = "no")
