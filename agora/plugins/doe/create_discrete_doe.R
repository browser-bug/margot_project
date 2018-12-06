create_doe <- function(knobs_config_list, doe_options, knobs_names, model_container_name, metric_names, limits = NULL, algorithm = "dmax", storage_type = "CSV") {
  suppressMessages(suppressPackageStartupMessages(library(DiceDesign)))
  
  full_design <- expand.grid(knobs_config_list)
  names(full_design) <- knobs_names

# Restrict design space ---------------------------------------------------

  if (!is.null(limits)) {
    if (any(grepl("system", limits))) {
      stop("Error: No funny plays with system calls through constraints evaluation are allowed. In case you did not meant to do system call, please do not use knobs with 'system' in the name.")
    }
    discarded_designs <- full_design
    for (limit_iter in limits) {
      full_design <- full_design %>% filter(!!parse_quo(limit_iter, env = environment()))
    }
    discarded_designs <- discarded_designs %>% setdiff(full_design)
  }
  
  if(doe_options$nobs >= nrow(full_design))

# Write model table -------------------------------------------------------

  if (storage_type == "CASSANDRA"){
    for (row.ind in 1:nrow(full_design))
    {
      set_columns <- str_c(c(knobs_names, str_c(metric_names, "_avg"), str_c(metric_names, "_std")), collapse = ", ")
      set_values <- str_c(c(full_design[row.ind, ], rep(NA, length(metric_names)*2)), collapse = ", ")
      dbSendUpdate(conn, str_c("INSERT INTO ", model_container_name, "(", set_columns, ") VALUES (", set_values, ")"))
    }
  } else if (storage_type == "CSV"){
    temp_df <- full_design
    for(metric_name in metric_names){
      metric_avg <- str_c(metric_name, "_avg")
      metric_std <- str_c(metric_name, "_std")
      temp_df <- temp_df %>% mutate(!!metric_avg := NA)
      temp_df <- temp_df %>% mutate(!!metric_std := NA)
    }
    write.table(temp_df, file = model_container_name, col.names = TRUE, row.names = FALSE, sep = ",", dec = ".")
  }
  
  if(doe_options$nobs >= nrow(full_design)){
    algorithm = "full_factorial"
  }

# Generate points to explore ----------------------------------------------
  
  if (algorithm == "full_factorial") {
    design <- full_design
  } else {
    # Set number of dimensions based on the number of knobs in the input list
    ndim <- length(knobs_config_list)
    
    # Set options from the options list
    nobs <- min(doe_options$nobs, nrow(full_design))
    eps <- doe_options$eps
    
    # Create design in [0,1]^n space
    design <- switch(algorithm,
                     strauss = straussDesign(nobs, ndim, eps),
                     dmax = dmaxDesign(nobs, ndim, eps),
                     lhs = lhsDesign(nobs, ndim),
                     wsp = {
      initial_design <- dmaxDesign(nobs, ndim, eps)$design
      wspDesign(initial_design, eps)}, 
                     factorial3 = {
      list(design = unique(t(combn(rep(c(1, 0, 0.5), ndim), ndim))))},
                     factorial5 = {
      list(design = unique(t(combn(rep(c(1, 0, 0.25, 0.75, 0.5), ndim), ndim))))
    })
    
    design <- design$design
    # Transform information from the knobs_config_list into matrix
    knob_transform <- sapply(knobs_config_list, function(knob_config) {
      knob_adjusted_max <- max(knob_config) - min(knob_config)
      knob_min <- min(knob_config)
      return(c(knob_min, knob_adjusted_max))
    })
    
    # Upscale design to the input space
    for (i in 1:ndim) {
      design[, i] <- design[, i] * knob_transform[2, i] + knob_transform[1, i]
    }

# Map points to the application DOE ---------------------------------------
    
    if (length(knobs_config_list) > 1) {
      # Create matrix of all the possible knob design points
      design_space_grid <- as.matrix(full_design)
      
      # Give design_space_grid row names of 1 to nobs
      row.names(design_space_grid) <- 1:dim(design_space_grid)[1]
      
      # Find the index of the point closes to the 1-st design point
      ind <- row.names(design_space_grid)[which.min(rowSums(abs(t(apply(design_space_grid, 1, function(x) {
        x - design[1, ]
      })))))]
      for (i in 2:dim(design)[1]) {
        # Find the index of the point closest to the i-th design point, discard already selected designs
        ind_temp <- which.min(rowSums(abs(t(apply(design_space_grid[-as.numeric(ind), ], 1, function(x) {
          x - design[i, ]
        })))))
        # Add row name of the point closest to the i-th design point from the grid to the index vector
        ind <- c(ind, row.names(design_space_grid[-as.numeric(ind), ])[ind_temp])
      }
      # Subset selected design points
      design <- design_space_grid[as.numeric(ind), ]
    } else {
      warning("Your knobs_config_list has only one knob.")
      
      design_space_grid <- knobs_config_list[[1]]
      names(design_space_grid) <- 1:length(design_space_grid)
      # Find the index of the point closes to the 1-st design point
      ind <- names(design_space_grid)[which.min(abs(design_space_grid - design[1, ]))]
      for (i in 2:length(design)) {
        # Find the index of the point closest to the i-th design point, discard already selected designs
        ind_temp <- which.min(abs(design_space_grid[-as.numeric(ind)] - design[i, ]))
        # Add row name of the point closest to the i-th design point from the grid to the index vector
        ind <- c(ind, names(design_space_grid[-as.numeric(ind)])[ind_temp])
      }
      # Subset selected design points
      design <- design_space_grid[as.numeric(ind)]
    }
    
  }
  
  return(design)
}
