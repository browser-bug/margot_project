/* agora/cassandra_fs_implementation.hpp
 * Copyright (C) 2018 Davide Gadioli
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef MARGOT_AGORA_CASSANDRA_FS_IMPLEMENTATION_HDR
#define MARGOT_AGORA_CASSANDRA_FS_IMPLEMENTATION_HDR

#include <string>
#include <cassert>

extern "C"
{
#include <cassandra.h>
}


#include "fs_handler.hpp"
#include "logger.hpp"

namespace margot
{


  // TODO: batch insert for populating the tables
  class CassandraClient: public FsHandler
  {
    private:

      bool is_connected;
      CassSession* session;
      const std::string database_name;
      const char default_application_separator;
      const char table_application_separator;

      std::string address;
      std::string username;
      std::string password;


      // send the query to database and check returning code
      CassFuture* send_query( const std::string& query );


      // for the free problem, we can't return the result
      void execute_query_synch( const std::string& query );


      // use this version if you actually would like to process the result
      template< class T >
      inline void execute_query_synch( const std::string& query, T& functor )
      {
        // execute the query
        CassFuture* query_future = send_query(query);

        // process the result of the query
        functor(cass_future_get_result(query_future));

        // free the future
        cass_future_free(query_future);
      }

      void store_metrics( const std::string& application_name, const application_metrics_t& metrics );
      application_metrics_t load_metrics( const std::string& application_name );

      void store_knobs( const std::string& application_name, const application_knobs_t& knobs );
      application_knobs_t load_knobs( const std::string& application_name );

      void store_features( const std::string& application_name, const application_features_t& features );
      application_features_t load_features( const std::string& application_name );


    public:

      CassandraClient(const std::string& url, const std::string& username = "", const std::string& password = "");
      ~CassandraClient( void );

      void store_description( const application_description_t& description )
      {
        store_metrics(description.application_name, description.metrics);
        store_features(description.application_name, description.features);
        store_knobs(description.application_name, description.knobs);
      }
      application_description_t load_description( const std::string& application_name )
      {
        return { application_name,
                 load_knobs(application_name),
                 load_features(application_name),
                 load_metrics(application_name)};
      }

      void store_model( const application_description_t& description, const model_t& model );
      model_t load_model( const std::string& application_name );

      void store_doe( const application_description_t& description, const doe_t& doe );
      doe_t load_doe( const std::string& application_name );
      void update_doe( const application_description_t& description, const std::string& values );

      void create_trace_table( const application_description_t& description );
      void insert_trace_entry( const application_description_t& description, const std::string& values );

      void erase( const std::string& application_name );


      std::string get_type( void ) const
      {
        return "CASSANDRA";
      }
      std::string get_address( void ) const
      {
        return address;
      }
      std::string get_username( void ) const
      {
        return username;
      }
      std::string get_password( void ) const
      {
        return password;
      }
      std::string get_observation_name( const std::string& application_name ) const
      {
        std::string table_name = application_name;
        std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
        return database_name + "." + table_name + "_trace";
      }
      std::string get_model_name( const std::string& application_name ) const
      {
        std::string table_name = application_name;
        std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
        return database_name + "." + table_name + "_model";
      }

      std::string get_knobs_name( const std::string& application_name ) const
      {
        std::string table_name = application_name;
        std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
        return database_name + "." + table_name + "_knobs";
      }
      std::string get_features_name( const std::string& application_name ) const
      {
        std::string table_name = application_name;
        std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
        return database_name + "." + table_name + "_features";
      }
  };


}

#endif // MARGOT_CASSANDRA_DATASTAX_FS_IMPLEMENTATION_HDR
