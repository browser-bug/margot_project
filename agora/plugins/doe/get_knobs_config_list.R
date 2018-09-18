get_knobs_config_list <- function(storage_type, knobs_container_name, conn = NULL) {
  
  if (storage_type == "CASSANDRA") {
    knobs_config_list <- dbGetQuery(conn, paste("SELECT values FROM ", knobs_container_name, sep = ""))
    
    knobs_config_list <- unlist(apply(as.matrix(knobs_config_list, nrow = 1), 1, function(x) {
      lapply(strsplit(substring(as.character(x), 2, nchar(as.character(x)) - 1), ", "), function(y) as.numeric(y))
    }), recursive = FALSE)
  }
  return(knobs_config_list)
}