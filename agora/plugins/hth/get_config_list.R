get_config_list <- function(storage_type, container_name, conn = NULL) {
  
  if (storage_type == "CASSANDRA") {
    knobs_config_list <- dbGetQuery(conn, paste("SELECT values FROM ", container_name, sep = ""))
    
    knobs_config_list <- unlist(apply(as.matrix(knobs_config_list, nrow = 1), 1, function(x) {
      lapply(strsplit(substring(as.character(x), 2, nchar(as.character(x)) - 1), ", "), function(y) as.numeric(y))
    }), recursive = FALSE)
  } else if ( storage_type == "CSV" ) {
    knobs_config_list <- read_csv(container_name) %>% pull(values)
    knobs_config_list <- str_split(knobs_config_list, ";")
    knobs_config_list <- map(knobs_config_list, function(x)as.numeric(x))
  }
  return(knobs_config_list)
}
