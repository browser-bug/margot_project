get_knobs_config_list <- function(conn, knobs_container_name){
  knobs_config_list <- dbGetQuery(conn, paste("SELECT values FROM ", knobs_container_name, sep = ""))
  
  # Get numeric vectors out of list of character list of values for the knobs
  # knobs_config_list <- unlist(apply(as.matrix(knobs_config_list, nrow = 1),
  #                                   1,
  #                                   function(x)
  #                                   {
  #                                     as.numeric(
  #                                      strsplit(
  #                                       substring(
  #                                         as.character( x ),
  #                                         2,
  #                                         nchar( as.character( x ) ) -1 ),
  #                                         ", " )
  #                                       )
  #                                    }
  #                                   ),
  #                             recursive = FALSE)
  
  knobs_config_list <- unlist(apply(as.matrix(knobs_config_list, nrow = 1),
                                    1,
                                    function(x)
                                    {
                                      lapply(
                                        strsplit(
                                          substring(
                                            as.character( x ),
                                            2,
                                            nchar( as.character( x ) ) -1 ),
                                          ", " ),
                                        function(y)as.numeric(y)
                                      )
                                    }
  ),
  recursive = FALSE)
  return(knobs_config_list)
}