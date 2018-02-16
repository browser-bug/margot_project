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


  class CassandraClient: public FsHandler
  {
    private:

      bool is_connected;
      CassSession* session;


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


    public:

      CassandraClient(const std::string& url, const std::string& username = "", const std::string& password = "");
      ~CassandraClient( void );

  };


}

#endif // MARGOT_CASSANDRA_DATASTAX_FS_IMPLEMENTATION_HDR
