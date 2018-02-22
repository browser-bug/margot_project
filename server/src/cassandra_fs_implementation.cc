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
#include <cassert>

#include "logger.hpp"
#include "cassandra_fs_implementation.hpp"


using namespace margot;

CassandraClient::CassandraClient(const std::string& url, const std::string& username, const std::string& password)
  : is_connected(false), database_name("agora"), default_application_separator('/'), table_application_separator('_')
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
  std::replace(table_name.begin(), table_name.end(), '.', table_application_separator );
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
  std::replace(table_name.begin(), table_name.end(), '.', table_application_separator );
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
  std::replace(table_name.begin(), table_name.end(), '.', table_application_separator );
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
  std::replace(table_name.begin(), table_name.end(), '.', table_application_separator );
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
  std::replace(table_name.begin(), table_name.end(), '.', table_application_separator );
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
  std::replace(table_name.begin(), table_name.end(), '.', table_application_separator );
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


void CassandraClient::store_doe( const std::string& application_name, const doe_t& doe )
{
  // compose the name of the table
  std::string table_name = application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  std::replace(table_name.begin(), table_name.end(), '.', table_application_separator );
  table_name += "_doe";

  // compose the query that creates the table
  std::string creation_query = "CREATE TABLE " + table_name + " ( ";
  std::string primary_key = " PRIMARY KEY (";
  std::string fields;
  const int number_of_fields = doe.fields_name.size();

  for ( int i = 0; i < number_of_fields; ++i )
  {
    // append the field to the query
    const std::string current_field = doe.fields_name[i];
    creation_query.append(current_field + " " + doe.fields_type[i] + ",");
    fields.append(current_field + ",");

    // check if it is a primary key
    const char field_identifier = current_field.at(0);

    if (field_identifier == 'k')
    {
      primary_key.append(current_field + ",");
    }
  }

  // remove the last coma from the strings
  primary_key.pop_back();
  fields.pop_back();

  // create the table
  execute_query_synch(creation_query + primary_key + ") );");

  // create a secondary index on the number of observations on configuration
  // to improve the efficiency of the queries
  execute_query_synch("CREATE INDEX ON " + table_name + " (counter);");

  // populate the query
  for ( const auto& configuration_pair : doe.doe )
  {
    // execute the query
    execute_query_synch( "INSERT INTO " + table_name + " (" + fields + ") VALUES (" + configuration_pair.first + "," + std::to_string(configuration_pair.second) + " );" );
  }
}


doe_t CassandraClient::load_doe( const std::string& application_name )
{
  // this will cotain the model
  doe_t output_doe;

  // compose the name of the table
  std::string table_name = application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  std::replace(table_name.begin(), table_name.end(), '.', table_application_separator );
  table_name += "_doe";


  // how the result of the query will be processed
  const auto result_handler = [&output_doe, &application_name] ( const CassResult * query_result )
  {
    // check if we actually have a result, i.e. table not created yet
    if (query_result != nullptr)
    {
      // this array it is used to store all the types of the columns of the query
      std::vector<CassValueType> column_types;

      // get information on the fields for names and types
      const size_t number_of_columns = cass_result_column_count(query_result);

      for ( size_t i = 0; i < number_of_columns; ++i )
      {
        // STEP 1: get the information about the specific column

        // get the name of the column
        const char* field_value_s;
        size_t lenght_output_string;
        CassError rc = cass_result_column_name(query_result, i, &field_value_s, &lenght_output_string);

        if (rc != CASS_OK)
        {
          warning("Cassandra client: unable to get a column name");
          output_doe.fields_name.emplace_back("Unkown");
        }
        else
        {
          output_doe.fields_name.emplace_back(std::string(field_value_s, lenght_output_string));
        }

        // get the type of the column
        const auto field_type = cass_result_column_type(query_result, i);
        column_types.emplace_back(field_type);

        switch (field_type)
        {
          case CASS_VALUE_TYPE_INT:
            output_doe.fields_type.emplace_back("int");
            break;

          case CASS_VALUE_TYPE_FLOAT:
            output_doe.fields_type.emplace_back("float");
            break;

          case CASS_VALUE_TYPE_DOUBLE:
            output_doe.fields_type.emplace_back("double");
            break;

          default:
            output_doe.fields_type.emplace_back("N/A");
        }
      }

      // NOTE: the following algotithm assumes that the last field is the counter
      if (output_doe.fields_name.back().compare("counter") != 0)
      {
        throw std::runtime_error("Cassandra client: the last field of the doe is not the counter!");
      }

      // STEP 2: Get the actual content of table
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

        while (cass_iterator_next(column_iterator))
        {
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
        output_doe.doe.emplace(predictor.substr(0, last_coma_pos), doe_counter);

        // free the iterator to the row
        cass_iterator_free(column_iterator);
      }

      // free the iterator through the rows
      cass_iterator_free(row_iterator);

      // free the result
      cass_result_free(query_result);

      // set the iterator of the doe
      output_doe.next_configuration = output_doe.doe.begin();
    }
  };

  // perform the query
  const std::string query = "SELECT * FROM " + table_name + " WHERE counter > 0 ALLOW FILTERING;";
  execute_query_synch(query, result_handler);


  return output_doe;
}


void CassandraClient::store_model( const std::string& application_name, const model_t& model )
{
  // compose the name of the table
  std::string table_name = application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  std::replace(table_name.begin(), table_name.end(), '.', table_application_separator );
  table_name += "_model";

  // compose the query that creates the table
  std::string creation_query = "CREATE TABLE " + table_name + " ( ";
  std::string primary_key = " PRIMARY KEY (";
  std::string fields;
  const int number_of_fields = model.fields_name.size();
  const int num_data_fields = model.num_data_fields();

  for ( int i = 0; i < number_of_fields; ++i )
  {
    // append the field to the query
    const std::string current_field = model.fields_name[i];
    creation_query.append(current_field + " " + model.fields_type[i] + ",");

    // store the number of fields in the current model
    if (i < num_data_fields)
    {
      fields.append(current_field + ",");
    }

    // check if it is a primary key
    const char field_identifier = current_field.at(0);

    if ((field_identifier == 'k') || (field_identifier == 'f'))
    {
      primary_key.append(current_field + ",");
    }
  }

  // remove the last coma from the strings
  primary_key.pop_back();
  fields.pop_back();

  // create the table
  execute_query_synch(creation_query + primary_key + ") );");

  // populate the query
  for ( const auto& configuration : model.model_data )
  {
    // execute the query
    execute_query_synch( "INSERT INTO " + table_name + " (" + fields + ") VALUES (" + configuration + " );" );
  }
}


model_t CassandraClient::load_model( const std::string& application_name )
{
  // this will cotain the model
  model_t output_model;

  // compose the name of the table
  std::string table_name = application_name;
  std::replace(table_name.begin(), table_name.end(), default_application_separator, table_application_separator );
  std::replace(table_name.begin(), table_name.end(), '.', table_application_separator );
  table_name += "_model";


  // how the result of the query will be processed
  const auto result_handler = [&output_model, &application_name] ( const CassResult * query_result )
  {
    // check if we actually have a result, i.e. table not created yet
    if (query_result != nullptr)
    {
      // this array it is used to store all the types of the columns of the query
      std::vector<CassValueType> column_types;

      // get information on the fields for names and types
      const size_t number_of_columns = cass_result_column_count(query_result);

      for ( size_t i = 0; i < number_of_columns; ++i )
      {
        // STEP 1: get the information about the specific column

        // get the name of the column
        const char* field_value_s;
        size_t lenght_output_string;
        CassError rc = cass_result_column_name(query_result, i, &field_value_s, &lenght_output_string);

        if (rc != CASS_OK)
        {
          warning("Cassandra client: unable to get a column name");
          output_model.fields_name.emplace_back("Unkown");
        }
        else
        {
          output_model.fields_name.emplace_back(std::string(field_value_s, lenght_output_string));
        }

        // get the type of the column
        const auto field_type = cass_result_column_type(query_result, i);
        column_types.emplace_back(field_type);

        switch (field_type)
        {
          case CASS_VALUE_TYPE_INT:
            output_model.fields_type.emplace_back("int");
            break;

          case CASS_VALUE_TYPE_FLOAT:
            output_model.fields_type.emplace_back("float");
            break;

          case CASS_VALUE_TYPE_DOUBLE:
            output_model.fields_type.emplace_back("double");
            break;

          default:
            output_model.fields_type.emplace_back("N/A");
        }
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
        output_model.model_data.emplace_back(predictor);

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
