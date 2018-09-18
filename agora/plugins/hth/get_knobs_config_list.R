get_knobs_config_list <- function(storage_type, knobs_container_name, conn = NULL) {
  
  if (storage_type == "CASSANDRA") {
    knobs_config_list <- dbGetQuery(conn, paste("SELECT values FROM ", knobs_container_name, sep = ""))
    
    knobs_config_list <- unlist(apply(as.matrix(knobs_config_list, nrow = 1), 1, function(x) {
      lapply(strsplit(substring(as.character(x), 2, nchar(as.character(x)) - 1), ", "), function(y) as.numeric(y))
    }), recursive = FALSE)
  } else if ( storage_type == "CSV" ) {
    knobs_config_df <- read.csv(knobs_container_name, stringsAsFactors = FALSE)$value
    knobs_config_list <- strsplit(knobs_config_df, ";")
    knobs_config_list <- lapply(knobs_config_list, function(x)as.numeric(x))
  }
  return(knobs_config_list)
}