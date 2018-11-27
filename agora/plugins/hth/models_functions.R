# FIT MODEL --------------------------------------------------------------

fit_selected_model <- function(model_name, training_data, inputs_columns, metric_name) {

  if(grepl("mars", model_name)) {
    X <- as.matrix(training_data[, input_columns])
    Y <- as.matrix(training_data[, metric_name])
  }
  
  if(grepl("kriging", model_name)) {
    # KRIGING CAN TAKE ONLY ONE OBSERVATION PER CONFIGURATION
    # THEREFORE IT IS NECESSARY TO TAKE MEAN AND SD OF OBSERVATIONS
    training_data <- training_data %>%
      group_by_at(input_columns) %>%
      summarize(mean = mean(!!sym(metric_name)), sd = sd(!!sym(metric_name)))
    # IF THERE IS ONLY 1 OBSERVATION SD IS NA, CHANGE IT TO 0
    training_data <- training_data %>% mutate(sd = replace(sd, is.na(sd), 0))
    
    X <- training_data %>% select(input_columns) %>% as.matrix
    Y <- training_data %>% pull(mean)
    
    if(all(near(0, training_data$sd))) {
    } else {
      noise.var <- training_data %>% pull(sd)
    }
  }
  
  fitted_model <- switch(as.character(model_name),
               linear = lm(formula = paste(metric_name, "~ ."), data = training_data),
               linear2 = lm(formula = paste( metric_name, "~ . + .^2" ), data = training_data),
               mars = mars(x = X, y = Y, degree = 2),
               polymars = polymars(predictors = X, responses = Y, gcv = 3),
               kriging = {
                 sink("kriging.log", append = T)
                 if(exists("noise.var")){
                   model_fit <- km(formula = ~ ., design = X, response = Y, noise.var = noise.var)
                 } else {
                   model_fit <- km(formula = ~ ., design = X, response = Y)  
                 }
                 sink()
                 return(model_fit)
                 },
                stop("Error: Unknown model name in model fitting function!")
             )
  
  return(fitted_model)
}

# PREDICT MODEL -----------------------------------------------------------

predict_selected_model <- function(model_name, model_fit, sub_cv_df, new_data, models_df = NA, stacking_weights = NA){ 

  if(grepl("bagging", model_name)){model_name <- "bagging"}
  switch(as.character(model_name),
         linear = {
           Y_hat <- predict(model_fit, new_data, se.fit = TRUE, interval = "confidence")
           list(fit = Y_hat$fit[, 1], sd = Y_hat$se.fit)},
         
         linear2 = {
           Y_hat <- predict(model_fit, new_data, se.fit = TRUE, interval = "confidence")
           list(fit = Y_hat$fit[, 1], sd = Y_hat$se.fit)},
         
         mars = {
           Y_hat <- predict(model_fit, new_data)
           list(fit = Y_hat, sd = -1)},
         
         polymars = {
           Y_hat <- predict(model_fit, as.matrix(new_data))
           list(fit = Y_hat, sd = -1)},
         
         kriging = {
           Y_hat <- predict(model_fit, newdata = new_data, type = "UK")
           list(fit = Y_hat$mean, sd = Y_hat$sd)},
         
         bagging = {
           if(!exists("sub_cv_df")){stop("Error: Bagging has not received cross validated models for prediction.")}
           Y_hat <- sub_cv_df %>% 
             mutate(predictions = map2(models,
                                       fitted_models,
                                       predict_selected_model,
                                       NA,
                                       new_data)) %>%
            mutate(pred_obs = map(predictions, function(x)x$fit)) %>%
            unnest(pred_obs) %>%
            group_by(models, fold) %>%
            mutate(grouped_id = row_number()) %>%
            group_by(models, grouped_id) %>%
            summarize(pred_obs = mean(pred_obs)) %>%
            pull(pred_obs)
            list(fit = Y_hat, sd = -1)  
            },
         stacking = {
           # if(!exists("models_df")){stop("Stacking has not received base models for prediction.")}
           # if(!exists("stacking_weights")){stop("Stacking has not received weights for base models ensemble.")}
           if(is.na("models_df")){stop("Error: Stacking has not received base models for prediction.")}
           if(is.na("stacking_weights")){stop("Error: Stacking has not received weights for base models ensemble.")}
           Y_hat <- models_df %>% 
             mutate(predictions = map2(models,
                                       fitted_models,
                                       predict_selected_model,
                                       NA,
                                       new_data)) %>%
             mutate(pred_obs = map(predictions, function(x)x$fit)) %>%
             unnest(pred_obs) %>%
             group_by(models) %>%
             mutate(grouped_id = row_number()) %>%
             select(models, pred_obs, grouped_id) %>%
             spread(key = models, value = pred_obs, drop = TRUE) %>%
             select(-grouped_id) %>%
             t() %>%
             magrittr::multiply_by(stacking_weights) %>%
             t() %>%
             as.tibble() %>%
             mutate(Y_fit = rowSums(.)) %>%
             pull(Y_fit)
           list(fit = Y_hat, sd = -1) 
           },
         stop("Error: Unknown model name in model prediction.")
         )
}

# CREATE MODEL LIST -------------------------------------------------------

model_list <- function(ninputs, zero_range_variable) {
  if(ninputs == 1 | zero_range_variable){
    used_models <- tibble(models = c("linear",
                                     "linear2",
                                     "mars",
                                     "polymars"))
  } else if(ninputs > 1){
    used_models <- tibble(models = c("linear",
                                     "linear2",
                                     "mars",
                                     "polymars",
                                     "kriging"))
  } else {
    stop("Error: Wrong number of input columns for model list creation.")
  }
}

# CROSS VALIDATION --------------------------------------------------------

cross_validation <- function(used_models, nfolds, observation_df, configuration_df, input_columns, metric_name) {
  
  cv_df <- as.tibble(expand.grid(used_models, 1:nfolds)) 
  colnames(cv_df) <- c("models", "fold")
  cv_df <- cv_df %>% mutate(models = as.character(models))  
  
  folds <- cut(seq(1, nrow(configuration_df)), breaks=nfolds, labels=FALSE)
  
  cv_df <- cv_df %>% mutate(fitted_models = map2(models,
                                                 fold,
                                                 fit_cross_validation_model,
                                                 observation_df,
                                                 configuration_df,
                                                 folds,
                                                 input_columns,
                                                 metric_name
                                                 )
                            )
  
  cv_df <- cv_df %>% mutate(measures = pmap(list(model_name = models,
                                            model_fit = fitted_models,
                                            fold = fold),
                                            possibly(compute_cv_measures, list(R2 = NA, MAE = NA, predicted_values = NA)),
                                            observation_df,
                                            configuration_df,
                                            folds,
                                            input_columns,
                                            metric_name)
                            )
  
}

fit_cross_validation_model <- function(model_name, fold, observation_df, configuration_df, folds, input_columns, metric_name){
  
  training_data <- inner_join(configuration_df %>% filter(folds != fold), observation_df, by = input_columns)
  fit_selected_model(model_name, training_data, input_columns, metric_name) 
}

compute_cv_measures <- function(model_name, model_fit, fold, observation_df, configuration_df, folds, input_columns, metric_name){
  
  test_configurations <- configuration_df %>% filter(folds == fold)
  list_predictions <- predict_selected_model(model_name, model_fit, NA, test_configurations)
  
  test_observations <- inner_join(observation_df, bind_cols(test_configurations, fit = list_predictions$fit), by = input_columns)
  R2 <- cor(test_observations$fit, test_observations %>% pull(metric_name))^2
  MAE <- mean(abs(test_observations$fit - (test_observations %>% pull(metric_name))))
  
  return(list(R2 = R2, MAE = MAE, predicted_values = list_predictions$fit))
}

extract_measure_cv <- function(model_name, measure, cv_df){
  cv_df %>%
    filter(models == model_name) %>%
    mutate(value = map_dbl(measures, function(x)x[[measure]])) %>%
    group_by(models) %>%
    summarize(res = mean(value, na.rm = TRUE)) %>%
    pull(res)
}

# STACKING ----------------------------------------------------------------

create_stacking_df <- function(cv_df, configuration_df, observation_df, input_columns){
  # THIS FUNCTIONS EXTRACT PREDICTIONS OF INDIVIDUAL CV MODELS
  # THEN GROUP MODELS BY MODEL NAMES AND CREATE DUMMY ROW NUMBER VARIABLE
  # SO THE SPREAD HAS UNIQUE IDENTIFICATOR FOR EACH ROW WHEN SPREADING DATA INTO
  # COLUMNS, OTHERWISE IT WOULD FAIL ON DUPLICATE IDENTIFICATOR ERROR
  # THEN THE CONFIGURATIONS ARE BINDED FROM THE CONFIGURATION_DF
  # AND REAL RESULTS ARE TAKEN FROM THE OBSERVATION_DF
  # LASTLY THE INPUT COLUMNS ARE DROPPED SINCE THEY ARE NOT NEEDED 
  # FOR THE OPTIMIZATION ALGORITHM
  stacking_df <- cv_df %>%
    mutate(predicted_values = map(measures, function(x)x$predicted_values)) %>%
    unnest(predicted_values, .drop = TRUE) %>%
    select(models, predicted_values) %>%
    group_by(models) %>%
    mutate(grouped_id = row_number()) %>%
    spread(key = models, value = predicted_values, convert = TRUE) %>%
    select(-grouped_id) %>%
    bind_cols(configuration_df) %>%
    inner_join(observation_df, by = input_columns) %>%
    select(-input_columns)
}

compute_stacking_weights <- function(stacking_df, d_temp){
  stack_col <- dim(stacking_df)[2]
  tryCatch({
    # Prepare data for quadratic optimization stacking_data <- unique(stacking_data)
    Rinv <- solve(chol(t(stacking_df) %*% stacking_df))
    C <- cbind(rep(1, stack_col), diag(stack_col))
    b <- c(1, rep(0, stack_col))
    d <- t(d_temp) %*% stacking_df
    # Do quadratic optimization and extract solution (weights for each model)
    stacking_weights <- solve.QP(Dmat = Rinv, factorized = TRUE, dvec = d, Amat = C, bvec = b, meq = 1)$solution
  }, error = function(e) print(e))
  if (!exists("stacking_weights"))
  {
    stacking_weights <- rep(1/stack_col, stack_col)
  }
  names(stacking_weights) <- colnames(stacking_df)
  return(stacking_weights)
}

# HOLDOUT SET VALIDATION --------------------------------------------------

holdout_set_validation <- function(models_df, cv_df, stacking_weights, holdout_conf, holdout_test){
  models_holdout_df <- models_df
  models_holdout_df <- models_holdout_df %>%
    bind_rows(list(models = c(str_c("bagging_", models_holdout_df$models), "stacking"))) %>%
    mutate(sub_cv_df = map(models, create_sub_cv_df, cv_df))
  
  models_holdout_df <- models_holdout_df %>%
    mutate(predictions = pmap(list(model_name = models,
                                   model_fit = fitted_models,
                                   sub_cv_df = sub_cv_df),
                              predict_selected_model,
                              holdout_conf,
                              models_df,
                              stacking_weights)) %>%
    mutate(pred_obs = map(predictions, function(x)x$fit)) %>%
    mutate(R2 = map_dbl(pred_obs, function(x, holdout_test)cor(x, holdout_test)^2, holdout_test)) %>%
    mutate(MAE = map_dbl(pred_obs, function(x, holdout_test)mean(abs((x - holdout_test))), holdout_test))
}

create_sub_cv_df <- function(model_name, cv_df)
{
  if(grepl("bagging", model_name)){
    cv_df %>% filter(models == str_replace(model_name, "bagging_", ""))
  } else
  {
    return(NA)
  }
}

holdout_validation <- function(holdout_fold_selected,  configuration_df, holdout_folds, observation_df, metric_name, input_columns, nfolds, error_normalization_value, zero_range_variable) {
  training_conf_df <- configuration_df %>% filter(holdout_folds != holdout_fold_selected)
  training_df <- observation_df %>% inner_join(training_conf_df, by = input_columns)
  holdout_conf <- configuration_df %>% filter(holdout_folds == holdout_fold_selected) %>% inner_join(observation_df, by = input_columns) %>% distinct()
  holdout_test <- holdout_conf %>% pull(metric_name)
  holdout_conf <- holdout_conf %>% select_at(input_columns)
  
  models_df <- model_list(length(input_columns), zero_range_variable)
  models_df <- models_df %>% mutate(fitted_models = map(.x = models, .f = fit_selected_model, training_df, input_columns, metric_name))
  cv_df <- cross_validation(models_df$models, nfolds, training_df, training_conf_df, input_columns, metric_name)
  
  stacking_df <- create_stacking_df(cv_df, training_conf_df, training_df, input_columns)
  stacking_df <- stacking_df %>% na.omit
  d_temp <- stacking_df %>% pull(metric_name)
  stacking_df <- stacking_df %>% select(-metric_name) %>% as.matrix()
  stacking_weights <- compute_stacking_weights(stacking_df, d_temp)
  
  print(stacking_weights)
  
  holdout_validation_df <- holdout_set_validation(models_df, cv_df, stacking_weights, holdout_conf, holdout_test)
  holdout_validation_df <- holdout_validation_df %>% mutate(MAE = MAE / error_normalization_value)
}

# HELPER FUNCTIONS --------------------------------------------------------

# Determine if range of vector is FP 0.
zero_range <- function(x) {
  if (length(x) == 1) return(TRUE)
  x <- range(x) / mean(x)
  isTRUE(near(x[1], x[2]))
}