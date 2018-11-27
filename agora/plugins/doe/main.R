library("tidyverse")
library("rlang")
library("magrittr")

options(scipen = 100)
map_to_input = TRUE
limits <- NULL

######################## GET THE ARGUMENTS ############################

args = commandArgs(trailingOnly = TRUE)
if (length(args) < 1)
{
  stop("Error: Number of input parameters is less than 1 (Please input the root_path)", call. = FALSE)
} else if (length(args) == 1)
{
  root_path <- args[1]
} else
{
  root_path <- args[1] 
  print(paste("Warning: the following program option are ignored:", args[2:nrow(args)], collapse = ", "))
}

######################## SET WORKSPACE PATH AND VARIABLES #######################

setwd(root_path)
source("create_discrete_doe.R")
source("get_config_list.R")

configurations <- readLines(con = "agora_config.env")
configurations <- strsplit(configurations, '"')

storage_type <- configurations %>% .[grepl("STORAGE_TYPE",.)] %>% unlist %>% .[2]
storage_address <- configurations %>% .[grepl("STORAGE_ADDRESS",.)] %>% unlist %>% .[2]
application_name <- configurations %>% .[grepl("APPLICATION_NAME",.)] %>% unlist %>% .[2]
metric_name <- configurations %>% .[grepl("METRIC_NAME",.)] %>% unlist %>% .[2]
algorithm <- configurations %>% .[grepl("DOE_NAME",.)] %>% unlist %>% .[2]
doe_eps <- configurations %>% .[grepl("MINIMUM_DISTANCE",.)] %>% unlist %>% .[2] %>% as.numeric
doe_obs_per_conf <- configurations %>% .[grepl("NUMBER_OBSERVATIONS_PER_CONFIGURATION",.)] %>% unlist %>% .[2] %>% as.numeric
doe_obs_per_iter <- configurations %>% .[grepl("NUMBER_CONFIGURATIONS_PER_ITERATION",.)] %>% unlist %>% .[2] %>% as.numeric
if(any(grepl("DOE_LIMITS", configurations))){
  limits <- configurations[grepl("DOE_LIMITS", configurations)]
  limits <- limits[[1]][2]
  limits <- strsplit(limits, ";")[[1]]
}

print(paste("Started DOE plugin. Metric:", metric_name))

########################### LOAD DATA #######################################
# CREATE THE TABLES NAMES
application_name <- gsub("/", "_", application_name)

if (storage_type == "CASSANDRA"){
  suppressMessages(suppressPackageStartupMessages(library("RJDBC")))  # connect to database using JDBC codecs
  
  knobs_container_name <- paste("margot.", application_name, "_knobs", sep = "")
  features_container_name <- paste("margot.", application_name, "_features", sep = "")
  observation_container_name <- paste("margot.", application_name, "_trace", sep = "")
  model_container_name <- paste("margot.", application_name, "_model", sep = "")
  doe_container_name <- paste("margot.", application_name, "_doe", sep = "")
  metrics_container_name <- paste("margot.", application_name, "_metrics", sep = "")
  
  # CONNECT TO CASSANDRA
  driver <- JDBC("com.github.adejanovski.cassandra.jdbc.CassandraDriver", "cassandra-jdbc-wrapper-3.1.0.jar", identifier.quote = "'")
  full_address_string <- paste("jdbc:cassandra://", storage_address, ":9042", sep = "")
  conn <- dbConnect(driver, full_address_string)
  
  # READ CONFIGURATION FROM CASSANDRA
  knobs_names <- dbGetQuery(conn, paste("SELECT name FROM ", knobs_container_name, sep = ""))
  features_names <- dbGetQuery(conn, paste("SELECT name FROM ", features_container_name, sep = ""))
  metric_names <- dbGetQuery(conn, paste("SELECT name FROM ", metrics_container_name, sep = ""))
  
} else if (storage_type == "CSV"){
  knobs_container_name <- paste(storage_address, "/", application_name, "_knobs.csv", sep = "")
  features_container_name <- paste(storage_address, "/", application_name, "_features.csv", sep = "")
  observation_container_name <- paste(storage_address, "/", application_name, "_trace.csv", sep = "")
  model_container_name <- paste(storage_address, "/", application_name, "_model.csv", sep = "")
  doe_container_name <- paste(storage_address, "/", application_name, "_doe.csv", sep = "")
  metrics_container_name <- paste(storage_address, "/", application_name, "_metrics.csv", sep = "")
  
  knobs_names <- read_csv(knobs_container_name) %>% pull(name)
  features_names <- read_csv(features_container_name) %>% pull(name)
  metric_names <- read_csv(metrics_container_name) %>% pull(name)
  
  conn <- NULL
} else{
  stop(paste("Error: uknown $STORAGE_TYPE ", storage_type, ". Please, select storage type CASSANDRA or CSV.", sep = ""), call. = FALSE)
}

nknobs <- length(knobs_names)
if (nknobs == 0)  {
  stop("Error: no knobs found. Please specify the knobs.")
}
if (length(features_names) == 0)  {
  features_names <- NULL
}

writeLines(str_c("Number of KNOBS: ", nknobs, "\nNumber of FEATURES: ", length(features_names)[1], if(is.null(features_names)){"0"}))

################################# PREPARE DOE OPTIONS #################################

# MAKE NAMES LOWERCASE FOR CASSANDRA (JUST TO BE SURE)
knobs_names <- str_to_lower(knobs_names)
if(!is.null(features_names)){
  features_names <- str_to_lower(features_names)
}
metric_name <- str_to_lower(metric_name)

# GET GRID CONFIGURATION
knobs_config_list <- get_config_list(storage_type, knobs_container_name, conn)
if(!is.null(features_names)){
  features_config_list <- get_config_list(storage_type, features_container_name, conn)
}

# SET THE DOE OPTIONS
doe_options <- list(nobs = doe_obs_per_iter, eps = doe_eps)

############################ CREATE DOE ############################
doe_design <- create_doe(knobs_config_list, doe_options, knobs_names, model_container_name, metric_names, limits, algorithm, storage_type)

# ADD COUNTER COLUMN
if (is.null(doe_design)){
  doe_design <- matrix(c(doe_design, rep(doe_obs_per_conf, length(doe_design))), ncol = 2)
} else{
  doe_design <- cbind(doe_design, doe_obs_per_conf)
}

doe_names <- c(knobs_names, "counter")
names(doe_design) <- doe_names

################################# WRITE DOE #################################

if (storage_type == "CASSANDRA")
{
  for (row.ind in 1:nrow(doe_design))
  {
    set_columns <- paste(doe_names, sep = "", collapse = ", ")
    set_values <- paste(doe_design[row.ind, ], sep = "", collapse = ", ")
    dbSendUpdate(conn, paste("INSERT INTO ", doe_container_name, "(", set_columns, ") VALUES (", set_values, ")", sep = ""))
  }
} else if (storage_type == "CSV")
{
  write.table(doe_design, file = doe_container_name, col.names = FALSE, row.names = FALSE, sep = ",", dec = ".", append = TRUE)
}

print("Wrote new DOE configurations")
q(save = "no")

