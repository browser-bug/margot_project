fit_models_agora <- function(observation_df, input_columns, metric_name, nobserved)
{
  library("DiceEval")
  
  # Set X as variable and Y as results
  X <- observation_df[, input_columns]
  Y <- observation_df[, metric_name]
  
  # Check whether the results are not constant. This should not happen, but it will break the models.
  if(length(unique(Y)) == 1)
  {
    stop("Error: Results at the selected points are constant. Please check your data/model.")
  }
  
  # Enforce X as a matrix, it is a vector if there is only one knob and models expect matrix
  if(is.null(dim(X)))
  {
    X <- as.matrix(X, ncol = 1)
  }
  
  # Enforce Y as a matrix
  if(is.null(dim(Y)))
  {
    Y <- as.matrix(Y, ncol = 1)
  }
  
  # Name columns of X by first letters of alphabet
  colnames(X) <- letters[1:length(input_columns)]
  
  # Prepare formula for the linear models
  linear_formula <- paste(metric_name, "~ .")
  linear_formula_second_interactions <- paste( metric_name, "~ . + .^2" )
  
  # Fit models
  model_linear <- lm( formula = linear_formula, data = observation_df, y = TRUE )
  model_linear2 <- lm( formula = linear_formula_second_interactions, data = observation_df, y = TRUE )
  model_mars <- mars( x = X, y = Y, degree = 2 )
  model_polymars <- polymars( predictors = X, responses = Y, gcv = 3 )
  # model_polymars_additive <- polymars(predictors = X, responses = Y, gcv = 3, additive = TRUE)
  
  # Create empty variable validation of the type list, it allows for the easy adding of results as multiple level named lists
  validation <- list()
  
  # Set the number of the folds to minimum of 10 or number of observations
  nfolds <- min( nobserved, 10 )
  if( nfolds < 10 )
  {
    warning("There is less than 10 observations in the TRACE table, the results may be very inconsistent")
  }
  
  validation$linear$R2 <- matrix(NA, nrow = 1, ncol = nfolds + 1 )
  validation$linear2$R2 <- matrix(NA, nrow = 1, ncol = nfolds + 1 )
  validation$mars$R2 <- matrix(NA, nrow = 1, ncol = nfolds + 1 )
  validation$polymars$R2 <- matrix(NA, nrow = 1, ncol = nfolds + 1 )
  
  # Compute MAE for the models
  validation$linear$MAE <- matrix(NA, nrow = 1, ncol = nfolds + 1 )
  validation$linear2$MAE <- matrix(NA, nrow = 1, ncol = nfolds + 1 )
  validation$mars$MAE <- matrix(NA, nrow = 1, ncol = nfolds + 1 )
  validation$polymars$MAE <- matrix(NA, nrow = 1, ncol = nfolds + 1 )
  
  # Store coefficients for the models
  validation$linear$coeff <- matrix(NA, nrow = length(model_linear$coefficients), ncol = nfolds + 1 )
  validation$linear2$coeff <- matrix(NA, nrow = length(model_linear2$coefficients), ncol = nfolds + 1 )
  validation$mars$coeff <- matrix(NA, nrow = 12, ncol = nfolds + 1 )  # BEWARE, the number of rows is arbitrarily set to 12, it possibly may happen, there will be more needed, but it is highly unlikely
  validation$polymars$coeff <- matrix(NA, nrow = 8, ncol = nfolds + 1 ) # BEWARE, the number of rows is arbitrarily set to 8, it possibly may happen, there will be more needed, but it is highly unlikely

  # Compute model R2, MAE and store coefficient -----------------------------------------------------------------------------------
  
  # Compute R2 for the models
  validation$linear$R2[, 1] <- R2(model_linear$y, model_linear$fitted.values)^2
  validation$linear2$R2[, 1] <- R2(model_linear2$y, model_linear2$fitted.values)^2
  validation$mars$R2[, 1] <- R2(Y, model_mars$fitted.values)^2
  validation$polymars$R2[, 1] <- model_polymars$Rsquared
  
  # Compute MAE for the models
  validation$linear$MAE[, 1] <- MAE(model_linear$y, model_linear$fitted.values)
  validation$linear2$MAE[, 1] <- MAE(model_linear2$y, model_linear2$fitted.values)
  validation$mars$MAE[, 1] <- MAE(Y, model_mars$fitted.values)
  validation$polymars$MAE[, 1] <- MAE(Y, model_polymars$fitted)
  
  # Store coefficients for the models
  validation$linear$coeff[, 1] <- model_linear$coefficients
  validation$linear2$coeff[, 1] <- model_linear2$coefficients
  validation$mars$coeff[model_mars$selected.terms, 1] <- model_mars$coefficients
  validation$polymars$coeff[1:model_polymars$model.size, 1] <- model_polymars$model$coefs
  
  # Compute cross-validation on the models -----------------------------------------------------------------------------------
  print("I make cross-validation now.")
  
  cv_sequence <- floor( seq( 0, nobserved, length.out = nfolds + 1 ) )
  
  for(i in 1:nfolds)
  {
    ind <- cv_sequence[i] < 1:nobserved & cv_sequence[i+1] >= 1:nobserved
    
    train_data <- observation_df[!ind, ]
    test_data <- observation_df[ind, ]
    
    X_train <- train_data[, knobs_names]
    Y_train <- train_data[, metric_name]
    X_test <- test_data[, knobs_names]
    Y_test <- test_data[, metric_name]
    
    model_linear_cv <- lm( formula = linear_formula, data  = train_data, y = TRUE )
    model_linear2_cv <- lm( formula = linear_formula_second_interactions, data = train_data, y = TRUE )
    model_mars_cv <- mars( x = X_train, y = Y_train, degree = 2 )
    model_polymars_cv <- polymars( predictors = X_train, responses = Y_train, gcv = 3 )
    
    predict_linear_cv <- predict(model_linear_cv, X_test, interval = "confidence")
    predict_linear2_cv <- predict(model_linear2_cv, X_test, interval = "confidence")
    predict_mars_cv <- predict(model_mars_cv, X_test)
    predict_polymars_cv <- predict(model_polymars_cv, X_test)
    # 
    # # Compute R2 for the models, R2 function package DiceEval
    # validation$linear$R2[, (i+1)] <- R2(model_linear_cv$y, model_linear_cv$fitted.values)
    # validation$linear2$R2[, (i+1)] <- R2(model_linear2_cv$y, model_linear2_cv$fitted.values)
    # validation$mars$R2[, (i+1)] <- R2(Y_train, model_mars_cv$fitted.values)
    # validation$polymars$R2[, (i+1)] <- model_polymars_cv$Rsquared
    # 
    # # Compute MAE for the models, MAE function package DiceEval
    # validation$linear$MAE[, (i+1)] <- MAE(model_linear_cv$y, model_linear_cv$fitted.values)
    # validation$linear2$MAE[, (i+1)] <- MAE(model_linear2_cv$y, model_linear2_cv$fitted.values)
    # validation$mars$MAE[, (i+1)] <- MAE(Y_train, model_mars_cv$fitted.values)
    # validation$polymars$MAE[, (i+1)] <- MAE(Y_train, model_polymars_cv$fitted)
    # 
    # Compute R2 for the models, R2 function package DiceEval
    validation$linear$R2[, (i+1)] <- R2(Y_test, predict_linear_cv[, 1])^2
    validation$linear2$R2[, (i+1)] <- R2(Y_test, predict_linear2_cv[, 1])^2
    validation$mars$R2[, (i+1)] <- R2(Y_test, predict_mars_cv)^2
    validation$polymars$R2[, (i+1)] <- R2(Y_test, predict_polymars_cv)^2
    
    # Compute MAE for the models, MAE function package DiceEval
    validation$linear$MAE[, (i+1)] <- MAE(Y_test, predict_linear_cv[, 1])
    validation$linear2$MAE[, (i+1)] <- MAE(Y_test, predict_linear2_cv[, 1])
    validation$mars$MAE[, (i+1)] <- MAE(Y_test, predict_mars_cv)
    validation$polymars$MAE[, (i+1)] <- MAE(Y_test, predict_polymars_cv)
    
    # print(model_mars_cv$selected.terms)
    # print(model_mars_cv$all.terms)
    
    # Store coefficients for the models
    validation$linear$coeff[, (i+1)] <- model_linear_cv$coefficients
    validation$linear2$coeff[, (i+1)] <- model_linear2_cv$coefficients
    validation$mars$coeff[model_mars_cv$selected.terms ,(i+1)] <- model_mars_cv$coefficients
    validation$polymars$coeff[1:model_polymars_cv$model.size, (i+1)] <- model_polymars_cv$model$coefs
  }
  
  # Compute kriging -----------------------------------------------------------------------------------------------------------
  
  # If the X has more than one column ( there is more than one knob ) use the kriging model - it does not work with one dimensional input
  if(dim(X)[2] > 1)
  {
    kriging_df <- as.data.frame( grouped_df( observation_df, knobs_names ) %>% summarise( mean = mean( !!sym( metric_name ) ), sd = sd( !!sym( metric_name ) ) ) )
    X <- kriging_df[, knobs_names]
    Y <- kriging_df$mean
    kriging_df$sd[is.na(kriging_df$sd)] <- 0
    print("I estimate kriging now.")
    tryCatch({
      model_kriging <- km( formula = ~ ., design = X, response = Y, noise.var = kriging_df$sd )
      
      kriging_fit <- predict(model_kriging, newdata = X, type = "UK")$mean
      if (sum(abs(Y - kriging_fit)) >= 1e-09) {
        print("Warning: Kriging model doesn't interpolate the data.")
      }
      
      # Set the number of the folds to minimum of 10 or number of observations in kriging_df
      kriging_nobserved <- dim(kriging_df)[1]
      nfolds <- min( kriging_nobserved, 10 )
      if( nfolds < 10 )
      {
        print("There is less than 10 observations in the TRACE table, the results may be very inconsistent. (Kriging)")
      }
      
      validation$kriging$R2 <- matrix(NA, nrow = 1, ncol = nfolds + 1 )
      
      # Compute MAE for the models
      validation$kriging$MAE <- matrix(NA, nrow = 1, ncol = nfolds + 1 )
      
      # Store coefficients for the models
      validation$kriging$coeff <- matrix(NA, nrow = length( c( model_kriging@trend.coef,  model_kriging@covariance@range.val ) ), ncol = nfolds + 1 )
      
      
      # Compute R2 for the kriging
      validation$kriging$R2[, 1] <- R2(Y, kriging_fit)^2
      
      # Compute MAE for the kriging
      validation$kriging$MAE[, 1] <- MAE(Y, kriging_fit)
      
      # Store coefficients for the kriging
      validation$kriging$coeff[, 1] <- c( model_kriging@trend.coef, model_kriging@covariance@range.val )
      
      
      # Cross validation kriging
      
      print("I make cross-validation on kriging now.")
      
      cv_sequence <- floor( seq( 0, kriging_nobserved, length.out = nfolds + 1 ) )
      
      for(i in 1:nfolds)
      {
        ind <- cv_sequence[i] < 1:kriging_nobserved & cv_sequence[i+1] >= 1:kriging_nobserved
        
        train_data <- kriging_df[!ind, ]
        test_data <- kriging_df[ind, ]
        
        X_train <- train_data[, knobs_names]
        Y_train <- train_data[, "mean"]
        sd_train <- train_data$sd
        X_test <- test_data[, knobs_names]
        Y_test <- test_data[, "mean"]
        
        model_kriging_cv <- km( formula = ~ ., design = X_train, response = Y_train, noise.var = sd_train )
        
        # kriging_fit <- predict(model_kriging_cv, newdata = X, type = "UK")$mean
        # if (sum(abs(Y - kriging_fit)) >= 1e-09) {
        #   warning("Warning: Kriging model doesn't interpolate the data.")
        # }
        
        predict_kriging_cv <- predict(model_kriging_cv, newdata = X_test, type = "UK")$mean
        
        # Compute R2 for the models, R2 function package DiceEval
        validation$kriging$R2[, (i+1)] <- R2(Y_test, predict_kriging_cv)^2
        
        # Compute MAE for the models, MAE function package DiceEval
        validation$kriging$MAE[, (i+1)] <- MAE(Y_test, predict_kriging_cv)
        
        # Store coefficients for the models
        validation$kriging$coeff[, (i+1)] <- c( model_kriging_cv@trend.coef, model_kriging_cv@covariance@range.val )
      }
    },
    error = function(e) print(e))
  }
  
  # Name the columns of each object in the validation list
  validation <- lapply(validation,
                       function(x) lapply(x,
                                          function(y){
                                            nval <- dim(y)[2] - 1
                                            colnames(y) <- c("Model", paste("CV", c(1:(nval)), sep = ""))
                                            return(y)}
                       )
  )
  
  # Store model into the validation list
  validation$linear$model <- model_linear
  validation$linear2$model <- model_linear2
  validation$mars$model <- model_mars
  validation$polymars$model <- model_mars
  if(!is.null(validation$kriging))
  {validation$kriging$model <- model_kriging}
  
  return(validation)
}