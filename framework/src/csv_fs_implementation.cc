#include <string>
#include <fstream>
#include <stdexcept>
#include <cstdint>
#include <map>
#include <vector>
#include <algorithm>
#include <cstdio>

#include "agora/csv_fs_implementation.hpp"
#include "agora/logger.hpp"

namespace agora
{


  // this is a helper structure to parse a csv file
  struct csv_parser_t
  {

    // the constructor we open the file and parse the table header
    csv_parser_t( const std::string& csv_file_path, const bool tokenize_row = false )
      : tokenize_row(tokenize_row)
    {
      in.open(csv_file_path, std::ios::in);

      if (!in.is_open())
      {
        // the file is not available for some reason, notify the event and quit
        warning("Csv manager: unable to open as input the file \"", csv_file_path, "\"");
      }
      else
      {
        // get the header of the csv from the file (first line)
        std::string header_line;
        std::getline(in, header_line);

        // make sure that we have a header
        if (header_line.empty())
        {
          warning("Csv manager: unable to read the header for file \"", csv_file_path, "\"");
          throw std::runtime_error("Csv manager: unable to read the header for file \"" + csv_file_path + "\"");
        }

        // tokenize the string only if needed
        if (tokenize_row)
        {
          // declare helper data structure
          std::istringstream header_stream(header_line);
          std::string field_name;
          std::size_t field_index = 0;

          // loop over the field
          while (std::getline(header_stream, field_name, ','))
          {
            table_header.emplace(field_name, field_index++);
          }

          // initialize the data structure to hold the csv line
          current_row.resize(table_header.size());
        }
      }
    }

    // in the destructor we close the file
    ~csv_parser_t( void )
    {
      in.close();
      table_header.clear();
      current_row.clear();
    }

    // this function read a new line from the csv file and it tokenize it
    // it returns true if there is available another line to read
    bool next( void )
    {
      std::getline(in, current_row_raw);
      const bool new_line_available = !current_row_raw.empty();

      if (new_line_available && tokenize_row)
      {
        // we are able to parse the row using string stream
        std::istringstream line_stream(current_row_raw);
        std::string token;
        std::size_t token_index = 0;

        // store all the fields in the row
        while ( std::getline(line_stream, token, ',') )
        {
          current_row[token_index++] = token;
        }
      }

      return new_line_available;
    }

    // it retrieves a field of the current row
    inline std::string get( const std::string& field_name ) const
    {
      return current_row[table_header.at(field_name)];
    }

    // it retrieves the whole line as a string
    inline std::string get( void ) const
    {
      return current_row_raw;
    }

    // attribute of the utility structure
    std::ifstream in;
    const bool tokenize_row;
    std::map< std::string, std::size_t > table_header;
    std::vector< std::string > current_row;
    std::string current_row_raw;
  };








  CsvStorage::CsvStorage( const std::string& storage_root_path )
    : storage_main_folder(storage_root_path), default_application_separator('/'), table_application_separator('_')
  {}

  void CsvStorage::store_description( const application_description_t& description )
  {
    // storing the software knobs
    std::ofstream out;
    out.open(get_knobs_name(description.application_name), std::ios::out | std::ios::trunc);

    if (out.is_open())
    {
      // print the table header
      out << "name,type,values" << std::endl;

      // loop over the software knobs and print them
      for ( const auto& knob : description.knobs )
      {

        // write the knob name, type and the first value
        out << knob.name << ',' << knob.type << ',' << knob.values[0];

        // get the how many other knob values we have
        const std::size_t number_of_knob_values = knob.values.size();

        // print the values
        for ( std::size_t i = 1; i < number_of_knob_values; ++i )
        {
            out << ';' << knob.values[i];
        }

        // write the last new line character
        out << std::endl;
      }
    }
    else
    {
      warning("Csv manager: unable to open/create file \"", get_knobs_name(description.application_name), "\"");
      throw std::runtime_error("Csv manager: unable to open/create file \"" + get_knobs_name(description.application_name) + "\"");
    }

    out.close();

    // storing the metrics
    out.open(get_metrics_name(description.application_name), std::ios::out | std::ios::trunc);

    if (out.is_open())
    {
      // print the table header
      out << "name,type,prediction" << std::endl;

      // loop over the software knobs and print them
      for ( const auto& metric : description.metrics )
      {
        out << metric.name << ',' << metric.type << ',' << metric.prediction_method << std::endl;
      }
    }
    else
    {
      warning("Csv manager: unable to open/create file \"", get_metrics_name(description.application_name), "\"");
      throw std::runtime_error("Csv manager: unable to open/create file \"" + get_metrics_name(description.application_name) + "\"");
    }

    out.close();

    // storing the input features
    out.open(get_features_name(description.application_name), std::ios::out | std::ios::trunc);

    if (out.is_open())
    {
      // print the table header
      out << "name,type,values" << std::endl;

      // loop over the software knobs and print them
      for ( const auto& feature : description.features )
      {

        // write the feature name, type and the first value
        out << feature.name << ',' << feature.type << ',' << feature.values[0];

        // get the how many other feature values we have
        const std::size_t number_of_feature_values = feature.values.size();

        // print the values
        for ( std::size_t i = 1; i < number_of_feature_values; ++i )
        {
            out << ';' << feature.values[i];
        }

        // write the last new line character
        out << std::endl;
      }
    }
    else
    {
      warning("Csv manager: unable to open/create file \"", get_features_name(description.application_name), "\"");
      throw std::runtime_error("Csv manager: unable to open/create file \"" + get_features_name(description.application_name) + "\"");
    }

    out.close();

    // storing the doe information
    out.open(get_doe_info_name(description.application_name), std::ios::out | std::ios::trunc);

    if (out.is_open())
    {
      // print the table header
      out << "property_name,value" << std::endl;

      // print the actual properties
      out << "number_point_per_dimension," << description.number_point_per_dimension << std::endl;
      out << "number_observations_per_point," << description.number_observations_per_point << std::endl;
      out << "doe_name," << description.doe_name << std::endl;
      out << "minimum_distance," << description.minimum_distance << std::endl;
    }
    else
    {
      warning("Csv manager: unable to open/create file \"", get_doe_info_name(description.application_name), "\"");
      throw std::runtime_error("Csv manager: unable to open/create file \"" + get_doe_info_name(description.application_name) + "\"");
    }

    out.close();
  }


  application_description_t CsvStorage::load_description( const std::string& application_name )
  {
    // initialize the object
    application_description_t description(application_name);

    // open the table of the knobs
    csv_parser_t knob_parser(get_knobs_name(application_name), true);

    // loop over the lines of the knobs
    while (knob_parser.next())
    {
      // get the knob name and type, prepare the stream for values
      const std::string knob_name = knob_parser.get("name");
      const std::string knob_type = knob_parser.get("type");
      std::stringstream knob_values_stream(knob_parser.get("values"));
      field_design_space_t knob_values;
      std::string knob_value;

      // parse all the possible values for the stream
      while ( std::getline(knob_values_stream, knob_value, ';') )
      {
        knob_values.emplace_back(knob_value);
      }

      // emplace the knob
      description.knobs.push_back({knob_name, knob_type, knob_values});
    }

    // open the table of the features
    csv_parser_t feature_parser(get_features_name(application_name), true);

    // loop over the lines of the features
    while (feature_parser.next())
    {
      // get the feature name and type, prepare the stream for values
      const std::string feature_name = feature_parser.get("name");
      const std::string feature_type = feature_parser.get("type");
      std::stringstream feature_values_stream(feature_parser.get("values"));
      field_design_space_t feature_values;
      std::string feature_value;

      // parse all the possible values for the stream
      while ( std::getline(feature_values_stream, feature_value, ';') )
      {
        feature_values.emplace_back(feature_value);
      }

      // emplace the feature
      description.features.push_back({feature_name, feature_type, feature_values});
    }

    // open the table of the metrics
    csv_parser_t metric_parser(get_metrics_name(application_name), true);

    // loop over the lines of the knobs
    while (metric_parser.next())
    {
      // get the metric name, type and prediction method
      const std::string metric_name = metric_parser.get("name");
      const std::string metric_type = metric_parser.get("type");
      const std::string metric_predictor = metric_parser.get("prediction");

      // emplace the metric
      description.metrics.push_back({metric_name, metric_type, metric_predictor});
    }

    // open the table of the doe info
    csv_parser_t doe_info_parser(get_doe_info_name(application_name), true);

    // loop over the lines of the knobs
    while (doe_info_parser.next())
    {
      // read the property
      const std::string property_name = doe_info_parser.get("property_name");
      const std::string property_value = doe_info_parser.get("value");

      // assign the correct value
      if (property_name.compare("number_point_per_dimension") == 0)
      {
        description.number_point_per_dimension = property_value;
      }
      else
      {
        if (property_name.compare("number_observations_per_point") == 0)
        {
          description.number_observations_per_point = property_value;
        }
        else
        {
          if (property_name.compare("doe_name") == 0)
          {
            description.doe_name = property_value;
          }
          else
          {
            if (property_name.compare("minimum_distance") == 0)
            {
              description.minimum_distance = property_value;
            }
            else
            {
              warning("Csv manager: unknown doe property \"" + property_name + "\" with value \"" + property_value + "\"");
            }
          }
        }
      }
    }

    // check if everything is correct and return the description
    return !(description.number_point_per_dimension.empty() || description.number_observations_per_point.empty() || description.doe_name.empty() ||
             description.minimum_distance.empty()) ? description : application_description_t{};
  }


  void CsvStorage::store_model( const application_description_t& description, const model_t& model )
  {
    // open the file for the model
    std::ofstream out;
    out.open(get_model_name(description.application_name), std::ios::out | std::ios::trunc);

    // write the header
    for ( const auto& knob : description.knobs )
    {
      out << knob.name << ',';
    }

    for ( const auto& feature : description.features )
    {
      out << feature.name << ',';
    }

    const std::size_t number_of_metrics = description.metrics.size() - 1;

    for ( std::size_t i = 0; i < number_of_metrics; ++i )
    {
      out << description.metrics[i].name << "_avg," << description.metrics[i].name << "_std,";
    }

    out << description.metrics[number_of_metrics].name << "_avg," << description.metrics[number_of_metrics].name << "_std" << std::endl;

    // get the number of expected fields
    const std::size_t number_of_metric_fields = static_cast<std::size_t>(2) * description.metrics.size();
    const int number_of_fields = number_of_metric_fields + description.knobs.size() + description.features.size();

    // check if we have all the information
    const bool metrics_available = number_of_fields == model.column_size();

    // loop over the entries in the model
    for ( const auto& configuration : model.knowledge )
    {
      // write the configuration line
      out << configuration;

      // generate the NA values for metrics if not available
      if (!metrics_available)
      {
        // write the coma for the metric section
        out << ',';

        // compute the number of fields for printing metrics
        const int stop_condition_loop = number_of_metric_fields - 1;

        // generate the metric field NAs
        for ( int i = 0; i < stop_condition_loop; ++i )
        {
          out << "NA,";
        }

        // write the last NA
        out << "NA";
      }

      // write the new line
      out << std::endl;
    }
  }


  model_t CsvStorage::load_model( const application_description_t& description )
  {
    // declare the output model
    model_t output_model;

    // open the table of the model
    csv_parser_t model_parser(get_model_name(description.application_name));

    // loop over the lines of the model
    while (model_parser.next())
    {
      // get the whole line
      std::string csv_line = model_parser.get();

      // remove any possible NAs
      output_model.knowledge.emplace_back(csv_line.substr(0, csv_line.find(",NA")));
    }

    // return the model
    return output_model;
  }


  void CsvStorage::store_doe( const application_description_t& description, const doe_t& doe )
  {
    // open the file for the doe
    std::ofstream out;
    out.open(get_doe_name(description.application_name), std::ios::out | std::ios::trunc);

    // write the header
    for ( const auto& knob : description.knobs )
    {
      out << knob.name << ',';
    }

    out << "counter" << std::endl;

    // write the all the required explorations
    for ( const auto& configuration : doe.required_explorations )
    {
      out << configuration.first << ',' << configuration.second << std::endl;
    }
  }


  doe_t CsvStorage::load_doe( const std::string& application_name )
  {
    // declare the output doe
    doe_t output_doe;

    // open the table of the doe
    csv_parser_t doe_parser(get_doe_name(application_name));

    // go through each line of the model
    while (doe_parser.next())
    {
      // get the whole line of the doe
      std::string csv_line = doe_parser.get();

      // get the split point between configuration and counter
      std::size_t coma_index = csv_line.find_last_of(',');

      // insert the configuration
      output_doe.required_explorations.emplace(csv_line.substr(0, coma_index), std::stoi(csv_line.substr(coma_index + 1)));
    }

    // set the next to begin
    output_doe.next_configuration = output_doe.required_explorations.begin();

    return output_doe;
  }


  void CsvStorage::update_doe( const application_description_t& description, const std::string& values )
  {
    // declare the new doe table
    doe_t output_doe;

    // get the parser for previous table
    csv_parser_t doe_parser(get_doe_name(description.application_name));

    // go through each line of the model
    while (doe_parser.next())
    {
      // get the whole line of the doe
      std::string csv_line = doe_parser.get();

      // get the split point between configuration and counter
      std::size_t coma_index = csv_line.find_last_of(',');

      // tokenize the configuration
      std::string conf = csv_line.substr(0, coma_index);
      int counter = std::stoi(csv_line.substr(coma_index + 1));

      // check if it is the configuration that we have explored
      // in that case we have to update the counter
      if (values.compare(conf) == 0)
      {
        --counter;
      }

      // insert the configuration in the doe
      output_doe.required_explorations.emplace(conf, counter);
    }

    // rewrite the doe
    store_doe(description, output_doe);
  }


  void CsvStorage::empty_doe_entries( const application_description_t& description )
  {
    // open the file for the doe
    std::ofstream out;
    out.open(get_doe_name(description.application_name), std::ios::out | std::ios::trunc);

    // write the header
    for ( const auto& knob : description.knobs )
    {
      out << knob.name << ',';
    }

    out << "counter" << std::endl;
  }


  void CsvStorage::create_trace_table( const application_description_t& description )
  {
    // open the file for the trace table
    std::ofstream out;
    out.open(get_observation_name(description.application_name), std::ios::out | std::ios::trunc);

    // write the common part of the header
    out << "sec,nanosec,client_id,";

    // write the software knobs
    for ( const auto& knob : description.knobs )
    {
      out << knob.name << ',';
    }

    // write the input features
    for ( const auto& feature : description.features )
    {
      out << feature.name << ',';
    }

    // get the number of metrics
    const std::size_t number_of_metrics = description.metrics.size() - static_cast<std::size_t>(1);

    // write the metrics
    for ( std::size_t i = 0; i < number_of_metrics; ++i )
    {
      out << description.metrics[i].name << ',';
    }

    out << description.metrics[number_of_metrics].name << std::endl;
  }


  void CsvStorage::insert_trace_entry( const application_description_t& description, const std::string& values )
  {
    // open the file for the trace table
    std::ofstream out;
    out.open(get_observation_name(description.application_name), std::ios::out | std::ios::app);
    out << values << std::endl;
  }


  void CsvStorage::erase( const std::string& application_name )
  {
    // declare a lambda to safely remove the file
    const auto safe_rm = [] ( const std::string & file_path )
    {
      if ( remove( file_path.c_str() ) != 0 )
      {
        warning("Csv manager: unable to remove the file \"", file_path, "\"");
      }
    };

    // deletes the tables
    safe_rm(get_observation_name(application_name));
    safe_rm(get_model_name(application_name));
    safe_rm(get_knobs_name(application_name));
    safe_rm(get_features_name(application_name));
    safe_rm(get_metrics_name(application_name));
    safe_rm(get_doe_name(application_name));
    safe_rm(get_doe_info_name(application_name));
  }

}
