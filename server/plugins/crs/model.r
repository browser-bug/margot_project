library(Hmisc)  # to get the correlation coefficients
library(crs)    # to get the response surface models


#######################################################################
#### GENERAL CONFIGURATION SECTION OF THE SCRIPT
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
if (length(args) < 2)
{
  stop("Error: unable to retrieve the name of the target metric (pass the name as program option)", call.=FALSE)
} else if (length(args) == 2)
{
  metric_name <- args[1]
  root_path <- args[2]
  correlation_threshold <- 0.2
} else if (length(args) == 3)
{
  metric_name <- args[1]
  root_path <- args[2]
  correlation_threshold <- args[3]
} else
{
  metric_name <- args[1]
  root_path <- args[2]
  correlation_threshold <- args[3]
  print(paste("Warning: the following program option are ignored:", args[4:nrow(args)], collapse = ", "))
}

# read the remainder of the configuration from txt files
knobs <- read.table(paste(root_path, "knobs.txt", sep = "/"), header = FALSE, stringsAsFactors = FALSE)
features <-read.table(paste(root_path, "features.txt", sep = "/"), header = FALSE, stringsAsFactors = FALSE)


#######################################################################
#### INPUT PART OF THE SCRIPT
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


# load the profile information from the dse
profiling_df <- read.table(paste(root_path, "dse.txt", sep = "/"), header = TRUE, sep=",", stringsAsFactors = FALSE)


# load the requested predictions
prediction_df <- read.table(paste(root_path, "prediction_request.txt", sep = "/"), header = TRUE, sep=",", stringsAsFactors = FALSE)









#######################################################################
#### GENERATE THE SURFACE MODEL OF THE APPLICATION
#######################################################################
# This part of the application build a model of the application, using
# regression techiniques based on splines to model each metric.
#
# INPUT:
# ---------------------------------------------------------------------
# All the information from the previous sections of the application
#
# OUTPUT:
# ---------------------------------------------------------------------
# update the prediction_df dataframe with predicted metrics




# this is a global configuration space, which is not exposed to the
# outer world, at least in this implementation
second_order_predictors_separator <- "__x__"
splines_degrees = 3



# ----------- generate the predictor space
# It takes into account first order predictors from the input data, and then
# generates second order predictors.


# unify the software knobs and input features as first order predictors
first_order_predictors <- rbind(knobs, features)[[1]]

# generate the second order predictor pairs as combination the first order ones
second_order_predictors <- expand.grid(first_order_predictors, first_order_predictors, stringsAsFactors = FALSE)
valid_predictors <- second_order_predictors$Var1 < second_order_predictors$Var2
second_order_predictors <- second_order_predictors[valid_predictors,]

# augment the input data frame with the second oreder predictors and generate
# also their names attached to the original dataframe
second_order_predictor_names <- c()
for( predictor_index in 1:nrow(second_order_predictors))
{
  first_term_name <- second_order_predictors[predictor_index, 1]
  second_term_name <- second_order_predictors[predictor_index, 2]
  this_predictor_name <- paste(first_term_name, second_term_name, sep = second_order_predictors_separator)
  second_order_predictor_names <- c(second_order_predictor_names, this_predictor_name)
  profiling_df <- cbind(profiling_df, profiling_df[,first_term_name] * profiling_df[,second_term_name])
  names(profiling_df)[length(profiling_df)] <- this_predictor_name
}


# ----------- generate the correlation matrix
terms_to_correlate <- c(first_order_predictors, second_order_predictor_names)
correlation_matrix <- abs(cor( profiling_df[,terms_to_correlate], profiling_df[,metric_name], method = "spearman"));

# ----------- prune the predictors to simplify the problem

# prune all the terms below the correlation threshold
selection_vector <- correlation_matrix > correlation_threshold
names(correlation_matrix) <- rownames(selection_vector)
correlation_matrix <- c(correlation_matrix[selection_vector])

# prune all the second order terms which are not better of their first order terms
valid_predictors <- c()
for( i in 1:length(correlation_matrix))
{
  # get the basic component of the term
  predictor_name <- names(correlation_matrix)[i]
  terms <- strsplit(predictor_name, second_order_predictors_separator)[[1]]

  # check if it is a second order predictor (NOTE: not sure it is a good thing)
  if (length(terms) > 1)
  {
    # get the max correlation of its basic predictors
    max_correlation_single <- max(correlation_matrix[terms], na.rm = TRUE)

    # check if the second order predictor should be considered
    if (correlation_matrix[i] > max_correlation_single)
    {
      valid_predictors <- c( valid_predictors, TRUE )
    }
    else
    {
      valid_predictors <- c( valid_predictors, FALSE )
    }
  }
  else
  {
    # we must keep the good first order predictors
    valid_predictors <- c( valid_predictors, TRUE )
  }
}
correlation_matrix <- correlation_matrix[valid_predictors]
print("[INFO]   Useful predictor(s):")
print(correlation_matrix)
useful_predictor_names <- names(correlation_matrix)


# ----------- prune the predictors to simplify the problem
# PS: it also plots some informations about it

# compose the text of the formula
text_of_formula <- paste(metric_name, "~", paste(useful_predictor_names, collapse= " + "), sep = " ")

# generate the actual formula
metric_formula <- as.formula(text_of_formula)

# generate the model
#degrees <- rep(splines_degrees, length(correlation_matrix))
#segments <- rep(5, length(correlation_matrix))
#metric_model <- crs(metric_formula, data = profiling_df, degree = degrees, segments = segments, knots = "quantiles", basis = "additive" )
metric_model <- crs(metric_formula, data = profiling_df, prune = TRUE, kernel = FALSE )
print(summary(metric_model))
pdf(paste(root_path, "/plot_", metric_name, ".pdf", sep = ""))
plot(metric_model)
dev.off()


# ----------- perform the predictions according to the generated model

# enhance the output dataframe with infomations regarding the useful predictors
for ( i in 1:length(useful_predictor_names) )
{
  predictor_name <- useful_predictor_names[i]
  terms <- strsplit(predictor_name, second_order_predictors_separator)[[1]]
  if (length(terms) > 1)
  {
    prediction_df <- cbind(prediction_df, prediction_df[,terms[1]] * prediction_df[,terms[2]])
    names(prediction_df)[length(prediction_df)] <- predictor_name
  }
}

# generate the metric mean value
mean_values <- predict(metric_model, newdata = prediction_df)

# generate the metric standard deviation
# by default, rcs computes the upper and lower bound with 95% of
# confidence, this knob is not exposed, therefore, since we want the
# standard deviation, we kind assume that it is a gaussian, beacuse
# it means that the 95% of confidence is 2 sigma. So, with take the
# delta between the mean and the upper bound and divide it by half.
# crude, but kind of necessary
stdandard_deviations <- (attr(mean_values, "upr") - mean_values) / 2

# combine the prediction in a single table
output_df <- prediction_df
output_df$Mean <- mean_values
output_df$Std <- stdandard_deviations










#######################################################################
#### WRITE THE RESULT OF THE PREDICTION
#######################################################################
# This part of the application store the predicted value of the metric
#
# INPUT:
# ---------------------------------------------------------------------
# output_df :   The prediction vector
# metric_name : The name of the target metric

write.table(output_df, file = paste(root_path, "prediction.txt", sep = "/"), sep = ",", row.names = FALSE, col.names = TRUE)
