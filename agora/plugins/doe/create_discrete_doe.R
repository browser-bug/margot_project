create_doe <- function(knobs_config_list, doe_options, map_to_input = TRUE, algorithm = "strauss")
{
  library(DiceDesign)
  
  # Set number of dimensions based on the number of knobs in the input list
  ndim <- length(knobs_config_list)
  
  # Get number of values for each knob
  config_lengths <- sapply(knobs_config_list, function(x)length(x))
  
  # Set options from the options list
  nobs <- min(doe_options$nobs, prod(config_lengths))
  eps  <- doe_options$eps
  
  # Create design in [0,1]^n space 
  design <- switch(algorithm,
                     strauss = straussDesign(nobs, ndim, eps),
                     dmax    = dmaxDesign(nobs, ndim, eps),
                     lhs     = lhsDesign(nobs, ndim))
  
  design <- design$design
  
  # Transform information from the knobs_config_list into matrix
  knob_transform <- sapply(knobs_config_list,
                           function(knob_config)
                             {
                             knob_adjusted_max <- max(knob_config) - min(knob_config)
                             knob_min <- min(knob_config)
                             return(c(knob_min, knob_adjusted_max))
                           })
  
  # Upscale design to the input space
  for(i in 1:ndim)
  {
    design[, i] <- design[, i] * knob_transform[2, i] + knob_transform[1, i]
  }
  
  # if(is.null(dim(design)))
  
  
  # If discrete settings, map design points to the closest possible option based on the knobs_config_list points
  if(map_to_input)
  {

    if( length( knobs_config_list ) > 1 )
    {
      # Create matrix of all the possible knob design points
      design_space_grid <- expand.grid(knobs_config_list)
      
      # Give design_space_grid row names of 1 to nobs
      row.names(design_space_grid) <- 1:dim(design_space_grid)[1]
      
      # Find the index of the point closes to the 1-st design point
      ind <- row.names(design_space_grid)[which.min( rowSums( abs(design_space_grid - design[1, ]) ) )]
      for(i in 2:nobs)
      {
        # Find the index of the point closest to the i-th design point, discard already selected designs
        ind_temp <- which.min( rowSums( abs( design_space_grid[-as.numeric(ind), ] - design[i, ]) ) )
        # Add row name of the point closest to the i-th design point from the grid to the index vector
        ind <- c(ind, row.names(design_space_grid[-as.numeric(ind), ])[ind_temp] )
      }
    # Subset selected design points
    design <- design_space_grid[as.numeric(ind), ]
    } else
    {
      warning("Your knobs_config_list has only one knob.")
      
      design_space_grid <- knobs_config_list[[1]]
      names(design_space_grid) <- 1:length(design_space_grid)
      # Find the index of the point closes to the 1-st design point
      ind <- names(design_space_grid)[which.min( abs(design_space_grid - design[1, ])  )]
      for(i in 2:nobs)
      {
        # Find the index of the point closest to the i-th design point, discard already selected designs
        ind_temp <- which.min( abs( design_space_grid[-as.numeric(ind)] - design[i, ]) )
        # Add row name of the point closest to the i-th design point from the grid to the index vector
        ind <- c(ind, names(design_space_grid[-as.numeric(ind)])[ind_temp] )
      }
      # Subset selected design points
      design <- design_space_grid[as.numeric(ind)]
    }
    
  }
  
  return(design)
  
}
