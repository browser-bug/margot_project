library("dplyr")
library("magrittr")

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

print(paste("Started DOE plugin. Metric:", metric_name))

######################## SET WORKSPACE PATH AND VARIABLES #######################

setwd(root_path)
source("create_discrete_doe.R")
source("get_knobs_config_list.R")

# CREATE THE TABLES NAMES
application_name <- gsub("/", "_", application_name)

########################### LOAD DATA #######################################

if (storage_type == "CASSANDRA")
{
  suppressMessages(suppressPackageStartupMessages(library("RJDBC")))  # connect to database using JDBC codecs
  
  knobs_container_name <- paste("margot.", application_name, "_knobs", sep = "")
  features_container_name <- paste("margot.", application_name, "_features", sep = "")
  observation_container_name <- paste("margot.", application_name, "_trace", sep = "")
  model_container_name <- paste("margot.", application_name, "_model", sep = "")
  doe_container_name <- paste("margot.", application_name, "_doe", sep = "")
  
  # CONNECT TO CASSANDRA
  driver <- JDBC("com.github.adejanovski.cassandra.jdbc.CassandraDriver", "cassandra-jdbc-wrapper-3.1.0.jar", identifier.quote = "'")
  full_address_string <- paste("jdbc:cassandra://", storage_address, ":9042", sep = "")
  conn <- dbConnect(driver, full_address_string)
  
  # READ CONFIGURATION FROM CASSANDRA
  knobs_names <- dbGetQuery(conn, paste("SELECT name FROM ", knobs_container_name, sep = ""))
  features_names <- dbGetQuery(conn, paste("SELECT name FROM ", features_container_name, sep = ""))
  
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
} else if (storage_type == "CSV")
{
  knobs_container_name <- paste(storage_address, "/", application_name, "_knobs.csv", sep = "")
  features_container_name <- paste(storage_address, "/", application_name, "_features.csv", sep = "")
  observation_container_name <- paste(storage_address, "/", application_name, "_trace.csv", sep = "")
  model_container_name <- paste(storage_address, "/", application_name, "_model.csv", sep = "")
  doe_container_name <- paste(storage_address, "/", application_name, "_doe.csv", sep = "")
  
  knobs_names <- read.csv(knobs_container_name, stringsAsFactors = FALSE)$name
  features_names <- read.csv(features_container_name, stringsAsFactors = FALSE)$name
  
  nknobs <- length(knobs_names)
  if (nknobs == 0)
  {
    stop("Error: no knobs found. Please specify the knobs.")
  }
  if (length(features_names) == 0)
  {
    features_names <- NULL
  }
  cat("Number of KNOBS: ", nknobs, "\nNumber of FEATURES: ", dim(features_names)[1])
  
  conn <- NULL
} else
{
  stop(paste("Error: uknown $STORAGE_TYPE ", storage_type, ". Please, select $STORAGE_TYPE=CASSANDRA.", sep = ""), call. = FALSE)
}

################################# PREPARE DOE OPTIONS #################################

# MAKE NAMES LOWERCASE
knobs_names <- sapply(knobs_names, tolower)

# BEWARE does not account for the features !!!!!!  GET THE GRID CONFIGURATION
knobs_config_list <- get_knobs_config_list(storage_type, knobs_container_name, conn)

# SET THE DOE OPTIONS
doe_options <- list(nobs = nknobs * doe_obs_per_dim, eps = doe_eps)

############################ CREATE DOE ############################
doe_design <- create_doe(knobs_config_list, doe_options, map_to_input, algorithm = algorithm)
doe_names <- c(knobs_names, "counter")

# ADD COUNTER COLUMN
if (is.null(doe_design))
{
  doe_design <- matrix(c(doe_design, rep(doe_obs_per_point, length(doe_design))), ncol = 2)
} else
{
  doe_design <- cbind(doe_design, doe_obs_per_point)
}

################################# WRITE DOE #################################

if (storage_type == "CASSANDRA")
{
  for (row.ind in 1:nrow(doe_design))
  {
    set_columns <- paste(doe_names, sep = "", collapse = ", ")
    set_values <- paste(doe_design[row.ind, ], sep = "", collapse = ", ")
    # set_statement <- paste(doe_names, ' = ', doe_design[row.ind, ], sep = '', collapse = ', ')
    dbSendUpdate(conn, paste("INSERT INTO ", doe_container_name, "(", set_columns, ") VALUES (", set_values, ")", sep = ""))
  }
} else if (storage_type == "CSV")
{
  write.table(doe_design, file = doe_container_name, col.names = TRUE, row.names = FALSE, sep = ",", dec = ".")
}

print("Wrote new DOE configurations")
q(save = "no")
