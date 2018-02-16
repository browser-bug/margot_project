/* agora/cassandra_fs_implementation.cc
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

#include <stdexcept>
#include <cassert>

#include "logger.hpp"
#include "cassandra_fs_implementation.hpp"


using namespace margot;

CassandraClient::CassandraClient(const std::string& url, const std::string& username, const std::string& password)
  : is_connected(false)
{
  // initialize the proper data structure
  CassCluster* cluster = cass_cluster_new();
  session = cass_session_new();

  // set the username and password for the connection, if any
  if (!(username.empty() || password.empty()))
  {
    cass_cluster_set_credentials(cluster, username.c_str(), password.c_str());
  }

  // set the address of the database
  cass_cluster_set_contact_points(cluster, url.c_str());

  // connect to the database pretending that it is a serial operation
  CassFuture* connection_result = cass_session_connect(session, cluster);
  CassError rc = cass_future_error_code(connection_result);

  // log the result of the connection
  if (rc == CASS_OK)
  {
    info("Cassandra client: successfully connected to databse at \"", url, "\" as \"", username, "\"");
    is_connected = true;
  }
  else
  {
    warning("Cassandra client: unable to connect to database at \"", url, "\" as \"", username, "\" due to: ", cass_error_desc(rc));
    throw std::runtime_error("Cassandra error: unable to connect to database, due to: " + std::string(cass_error_desc(rc)));
  }

  // free the memory, since we are either connected or not
  cass_future_free(connection_result);
  cass_cluster_free(cluster);

  // create the database, ehm the key space >.>
  execute_query_synch("CREATE KEYSPACE IF NOT EXISTS margot WITH REPLICATION = { 'class' : 'SimpleStrategy', 'replication_factor' : 1 };");

  // switch to the correct database
  execute_query_synch("USE margot");

}


CassandraClient::~CassandraClient( void )
{
  // disconnect from the database
  CassFuture* connection_result = cass_session_close(session);
  cass_future_wait(connection_result); // wait for inflight traffic, might happens
  cass_future_free(connection_result);
}



void CassandraClient::execute_query_synch( const std::string& query )
{
  // execute the query
  CassFuture* query_future = send_query(query);

  // free the future
  cass_future_free(query_future);
}


CassFuture* CassandraClient::send_query( const std::string& query )
{
  // check if we are connected to the cassandra database
  assert(is_connected && "Error: we must be connected in order to perform queries");

  // execute the query
  CassStatement* query_text = cass_statement_new(query.c_str(), 0);
  CassFuture* query_future = cass_session_execute(session, query_text);
  cass_statement_free(query_text);

  // evaluate if it went well
  CassError rc = cass_future_error_code(query_future);
  const bool is_query_ok = rc == CASS_OK;

  if ( is_query_ok )
  {
    debug("Cassandra client: query \"", query, "\" executed successfully");
  }
  else
  {
    warning("Cassandra client: query \"", query, "\" failed, due to \"", cass_error_desc(rc));
  }

  return query_future;
}
