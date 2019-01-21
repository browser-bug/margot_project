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
#include <algorithm>
#include <cstring>
#include <cassert>
#include <cmath> // for floor()

#include "agora/logger.hpp"
#include "agora/cassandra_fs_implementation.hpp"

using namespace agora;

CassandraClient::CassandraClient(const std::string& url, const std::string& username, const std::string& password)
  : is_connected(false), database_name("margot"), default_application_separator('/'), table_application_separator('_')
  , address(url), username(username), password(password)
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
  execute_query_synch("CREATE KEYSPACE IF NOT EXISTS " + database_name + " WITH REPLICATION = { 'class' : 'SimpleStrategy', 'replication_factor' : 1 };");

  // switch to the correct database
  execute_query_synch("USE " + database_name);

}


CassandraClient::~CassandraClient( void )
{
  // disconnect from the database
  CassFuture* connection_result = cass_session_close(session);
  cass_future_wait(connection_result); // wait for inflight traffic, might happens
  cass_future_free(connection_result);
  cass_session_free(session);
  info("Cassandra client: disconnected from the databse");
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

  // wait until the query is executed
  cass_future_wait(query_future);

  // evaluate if it went well
  CassError rc = cass_future_error_code(query_future);
  const bool is_query_ok = rc == CASS_OK;

  if ( is_query_ok )
  {
    debug("Cassandra client: query \"", query, "\" executed successfully");
  }
  else
  {
    warning("Cassandra client: query \"", query, "\" failed, due to \"", cass_error_desc(rc), "\"");
  }

  return query_future;
}




void CassandraClient::store_metrics( const std::string& application_name, const application_metrics_t& metrics )
{
  std::string table_name = application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  table_name += "_metrics";

  // create the table
  execute_query_synch("CREATE TABLE " + table_name + " ( name text PRIMARY KEY, type text, prediction text);");

  // populate the query
  for ( const auto& metric : metrics )
  {
    execute_query_synch("INSERT INTO " + table_name + " (name,type,prediction) VALUES ('" + metric.name +
                        "', '" + metric.type + "', '" + metric.prediction_method + "');");
  }
}


application_metrics_t CassandraClient::load_metrics( const std::string& application_name )
{
  // this will cotain the model
  application_metrics_t application_metrics;

  // compose the name of the table
  std::string table_name = application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  table_name += "_metrics";

  // how the result of the query will be processed
  const auto result_handler = [&application_metrics, &application_name] ( const CassResult * query_result )
  {

    // loop over the results
    if (query_result != nullptr)
    {
      CassIterator* row_iterator = cass_iterator_from_result(query_result);

      while (cass_iterator_next(row_iterator))
      {
        // get the reference from the row
        const CassRow* row = cass_iterator_get_row(row_iterator);

        // declare the object used to retrieve data from Cassandra
        const CassValue* field_value;
        const char* field_value_s;
        size_t lenght_output_string;
        CassError rc;

        // get the metric name
        field_value = cass_row_get_column_by_name(row, "name");
        rc = cass_value_get_string(field_value, &field_value_s, &lenght_output_string);

        if (rc != CASS_OK)
        {
          warning("Cassandra client: unable to convert a field to string");
        }

        const std::string metric_name(field_value_s, lenght_output_string);

        // get the metric predictor method
        field_value = cass_row_get_column_by_name(row, "prediction");
        rc = cass_value_get_string(field_value, &field_value_s, &lenght_output_string);

        if (rc != CASS_OK)
        {
          warning("Cassandra client: unable to convert a field to string");
        }

        const std::string predictor_method(field_value_s, lenght_output_string);

        // get the metric type
        field_value = cass_row_get_column_by_name(row, "type");
        rc = cass_value_get_string(field_value, &field_value_s, &lenght_output_string);

        if (rc != CASS_OK)
        {
          warning("Cassandra client: unable to convert a field to string");
        }

        const std::string metric_type(field_value_s, lenght_output_string);

        // emplace back the new information
        application_metrics.push_back({metric_name, metric_type, predictor_method});
      }

      // free the iterator through the rows
      cass_iterator_free(row_iterator);

      // free the result
      cass_result_free(query_result);
    }
  };

  // perform the query
  const std::string query = "SELECT * FROM " + table_name + ";";
  execute_query_synch(query, result_handler);


  return application_metrics;
}




void CassandraClient::store_knobs( const std::string& application_name, const application_knobs_t& knobs )
{
  // compose the name of the table
  std::string table_name = application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  table_name += "_knobs";

  // create the table
  execute_query_synch("CREATE TABLE " + table_name + " ( name text PRIMARY KEY, type text, values set<text> );");

  // populate the query
  for ( const auto& knob : knobs )
  {
    // write the sequence of values
    std::string values = "{ ";

    for ( const auto& value : knob.values )
    {
      values += "'" + value + "', ";
    }

    // remove the last coma
    values[values.size() - 2] = ' ';

    // execute the query
    execute_query_synch("INSERT INTO " + table_name + " (name,type,values) VALUES ('" + knob.name +
                        "', '" + knob.type + "', " + values + "} );");
  }
}


application_knobs_t CassandraClient::load_knobs( const std::string& application_name )
{
  // this will cotain the model
  application_knobs_t application_knobs;

  // compose the name of the table
  std::string table_name = application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  table_name += "_knobs";

  // how the result of the query will be processed
  const auto result_handler = [&application_knobs, &application_name] ( const CassResult * query_result )
  {

    // loop over the results
    if (query_result != nullptr)
    {
      CassIterator* row_iterator = cass_iterator_from_result(query_result);

      while (cass_iterator_next(row_iterator))
      {
        // get the reference from the row
        const CassRow* row = cass_iterator_get_row(row_iterator);

        // declare the object used to retrieve data from Cassandra
        const CassValue* field_value;
        const char* field_value_s;
        size_t lenght_output_string;
        CassError rc;

        // get the knob name
        field_value = cass_row_get_column_by_name(row, "name");
        rc = cass_value_get_string(field_value, &field_value_s, &lenght_output_string);

        if (rc != CASS_OK)
        {
          warning("Cassandra client: unable to convert a field to string");
        }

        const std::string knob_name(field_value_s, lenght_output_string);

        // get the metric type
        field_value = cass_row_get_column_by_name(row, "type");
        rc = cass_value_get_string(field_value, &field_value_s, &lenght_output_string);

        if (rc != CASS_OK)
        {
          warning("Cassandra client: unable to convert a field to string");
        }

        const std::string knob_type(field_value_s, lenght_output_string);


        // get the possible values
        std::vector<std::string> knob_values;
        field_value = cass_row_get_column_by_name(row, "values");
        CassIterator* collection_data_iterator = cass_iterator_from_collection(field_value);

        while (cass_iterator_next(collection_data_iterator))
        {
          // get the actual value from the iterator
          field_value = cass_iterator_get_value(collection_data_iterator);
          rc = cass_value_get_string(field_value, &field_value_s, &lenght_output_string);

          if (rc != CASS_OK)
          {
            warning("Cassandra client: unable to convert a field to string");
          }

          knob_values.emplace_back(field_value_s, lenght_output_string);
        }

        cass_iterator_free(collection_data_iterator);

        // emplace back the new information
        application_knobs.push_back({knob_name, knob_type, knob_values});
      }

      // free the iterator through the rows
      cass_iterator_free(row_iterator);

      // free the result
      cass_result_free(query_result);
    }
  };

  // perform the query
  const std::string query = "SELECT * FROM " + table_name + ";";
  execute_query_synch(query, result_handler);


  return application_knobs;
}


void CassandraClient::store_features( const std::string& application_name, const application_features_t& features )
{
  // compose the name of the table
  std::string table_name = application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  table_name += "_features";

  // create the table
  execute_query_synch("CREATE TABLE " + table_name + " ( name text PRIMARY KEY, type text, values set<text> );");

  // populate the query
  for ( const auto& feature : features )
  {
    // write the sequence of values
    std::string values = "{ ";

    for ( const auto& value : feature.values )
    {
      values += "'" + value + "', ";
    }

    // remove the last coma
    values[values.size() - 2] = ' ';

    // execute the query
    execute_query_synch("INSERT INTO " + table_name + " (name,type,values) VALUES ('" + feature.name +
                        "', '" + feature.type + "', " + values + "} );");
  }
}

application_features_t CassandraClient::load_features( const std::string& application_name )
{
  // this will cotain the model
  application_features_t application_features;

  // compose the name of the table
  std::string table_name = application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  table_name += "_features";

  // how the result of the query will be processed
  const auto result_handler = [&application_features, &application_name] ( const CassResult * query_result )
  {

    // loop over the results
    if (query_result != nullptr)
    {
      CassIterator* row_iterator = cass_iterator_from_result(query_result);

      while (cass_iterator_next(row_iterator))
      {
        // get the reference from the row
        const CassRow* row = cass_iterator_get_row(row_iterator);

        // declare the object used to retrieve data from Cassandra
        const CassValue* field_value;
        const char* field_value_s;
        size_t lenght_output_string;
        CassError rc;

        // get the knob name
        field_value = cass_row_get_column_by_name(row, "name");
        rc = cass_value_get_string(field_value, &field_value_s, &lenght_output_string);

        if (rc != CASS_OK)
        {
          warning("Cassandra client: unable to convert a field to string");
        }

        const std::string feature_name(field_value_s, lenght_output_string);

        // get the metric type
        field_value = cass_row_get_column_by_name(row, "type");
        rc = cass_value_get_string(field_value, &field_value_s, &lenght_output_string);

        if (rc != CASS_OK)
        {
          warning("Cassandra client: unable to convert a field to string");
        }

        const std::string feature_type(field_value_s, lenght_output_string);


        // get the possible values
        std::vector<std::string> feature_values;
        field_value = cass_row_get_column_by_name(row, "values");
        CassIterator* collection_data_iterator = cass_iterator_from_collection(field_value);

        while (cass_iterator_next(collection_data_iterator))
        {
          // get the actual value from the iterator
          field_value = cass_iterator_get_value(collection_data_iterator);
          rc = cass_value_get_string(field_value, &field_value_s, &lenght_output_string);

          if (rc != CASS_OK)
          {
            warning("Cassandra client: unable to convert a field to string");
          }

          feature_values.emplace_back(field_value_s, lenght_output_string);
        }

        cass_iterator_free(collection_data_iterator);

        // emplace back the new information
        application_features.push_back({feature_name, feature_type, feature_values});
      }

      // free the iterator through the rows
      cass_iterator_free(row_iterator);

      // free the result
      cass_result_free(query_result);
    }
  };

  // perform the query
  const std::string query = "SELECT * FROM " + table_name + ";";
  execute_query_synch(query, result_handler);


  return application_features;
}


void CassandraClient::store_doe( const application_description_t& description, const doe_t& doe )
{
  // compose the name of the table
  std::string table_name = description.application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  table_name += "_doe";

  // compose the table description and the primary keys
  std::string table_desc = "";
  std::string primary_key = "";
  const int number_of_knobs = description.knobs.size();

  for ( int i = 0; i < number_of_knobs; ++i )
  {
    table_desc.append(description.knobs[i].name +  " " + description.knobs[i].type + ",");
    primary_key.append(description.knobs[i].name + ",");
  }

  // append the counter field in the doe and remove the last coma
  table_desc.append("counter int");
  primary_key.pop_back();

  // append a copy of the counter field that will not be modified and can be used to reset the real counter
  table_desc.append(",original_counter int");

  // create the table
  execute_query_synch("CREATE TABLE " + table_name + " (" + table_desc + ", PRIMARY KEY (" + primary_key + ") );");


  // create a secondary index on the number of observations on configuration
  // to improve the efficiency of the queries
  execute_query_synch("CREATE INDEX ON " + table_name + " (counter);");

  // populate the query
  for ( const auto& configuration_pair : doe.required_explorations )
  {
    // execute the query
    execute_query_synch( "INSERT INTO " + table_name + " (" + primary_key + ",counter,original_counter) VALUES (" + configuration_pair.first + "," + std::to_string(
                           configuration_pair.second) + "," + std::to_string(configuration_pair.second) + " );" );
  }
}


doe_t CassandraClient::load_doe( const std::string& application_name )
{
  // this will cotain the model
  doe_t output_doe;

  // compose the name of the table
  std::string table_name = application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  table_name += "_doe";


  // how the result of the query will be processed
  const auto result_handler = [&output_doe, &application_name] ( const CassResult * query_result )
  {
    // check if we actually have a result, i.e. table not created yet
    if (query_result != nullptr)
    {
      // this array it is used to store all the types of the columns of the query
      std::vector<CassValueType> column_types;

      // get information on the fields
      const size_t temp_number_of_columns = cass_result_column_count(query_result);

      // subtract the "original_counter" column, we do not need to send that
      const size_t number_of_columns = temp_number_of_columns - 1;

      for ( size_t i = 0; i < number_of_columns; ++i )
      {
        column_types.emplace_back(cass_result_column_type(query_result, i));
      }

      // Get the actual content of table
      CassIterator* row_iterator = cass_iterator_from_result(query_result);

      while (cass_iterator_next(row_iterator))
      {
        // get the reference from the row
        const CassRow* row = cass_iterator_get_row(row_iterator);

        // initialize the string for the row
        configuration_t predictor;
        int doe_counter;

        // iterate over the column
        CassIterator* column_iterator = cass_iterator_from_row(row);
        auto type_iterator = column_types.cbegin();

        // cycle over the row values except for the last "original_counter" column, we do not need to send that
        for ( size_t i = 0; i < number_of_columns; ++i )
        {
          cass_iterator_next(column_iterator);

          // retrieve the field value
          const CassValue* field_value = cass_iterator_get_column(column_iterator);

          // check if the value is actually missing
          if (cass_value_is_null(field_value))
          {
            warning("Cassandra client: error, we have an empty doe row");
            throw std::runtime_error("Cassandra client error: empty doe row");
          }

          // store it as a string
          switch (*type_iterator)
          {
            case CASS_VALUE_TYPE_INT:
            {
              int32_t out_value32;
              CassError rc = cass_value_get_int32(field_value, &out_value32);

              if (rc == CASS_OK)
              {
                predictor.append(std::to_string(out_value32) + ",");
                doe_counter = out_value32; // for the counter should be enough
              }
              else
              {
                int64_t out_value64;
                CassError rc = cass_value_get_int64(field_value, &out_value64);

                if (rc == CASS_OK)
                {
                  predictor.append(std::to_string(out_value64) + ",");
                  doe_counter = out_value64; // for the counter should be enough
                }
                else
                {
                  warning("Cassandra client: i have a huge int, can't handle it :(");
                  predictor.append("N/A,");
                  doe_counter = -1; // we know that the counter is the last field
                }
              }
            }
            break;

            case CASS_VALUE_TYPE_FLOAT:
            {
              float out_value_f;
              CassError rc = cass_value_get_float(field_value, &out_value_f);

              if (rc == CASS_OK)
              {
                predictor.append(std::to_string(out_value_f) + ",");
              }
              else
              {
                warning("Cassandra client: i have a float which is not a float... yeah, exactly...");
                predictor.append("N/A,");
              }
            }
            break;

            case CASS_VALUE_TYPE_DOUBLE:
            {
              double out_value_d;
              CassError rc = cass_value_get_double(field_value, &out_value_d);

              if (rc == CASS_OK)
              {
                predictor.append(std::to_string(out_value_d) + ",");
              }
              else
              {
                warning("Cassandra client: i have a double which is not a double... yeah, exactly...");
                predictor.append("N/A,");
              }
            }
            break;

            default:
              warning("Cassandra client: i am reading an unknown value from the db");
              predictor.append("N/A,");
          }

          // increment the type counter
          ++type_iterator;
        }


        // pop the last coma from the string
        predictor.pop_back();

        // now we have to remove the last bit from the predictor
        const auto last_coma_pos = predictor.find_last_of(',');

        // append the row to the model
        output_doe.required_explorations.emplace(predictor.substr(0, last_coma_pos), doe_counter);

        // free the iterator to the row
        cass_iterator_free(column_iterator);
      }

      // free the iterator through the rows
      cass_iterator_free(row_iterator);

      // free the result
      cass_result_free(query_result);

      // set the iterator of the doe
      output_doe.next_configuration = output_doe.required_explorations.begin();
    }
  };

  // perform the query
  const std::string query = "SELECT * FROM " + table_name + " WHERE counter > 0 ALLOW FILTERING;";
  execute_query_synch(query, result_handler);


  return output_doe;
}


void CassandraClient::store_model( const application_description_t& description, const model_t& model )
{
  // compose the name of the table
  std::string table_name = description.application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  table_name += "_model";

  // compose the table description and the primary keys
  std::string table_desc = "";
  std::string primary_key = "";
  std::string non_primary_key = "";
  const int number_of_knobs = description.knobs.size();
  const int number_of_features = description.features.size();
  const int number_of_metrics = description.metrics.size();

  for ( int i = 0; i < number_of_knobs; ++i )
  {
    table_desc.append(description.knobs[i].name +  " " + description.knobs[i].type + ",");
    primary_key.append(description.knobs[i].name + ",");
  }

  for ( int i = 0; i < number_of_features; ++i )
  {
    table_desc.append(description.features[i].name +  " " + description.features[i].type + ",");
    primary_key.append(description.features[i].name + ",");
  }

  for ( int i = 0; i < number_of_metrics; ++i )
  {
    table_desc.append(description.metrics[i].name +  "_avg " + description.metrics[i].type + ",");
    table_desc.append(description.metrics[i].name +  "_std " + description.metrics[i].type + ",");
    non_primary_key.append(description.metrics[i].name + "_avg,");
    non_primary_key.append(description.metrics[i].name + "_std,");
  }

  // remove the last come from the field list
  primary_key.pop_back();
  non_primary_key.pop_back();

  // create the table
  execute_query_synch("CREATE TABLE " + table_name + " (" + table_desc + " PRIMARY KEY (" + primary_key + ") );");

  // set the number of fields to actually set
  std::string inserted_fields = model.column_size() == number_of_knobs + number_of_features + (2 * number_of_metrics) ?
                                primary_key + non_primary_key : primary_key;

  // populate the query
  for ( const auto& configuration : model.knowledge )
  {
    // execute the query
    execute_query_synch( "INSERT INTO " + table_name + " (" + inserted_fields + ") VALUES (" + configuration + " );" );
  }
}


model_t CassandraClient::load_model( const std::string& application_name )
{
  // this will cotain the model
  model_t output_model;

  // compose the name of the table
  std::string table_name = application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  table_name += "_model";


  // how the result of the query will be processed
  const auto result_handler = [&output_model, &application_name] ( const CassResult * query_result )
  {
    // check if we actually have a result, i.e. table not created yet
    if (query_result != nullptr)
    {
      // this array it is used to store all the types of the columns of the query
      std::vector<CassValueType> column_types;

      // get information on the fields
      const size_t number_of_columns = cass_result_column_count(query_result);

      for ( size_t i = 0; i < number_of_columns; ++i )
      {
        // get the type of the column
        const auto field_type = cass_result_column_type(query_result, i);
        column_types.emplace_back(field_type);
      }

      // STEP 2: Get the actual content of table
      CassIterator* row_iterator = cass_iterator_from_result(query_result);

      while (cass_iterator_next(row_iterator))
      {
        // get the reference from the row
        const CassRow* row = cass_iterator_get_row(row_iterator);

        // initialize the string for the row
        configuration_t predictor;

        // iterate over the column
        CassIterator* column_iterator = cass_iterator_from_row(row);
        auto type_iterator = column_types.cbegin();

        while (cass_iterator_next(column_iterator))
        {
          // retrieve the field value
          const CassValue* field_value = cass_iterator_get_column(column_iterator);

          // check if the value is actually missing
          if (cass_value_is_null(field_value))
          {
            break; // we reached the metrics section
          }

          // store it as a string
          switch (*type_iterator)
          {
            case CASS_VALUE_TYPE_INT:
            {
              int32_t out_value32;
              CassError rc = cass_value_get_int32(field_value, &out_value32);

              if (rc == CASS_OK)
              {
                predictor.append(std::to_string(out_value32) + ",");
              }
              else
              {
                int64_t out_value64;
                CassError rc = cass_value_get_int64(field_value, &out_value64);

                if (rc == CASS_OK)
                {
                  predictor.append(std::to_string(out_value64) + ",");
                }
                else
                {
                  warning("Cassandra client: i have a huge int, can't handle it :(");
                  predictor.append("N/A,");
                }
              }
            }
            break;

            case CASS_VALUE_TYPE_FLOAT:
            {
              float out_value_f;
              CassError rc = cass_value_get_float(field_value, &out_value_f);

              if (rc == CASS_OK)
              {
                predictor.append(std::to_string(out_value_f) + ",");
              }
              else
              {
                warning("Cassandra client: i have a float which is not a float... yeah, exactly...");
                predictor.append("N/A,");
              }
            }
            break;

            case CASS_VALUE_TYPE_DOUBLE:
            {
              double out_value_d;
              CassError rc = cass_value_get_double(field_value, &out_value_d);

              if (rc == CASS_OK)
              {
                predictor.append(std::to_string(out_value_d) + ",");
              }
              else
              {
                warning("Cassandra client: i have a double which is not a double... yeah, exactly...");
                predictor.append("N/A,");
              }
            }
            break;

            default:
              warning("Cassandra client: i am reading an unknown value from the db");
              predictor.append("N/A,");
          }

          // increment the type counter
          ++type_iterator;
        }

        // pop the last coma from the string
        predictor.pop_back();

        // append the row to the model
        output_model.knowledge.emplace_back(predictor);

        // free the iterator to the row
        cass_iterator_free(column_iterator);
      }

      // free the iterator through the rows
      cass_iterator_free(row_iterator);

      // free the result
      cass_result_free(query_result);
    }
  };

  // perform the query
  const std::string query = "SELECT * FROM " + table_name + ";";
  execute_query_synch(query, result_handler);

  return output_model;
}


void CassandraClient::create_trace_table( const application_description_t& description )
{
  // compose the name of the table
  std::string table_name = description.application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  table_name += "_trace";

  // compose the table description and the primary keys
  std::string table_desc = "day date,time time,client_id text,";
  std::string primary_key = "day,time,client_id,";
  const int number_of_knobs = static_cast<int>(description.knobs.size());
  const int number_of_features = static_cast<int>(description.features.size());
  const int number_of_metrics = static_cast<int>(description.metrics.size());

  for ( int i = 0; i < number_of_knobs; ++i )
  {
    table_desc.append(description.knobs[i].name +  " " + description.knobs[i].type + ",");
    primary_key.append(description.knobs[i].name + ",");
  }

  for ( int i = 0; i < number_of_features; ++i )
  {
    table_desc.append(description.features[i].name +  " " + description.features[i].type + ",");
    primary_key.append(description.features[i].name + ",");
  }

  for ( int i = 0; i < number_of_metrics; ++i )
  {
    table_desc.append(description.metrics[i].name +  " " + description.metrics[i].type + ",");
  }

  // additional attributes that will contain the predicted values for the metrics from the model
  for ( int i = 0; i < number_of_metrics; ++i )
  {
    table_desc.append("model_" + description.metrics[i].name +  " " + description.metrics[i].type + ",");
  }

  // remove the last come from the field list
  primary_key.pop_back();

  // create the table
  execute_query_synch("CREATE TABLE " + table_name + " (" + table_desc + " PRIMARY KEY (" + primary_key + ") );");
}

void CassandraClient::insert_trace_entry( const application_description_t& description, const std::string& values )
{
  // compose the name of the table
  std::string table_name = description.application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  table_name += "_trace";

  // compose the table description and the primary keys
  std::string fields = "day,time,client_id,";

  for ( const auto& knob : description.knobs )
  {
    fields.append(knob.name + ",");
  }

  for ( const auto& feature : description.features )
  {
    fields.append(feature.name + ",");
  }

  // first we have to get the number of seconds and nanoseconds from epoch
  const auto pos_first_coma = values.find_first_of(',', 0);
  const auto pos_second_coma = values.find_first_of(',', pos_first_coma + 1);

  // compute the length of the actual data of the "values" string
  int values_lenght = values.length() - pos_second_coma;

  // get the names of the metrics for the query if provided, otherwise use all the metrics in the db
  const auto pos_slash = values.find_first_of('/', pos_second_coma + 1); // find the slash separator if present

  // boolean variable to know whether or not the estimates are valid (not null) or "null"
  // if they are "null" is because the client does not have the model (and thus the estimates) yet
  bool null_estimates = false;

  if (values.find("null") != std::string::npos)
  {
    null_estimates = true;
  }

  // to remove the last 5 chars of the ",null" word
  if (null_estimates)
  {
    values_lenght -= 5;
  }

  if (pos_slash == std::string::npos)  // if no metric names provided
  {
    for ( const auto& metric : description.metrics ) // use all the metrics in the db
    {
      fields.append(metric.name + ",");
    }

    if (!null_estimates)
    {
      // repeat the cycle for the names of the attributes of the metrics estimates, prefixed by the "model_" prefix
      for ( const auto& metric : description.metrics ) // use all the metrics in the db
      {
        fields.append("model_" + metric.name + ",");
      }
    }

    // remove the last comma from the field list
    fields.pop_back();
  }
  else     // get the metrics names
  {
    // append just the names of the metrics currently enabled for which we have a value
    fields.append(values.substr((pos_slash + 1), std::string::npos));

    if (!null_estimates)
    {
      fields.append(",");

      // repeat the same as above for the metrics estimates for the model used
      // we have all the estimates theoretically "free of charge" in the model, but margot only passes the estimates for the enabled metrics
      std::string temp_string = "model_" + values.substr((pos_slash + 1), std::string::npos);

      // replace all the occurrences of "," inside the temporary string above with ",model_"
      // to adapt the names of the enabled metrics to the names of the attributes for the estimates in the table
      std::string metric_estimates;
      metric_estimates.reserve(temp_string.length());  // avoids a few memory allocations

      std::string::size_type lastPos = 0;
      std::string::size_type findPos;

      // while an occurrence of ',' is found
      while (std::string::npos != (findPos = temp_string.find(",", lastPos)))
      {
        // append whathever is before the ',' in the new string
        metric_estimates.append(temp_string, lastPos, findPos - lastPos);
        // replace the ',' with ",model_"
        metric_estimates += ",model_";
        // increase the current search index of 1, the size of the comma just analyzed
        lastPos = findPos + 1;
      }

      // Care for the rest after last occurrence
      metric_estimates += temp_string.substr(lastPos);

      fields.append(metric_estimates);
    }

    values_lenght = values_lenght - (values.length() - pos_slash); // update the length of the final actual data of the "values" string "cutting" away the metric names
  }

  time_t secs_since_epoch;
  int64_t nanosecs_since_secs;
  std::istringstream( values.substr(0, pos_first_coma) ) >> secs_since_epoch;
  std::istringstream( values.substr(pos_first_coma + 1, pos_second_coma - pos_first_coma) ) >> nanosecs_since_secs;

  // now we have to convert them in the funny cassandra format
  cass_uint32_t year_month_day = cass_date_from_epoch(secs_since_epoch);
  cass_int64_t time_of_day = cass_time_from_epoch(secs_since_epoch);


  // now we add to the time of a day the missing information
  time_of_day += nanosecs_since_secs;

  // now we have to compose again the values string
  std::string&& actual_data = std::to_string(year_month_day) + "," + std::to_string(time_of_day) + values.substr(pos_second_coma, values_lenght);

  // create the table
  execute_query_synch("INSERT INTO " + table_name + " (" + fields + ") VALUES (" + actual_data + ");");
}


void CassandraClient::update_doe( const application_description_t& description, const std::string& values )
{
  // compose the name of the table
  std::string table_name = description.application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  table_name += "_doe";

  // compose the table description and the primary keys
  std::string fields = "";

  for ( const auto& knob : description.knobs )
  {
    fields.append(knob.name + ",");
  }

  fields.append("counter");

  // create the table
  execute_query_synch("INSERT INTO " + table_name + " (" + fields + ") VALUES (" + values + ");");
}

/**
 * @details
 * Reset the doe to its initial status, drop the model and truncate the trace.
 * This method is used by agorà to perform the retraining.
 * According to the timestamp passad as argument the trace can be either truncated totally
 * or it can be partially delete. In the latter case it would be deleted for just all the
 * observations with timestamp older than the provided one.
 */
void CassandraClient::reset_doe( const application_description_t& description, const std::string& timestamp)
{

  // compose the name of the table
  std::string table_name = description.application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  table_name += "_doe";

  // this will contain the original number of observations for the doe
  int num_observations;

  // how the result of the query will be processed
  const auto result_handler = [&num_observations] ( const CassResult * query_result )
  {
    // check if we actually have a result, i.e. table not created yet
    if (query_result != nullptr)
    {

      // get the number of columns
      const size_t number_of_columns = cass_result_column_count(query_result);

      // if we have more than one column there is an error because we expect just one column
      if (number_of_columns != (size_t)1)
      {
        warning("Cassandra client: error, we too many \"original_counter\" rows!");
        throw std::runtime_error("Cassandra client error: wrong \"original_counter\" structure.");
      }

      // to store the type of the column of the query
      CassValueType column_type = cass_result_column_type(query_result, 0);

      // Get the first row
      const CassRow* row = cass_result_first_row(query_result);

      // iterate over the column (we know there is just one column, see above)
      CassIterator* column_iterator = cass_iterator_from_row(row);

      // point to the unique column (of the unique row)
      cass_iterator_next(column_iterator);

      // retrieve the field value
      const CassValue* field_value = cass_iterator_get_column(column_iterator);

      // check if the value is actually missing
      if (cass_value_is_null(field_value))
      {
        warning("Cassandra client: error, we have an \"original_counter\" row");
        throw std::runtime_error("Cassandra client error: empty \"original_counter\" row");
      }

      // check if the value is an integer, which is what we expect
      switch (column_type)
      {
        case CASS_VALUE_TYPE_INT:
        {
          int32_t out_value32;
          CassError rc = cass_value_get_int32(field_value, &out_value32);

          if (rc == CASS_OK)
          {
            num_observations = out_value32; // for the counter should be enough
          }
          else
          {
            int64_t out_value64;
            CassError rc = cass_value_get_int64(field_value, &out_value64);

            if (rc == CASS_OK)
            {
              num_observations = out_value64; // for the counter should be enough
            }
            else
            {
              warning("Cassandra client: i have a huge int, can't handle it :(");
              num_observations = -1; // we know that the counter is the last field
            }
          }
        }
        break;

        default:
          warning("Cassandra client: i am reading an unknown value from the db");
      }

      // free the iterator to the row
      cass_iterator_free(column_iterator);

      // free the result
      cass_result_free(query_result);
    }
  };

  /** Perform the query:
   * get the number of observations for the doe originally set by the user
   * since this number is the same for all the rows in the table,
   * we limit the query to the first result obtained
   */
  const std::string query = "SELECT original_counter FROM " + table_name + " limit 1;";
  execute_query_synch(query, result_handler);

  debug("Restoring DOE num of observations to: ", num_observations);

  // Get all the primary keys for the final query
  std::vector<std::string> primary_keys;

  // how the result of the query will be processed
  const auto result_handler_combinations = [&primary_keys] ( const CassResult * query_result )
  {
    // check if we actually have a result, i.e. table not created yet
    if (query_result != nullptr)
    {
      // this array it is used to store all the types of the columns of the query
      std::vector<CassValueType> column_types;

      // get information on the fields
      const size_t number_of_columns = cass_result_column_count(query_result);

      for ( size_t i = 0; i < number_of_columns; ++i )
      {
        column_types.emplace_back(cass_result_column_type(query_result, i));
      }

      // Get the actual content of table
      CassIterator* row_iterator = cass_iterator_from_result(query_result);

      while (cass_iterator_next(row_iterator))
      {
        // get the reference from the row
        const CassRow* row = cass_iterator_get_row(row_iterator);

        // initialize the string for the row
        std::string current_row_keys = "";

        // iterate over the column
        CassIterator* column_iterator = cass_iterator_from_row(row);
        auto type_iterator = column_types.cbegin();

        // cycle over the row values
        for ( size_t i = 0; i < number_of_columns; ++i )
        {
          cass_iterator_next(column_iterator);

          // retrieve the field value
          const CassValue* field_value = cass_iterator_get_column(column_iterator);

          // check if the value is actually missing
          if (cass_value_is_null(field_value))
          {
            warning("Cassandra client: error, we have an empty doe row");
            throw std::runtime_error("Cassandra client error: empty doe row");
          }

          // store it as a string
          switch (*type_iterator)
          {
            case CASS_VALUE_TYPE_INT:
            {
              int32_t out_value32;
              CassError rc = cass_value_get_int32(field_value, &out_value32);

              if (rc == CASS_OK)
              {
                current_row_keys.append(std::to_string(out_value32) + ",");
              }
              else
              {
                int64_t out_value64;
                CassError rc = cass_value_get_int64(field_value, &out_value64);

                if (rc == CASS_OK)
                {
                  current_row_keys.append(std::to_string(out_value64) + ",");
                }
                else
                {
                  warning("Cassandra client: i have a huge int, can't handle it :(");
                  current_row_keys.append("N/A,");
                }
              }
            }
            break;

            case CASS_VALUE_TYPE_FLOAT:
            {
              float out_value_f;
              CassError rc = cass_value_get_float(field_value, &out_value_f);

              if (rc == CASS_OK)
              {
                current_row_keys.append(std::to_string(out_value_f) + ",");
              }
              else
              {
                warning("Cassandra client: i have a float which is not a float... yeah, exactly...");
                current_row_keys.append("N/A,");
              }
            }
            break;

            case CASS_VALUE_TYPE_DOUBLE:
            {
              double out_value_d;
              CassError rc = cass_value_get_double(field_value, &out_value_d);

              if (rc == CASS_OK)
              {
                current_row_keys.append(std::to_string(out_value_d) + ",");
              }
              else
              {
                warning("Cassandra client: i have a double which is not a double... yeah, exactly...");
                current_row_keys.append("N/A,");
              }
            }
            break;

            default:
              warning("Cassandra client: i am reading an unknown value from the db");
              current_row_keys.append("N/A,");
          }

          // increment the type counter
          ++type_iterator;
        }

        // pop the last comma from the string
        current_row_keys.pop_back();

        // append the row to the vector
        primary_keys.emplace_back(current_row_keys);

        // free the iterator to the row
        cass_iterator_free(column_iterator);
      }

      // free the iterator through the rows
      cass_iterator_free(row_iterator);

      // free the result
      cass_result_free(query_result);
    }
  };

  // compose the table primary keys
  std::string fields = "";

  for ( const auto& knob : description.knobs )
  {
    fields.append(knob.name + ",");
  }

  // remove the last comma from the field list
  fields.pop_back();

  /** Perform the query:
   * get all the knobs combinations for the doe (primary keys)
   * and store them in a vector of strings.
   * Each string represents a combination (a primary key).
   */
  const std::string query_combinations = "SELECT " + fields + " FROM " + table_name + ";";
  execute_query_synch(query_combinations, result_handler_combinations);

  // Restore the original counter collected before
  for ( const auto i : primary_keys )
  {
    //debug("Primary Keys: ", i);
    // execute the query
    execute_query_synch( "INSERT INTO " + table_name + " (" + fields + ",counter) VALUES (" + i + "," + std::to_string(
                           num_observations) + " );" );
  }

  // erase the model before computing the new one
  table_name = description.application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );

  execute_query_synch("DROP TABLE " + table_name + "_model;");

  // delete the trace according to the content of the timestamp received
  // by design if the timestamp == "null" then delete the whole trace
  if (timestamp == "null")
  {
    // delete everything from the trace
    execute_query_synch("TRUNCATE TABLE " + table_name + "_trace;");
  }
  else
  {
    // delete just from the top of the trace to the element passed as timestamp

    std::vector<cass_uint32_t> dates;

    // how the result of the query will be processed
    const auto result_handler = [&dates] ( const CassResult * query_result )
    {

      // loop over the results
      if (query_result != nullptr)
      {

        // this array it is used to store all the types of the columns of the query
        std::vector<CassValueType> column_types;

        // get information on the fields
        const size_t number_of_columns = cass_result_column_count(query_result);

        for ( size_t i = 0; i < number_of_columns; ++i )
        {
          // get the type of the column
          const auto field_type = cass_result_column_type(query_result, i);
          column_types.emplace_back(field_type);
        }

        // STEP 2: Get the actual content of table
        CassIterator* row_iterator = cass_iterator_from_result(query_result);

        // cycle over all the rows of the result
        while (cass_iterator_next(row_iterator))
        {

          // string to store the current row date
          cass_uint32_t current_date;

          // get the reference from the row
          const CassRow* row = cass_iterator_get_row(row_iterator);

          // iterate over the columns
          CassIterator* column_iterator = cass_iterator_from_row(row);
          auto type_iterator = column_types.cbegin();

          while (cass_iterator_next(column_iterator))
          {
            // retrieve the field value
            const CassValue* field_value = cass_iterator_get_column(column_iterator);
            cass_uint32_t year_month_day;


            // store it as a string
            switch (*type_iterator)
            {

              case CASS_VALUE_TYPE_DATE:
              {

                CassError rc = cass_value_get_uint32(field_value, &year_month_day);

                if (rc == CASS_OK)
                {
                  current_date = year_month_day;
                  //debug("Entered date");
                }
                else
                {
                  warning("Error in date handling");
                }
              }
              break;

              default:
                // declare the object used to retrieve data from Cassandra
                const char* field_value_s;
                size_t lenght_output_string;
                CassError rc;
                rc = cass_value_get_string(field_value, &field_value_s, &lenght_output_string);

                if (rc != CASS_OK)
                {
                  warning("Cassandra client: unable to convert a field to string");
                }

                //debug("Entered string");
                const std::string current_string_field(field_value_s, lenght_output_string);
            }

            // increment the type counter
            ++type_iterator;
          }

          // emplace back the new information
          dates.emplace_back(current_date);

          // free the column iterator
          cass_iterator_free(column_iterator);
        }

        // free the iterator through the rows
        cass_iterator_free(row_iterator);

        // free the result
        cass_result_free(query_result);
      }
    };

    // we need to separate the timestamp which is "year_month_day,time_of_day"
    // look for the comma
    const auto pos_first_comma = timestamp.find_first_of(',', 0);
    std::string seconds = timestamp.substr(0, pos_first_comma);
    std::string nanoseconds = timestamp.substr(pos_first_comma + 1, std::string::npos);

    cassandra_time date_time = compute_cassandra_timestamps(seconds, nanoseconds);

    // perform the query
    // first get all the observations from the previous days
    const std::string query = "SELECT day FROM " + table_name + "_trace WHERE day < '" + std::to_string(date_time.year_month_day) + "' ALLOW FILTERING;";
    execute_query_synch(query, result_handler);

    for (auto& i : dates)
    {
      // the 1st query deleted all the rows with date < date_of_the_change
      //debug("Result: ", i);
      execute_query_synch("DELETE FROM " + table_name + "_trace WHERE day = '" + std::to_string(i) + "';");
    }

    // then deletes all the rows with date = date_of_the_change but with time <= time_of_the_change
    execute_query_synch("DELETE FROM " + table_name + "_trace WHERE day = '" + std::to_string(date_time.year_month_day) + "' AND time <= '" + std::to_string(date_time.time_of_day) + "';");
  }
}

// Return the list of clients (without duplicates) running a specific application
application_list_t CassandraClient::load_clients( const std::string& application_name )
{

  // this will contain the list of clients running a specific application
  application_list_t clients_list;

  // compose the name of the table
  std::string table_name = application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  table_name += "_trace";

  // how the result of the query will be processed
  const auto result_handler = [&clients_list, &application_name] ( const CassResult * query_result )
  {

    // loop over the results
    if (query_result != nullptr)
    {
      CassIterator* row_iterator = cass_iterator_from_result(query_result);

      while (cass_iterator_next(row_iterator))
      {
        // get the reference from the row
        const CassRow* row = cass_iterator_get_row(row_iterator);

        // declare the object used to retrieve data from Cassandra
        const CassValue* field_value;
        const char* field_value_s;
        size_t lenght_output_string;
        CassError rc;

        // get the client name
        field_value = cass_row_get_column_by_name(row, "client_id");
        rc = cass_value_get_string(field_value, &field_value_s, &lenght_output_string);

        if (rc != CASS_OK)
        {
          warning("Cassandra client: unable to convert a field to string");
        }

        const std::string client_name(field_value_s, lenght_output_string);

        // emplace back the new information
        clients_list.emplace(client_name);
      }

      // free the iterator through the rows
      cass_iterator_free(row_iterator);

      // free the result
      cass_result_free(query_result);
    }
  };

  // perform the query
  const std::string query = "SELECT client_id FROM " + table_name + ";";
  execute_query_synch(query, result_handler);

  return clients_list;
}


/**
 * @details
 * Return a vector of strings containing all the rows of the trace with timestamp newer than the one passed as argument.
 */
observations_list_t CassandraClient::load_client_observations( const std::string& application_name, const std::string& client_name, const std::string& query_select, const std::string& seconds,
    const std::string& nanoseconds )
{

  // this will contain the list of all the observations in the trace belonging to a specific pair of application-client
  observations_list_t observations_list;

  // compose the name of the table
  std::string table_name = application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  table_name += "_trace";

  // how the result of the query will be processed
  const auto result_handler = [&observations_list, &application_name] ( const CassResult * query_result )
  {

    // loop over the results
    if (query_result != nullptr)
    {

      // this array it is used to store all the types of the columns of the query
      std::vector<CassValueType> column_types;

      // get information on the fields
      const size_t number_of_columns = cass_result_column_count(query_result);

      for ( size_t i = 0; i < number_of_columns; ++i )
      {
        // get the type of the column
        const auto field_type = cass_result_column_type(query_result, i);
        column_types.emplace_back(field_type);
      }

      // STEP 2: Get the actual content of table
      CassIterator* row_iterator = cass_iterator_from_result(query_result);

      // cycle over all the rows of the result
      while (cass_iterator_next(row_iterator))
      {

        // string to store the current row, with the fields separated by comma
        observation_t current_observation;

        // get the reference from the row
        const CassRow* row = cass_iterator_get_row(row_iterator);

        // iterate over the columns
        CassIterator* column_iterator = cass_iterator_from_row(row);
        auto type_iterator = column_types.cbegin();

        bool got_time = false;
        bool got_date = false;
        int i = 0;

        while (cass_iterator_next(column_iterator))
        {
          //debug("Column: ", i);
          // retrieve the field value
          const CassValue* field_value = cass_iterator_get_column(column_iterator);
          cass_uint32_t year_month_day;
          cass_int64_t time_of_day;


          // store it as a string
          switch (*type_iterator)
          {
            case CASS_VALUE_TYPE_INT:
            {
              int32_t out_value32;
              CassError rc = cass_value_get_int32(field_value, &out_value32);

              if (rc == CASS_OK)
              {
                current_observation.append(std::to_string(out_value32) + " ");
                //debug("Entered int");
              }
              else
              {
                int64_t out_value64;
                CassError rc = cass_value_get_int64(field_value, &out_value64);

                if (rc == CASS_OK)
                {
                  current_observation.append(std::to_string(out_value64) + " ");
                  //debug("Entered int");
                }
                else
                {
                  warning("Cassandra client: i have a huge int, can't handle it :(");
                  current_observation.append("N/A ");
                }
              }
            }
            break;

            case CASS_VALUE_TYPE_FLOAT:
            {
              float out_value_f;
              CassError rc = cass_value_get_float(field_value, &out_value_f);

              if (rc == CASS_OK)
              {
                current_observation.append(std::to_string(out_value_f) + " ");
                //debug("Entered float");
              }
              else
              {
                // warning("Handling float NULL value due to observed value for the respective metric not provided by client");
                current_observation.append("N/A ");
              }
            }
            break;

            case CASS_VALUE_TYPE_DOUBLE:
            {
              double out_value_d;
              CassError rc = cass_value_get_double(field_value, &out_value_d);

              if (rc == CASS_OK)
              {
                current_observation.append(std::to_string(out_value_d) + " ");
                //debug("Entered double");
              }
              else
              {
                // warning("Handling double NULL value due to observed value for the respective metric not provided by client");
                current_observation.append("N/A ");
              }
            }
            break;

            case CASS_VALUE_TYPE_DATE:
            {

              CassError rc = cass_value_get_uint32(field_value, &year_month_day);

              if (rc == CASS_OK)
              {
                //debug("Entered date");
                got_date = true;
              }
              else
              {
                warning("Error in date handling");
                current_observation.append("N/A ");
              }
            }
            break;

            case CASS_VALUE_TYPE_TIME:
            {
              CassError rc = cass_value_get_int64(field_value, &time_of_day);

              if (rc == CASS_OK)
              {
                //debug("Entered time");
                got_time = true;
              }
              else
              {
                warning("Error in time handling");
                current_observation.append("N/A ");
              }
            }
            break;

            default:
              // declare the object used to retrieve data from Cassandra
              const char* field_value_s;
              size_t lenght_output_string;
              CassError rc;
              rc = cass_value_get_string(field_value, &field_value_s, &lenght_output_string);

              if (rc != CASS_OK)
              {
                warning("Cassandra client: unable to convert a field to string");
              }

              //debug("Entered string");
              const std::string current_string_field(field_value_s, lenght_output_string);
              current_observation.append(current_string_field + " ");
          }

          if ((got_time == true) && (got_date == true))
          {
            // get the seconds since epoch
            time_t seconds = (time_t)cass_date_time_to_epoch(year_month_day, time_of_day);
            // debug("Date and time: ", asctime(localtime(&seconds)));


            // in order to get the nanoseconds since seconds:
            // 1 nanosecond = 1e^(-9)
            // see: https://docs.datastax.com/en/developer/cpp-driver/2.6/topics/basics/date_and_time/
            // compute the modulus of nanoseconds since midnight (time_of_day) divided by one
            // nanosecond and we get the number of nanoseconds since seconds (since epoch)
            auto nanoseconds = time_of_day % 1000000000;

            //debug("Seconds epoch: ", seconds);
            //debug("NanoSeconds: ", nanoseconds);

            current_observation.append(std::to_string(seconds) + " " + std::to_string(nanoseconds) + " ");
            got_time = got_date = false;
          }

          // increment the type counter
          ++type_iterator;
          i++;
        }

        // pop the last coma from the string
        current_observation.pop_back();

        // emplace back the new information
        observations_list.emplace_back(current_observation);

        // free the column iterator
        cass_iterator_free(column_iterator);
      }

      // free the iterator through the rows
      cass_iterator_free(row_iterator);

      // free the result
      cass_result_free(query_result);
    }
  };

  cassandra_time date_time = compute_cassandra_timestamps(seconds, nanoseconds);
  // perform the query
  // first get all the observations from the same day but with a later time
  const std::string query = "SELECT " + query_select + " FROM " + table_name + " WHERE client_id = '" + client_name + "' AND day = '" + std::to_string(
                              date_time.year_month_day) + "' AND time > '" + std::to_string(date_time.time_of_day) + "' ALLOW FILTERING;";
  execute_query_synch(query, result_handler);
  // then get all the observations from the following days
  const std::string query2 = "SELECT " + query_select + " FROM " + table_name + " WHERE client_id = '" + client_name + "' AND day > '" + std::to_string(date_time.year_month_day) + "' ALLOW FILTERING;";
  execute_query_synch(query2, result_handler);

  return observations_list;
}


void CassandraClient::erase( const std::string& application_name )
{
  std::string table_name = application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );

  // obliterate the information about the application
  execute_query_synch("DROP TABLE " + table_name + "_knobs;");
  execute_query_synch("DROP TABLE " + table_name + "_features;");
  execute_query_synch("DROP TABLE " + table_name + "_metrics;");
  execute_query_synch("DROP TABLE " + table_name + "_doe;");
  execute_query_synch("DROP TABLE " + table_name + "_model;");
  execute_query_synch("DROP TABLE " + table_name + "_trace;");
}

// This function converts the timestamp received in input as a two strings representing respectively
// the standard ctime format of seconds since epoch and nanoseconds in the Cassandra date and time format.
// Returns a "cassandra_time" struct.
cassandra_time CassandraClient::compute_cassandra_timestamps(const std::string& seconds, const std::string& nanoseconds)
{
  cassandra_time result_timestamp;

  time_t front_secs_since_epoch;
  int64_t front_nanosecs_since_secs;
  // convert the strings in the input struct into the proper time formats
  std::istringstream( seconds ) >> front_secs_since_epoch;
  std::istringstream( nanoseconds ) >> front_nanosecs_since_secs;

  // now we have to convert them in the funny cassandra format
  result_timestamp.year_month_day = cass_date_from_epoch(front_secs_since_epoch);
  result_timestamp.time_of_day = cass_time_from_epoch(front_secs_since_epoch);

  // now we add to the time of a day the missing information
  result_timestamp.time_of_day += front_nanosecs_since_secs;

  return result_timestamp;
}
