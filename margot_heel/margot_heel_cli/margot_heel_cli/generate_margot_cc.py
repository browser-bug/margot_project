import os                                # for creating the path
import errno                             # for checking before creating a path


from .generate_utility import generate_start_monitor_signature
from .generate_utility import generate_stop_monitor_signature
from .generate_utility import generate_update_signature



from .model_data_feature import DataFeatureModel



what_translator = {
  "AVERAGE"  : "average",
  "STDDEV"   : "standard_deviation",
  "MAX"      : "max",
  "MIN"      : "min",
}

cfun_feature_translator = {
  "GE"  : "margot::FeatureComparison::GREATER_OR_EQUAL",
  "LE"  : "margot::FeatureComparison::LESS_OR_EQUAL",
  "-"   : "margot::FeatureComparison::DONT_CARE"
}



def generate_block_body( block_model, op_lists, cc ):
  """
  Generates the per block code
  """

  # write the begin of the namespace
  cc.write('\n\n\tnamespace {0} {{\n'.format(block_model.block_name))

  # write all the monitors
  if block_model.monitor_models:

    # open the monitor namespace
    cc.write('\n\t\tnamespace monitor {\n')

    # loop over the monitor
    for monitor_model in block_model.monitor_models:
      cc.write('\t\t\t{0} {1};\n'.format(monitor_model.monitor_class, monitor_model.monitor_name))

    # close the monitor namespace
    cc.write('\t\t} // namespace monitor\n')

  # write all the goals
  if block_model.goal_models:

    # open the goal namespace
    cc.write('\n\t\tnamespace goal {\n')

    # loop over the goals
    for goal_model in block_model.goal_models:

      #try to figure out the type of the goal
      goal_type = 'long double'
      if goal_model.metric_name_ref:
        for metric_model in block_model.metrics:
          if metric_model.name == goal_model.metric_name_ref:
            goal_type = metric_model.type
      if goal_model.parameter_name_ref:
        for knob_model in block_model.software_knobs:
          if knob_model.name == goal_model.parameter_name_ref:
            goal_type = knob_model.var_type

      # write its declaration
      cc.write('\t\t\t{1} {0};\n'.format(goal_model.name, goal_model.get_c_goal_type(goal_type)))

    # close the goal namespace
    cc.write('\t\t} // namespace goal\n')

  # write the observed data features variables (if any)
  if (block_model.features):

    # open the data feature namespace
    cc.write('\n\t\tnamespace features {\n')

    # loop over the features
    for feature in block_model.features:
      cc.write('\t\t\t{1} {0};\n'.format(feature.name, feature.type))

    # close the feature namespace
    cc.write('\t\t} // namespace features\n')

  # open the software knobs namespace
  cc.write('\n\t\tnamespace knobs {\n')

  # loop over the knobs
  for knob_model in block_model.software_knobs:
    cc.write('\t\t\t{1} {0};\n'.format(knob_model.var_name, knob_model.var_type))

  # close the software knobs namespace
  cc.write('\t\t} // namespace knobs\n')

  # write the manager (check if we have data feature)
  if (block_model.metrics and block_model.software_knobs ):
      if (block_model.features):

        # get the names of data feature, in alphabetical order
        names = sorted([ x.name for x in block_model.features ])

        # set the type of the asrtm
        features_types = set([x.type for x in block_model.features])
        features_types_indexes = [ DataFeatureModel.available_var_types.index(x) for x in features_types]
        features_type = DataFeatureModel.available_var_types[max(features_types_indexes)]

        # loop over the data feature fields to compose the type
        features_cf = []
        for name in names:
          # get the corresponding data feature comparison function
          feature_cf = [ x.cf for x in block_model.features if x.name == name][0]
          features_cf.append(cfun_feature_translator[feature_cf])

        # print the new type of the asrtm
        cc.write('\n\n\t\tDataAwareAsrtm< Asrtm< MyOperatingPoint >, {0}, margot::FeatureDistanceType::{1}, {2} > manager;\n\n\n'.format(features_type, block_model.feature_distance, ', '.join(features_cf)))
      else:
        # write the manager
        cc.write('\n\n\t\tAsrtm< MyOperatingPoint > manager;\n\n\n')

  # write the logger
  cc.write('\n\t\t#ifdef MARGOT_LOG_FILE\n')
  cc.write('\t\tLogger file_logger;\n')
  cc.write('\t\t#endif // MARGOT_LOG_FILE\n\n\n')

  # write all the exposed variables from the monitors
  for monitor_model in block_model.monitor_models:
    for exposed_var_what in monitor_model.exposed_metrics:
      cc.write('\n\t\t{1}::statistical_type {0};\n'.format(monitor_model.exposed_metrics[exposed_var_what], monitor_model.monitor_class))

  # write the update function
  cc.write('\n\n\t\tbool {0}\n'.format(generate_update_signature(block_model)))
  cc.write('\t\t{\n')

  # check if we should handle the data features
  if (block_model.metrics and block_model.software_knobs ):
      if block_model.features:

        # write the statements that assign the data features to the global variable
        feature_names = []
        for feature in block_model.features:
          cc.write('\t\t\tfeatures::{0} = {0};\n'.format(feature.name))
          feature_names.append(feature.name)

        # write the statement that select the correct asrtm
        feature_string = '{{{{{0}}}}}'.format(', '.join(feature_names))
        cc.write('\t\t\tmanager.select_feature_cluster({0});\n'.format(feature_string))


      cc.write('\t\t\tif (!manager.is_application_knowledge_empty())\n')
      cc.write('\t\t\t{\n')
      cc.write('\t\t\t\tmanager.find_best_configuration();\n')
      cc.write('\t\t\t\tbool conf_changed = false;\n')
      cc.write('\t\t\t\tconst auto new_conf = manager.get_best_configuration(&conf_changed);\n')
      cc.write('\t\t\t\tif (conf_changed)\n')
      cc.write('\t\t\t\t{\n')
      param_list = ['{0}& {1}'.format(x.var_type, x.var_name) for x in block_model.software_knobs]
      for knob in block_model.software_knobs:
        cc.write('\t\t\t\t\t{0} = new_conf.get_mean<static_cast<std::size_t>(Knob::{1})>();\n'.format(knob.var_name, knob.name.upper()))
      cc.write('\t\t\t\t}\n')
      for knob in block_model.software_knobs:
        cc.write('\t\t\t\tknobs::{0} = {0};\n'.format(knob.var_name))
      cc.write('\t\t\t\treturn conf_changed;\n')
      cc.write('\t\t\t}\n')
      for knob in block_model.software_knobs:
        cc.write('\t\t\tknobs::{0} = {0};\n'.format(knob.var_name))
      cc.write('\t\t\treturn false;\n')
  else:
      cc.write('\t\t\treturn false;\n')
  cc.write('\t\t}\n')

  # write the start_monitor function
  cc.write('\n\n\t\tvoid {0}\n'.format(generate_start_monitor_signature(block_model)))
  cc.write('\t\t{\n')
  for monitor_model in block_model.monitor_models:
    if monitor_model.start_method:
      cc.write('\t\t\tmonitor::{0}.{1}('.format(monitor_model.monitor_name, monitor_model.start_method))
      possible_arguments = [x.var_name for x in monitor_model.start_parameters if x.var_name]
      possible_arguments.extend([str(x.param_value) for x in monitor_model.start_parameters if x.param_value])
      cc.write(','.join(possible_arguments))
      cc.write(');\n')
  cc.write('\t\t}\n')


  # write the stop_monitor function
  cc.write('\n\n\t\tvoid {0}\n'.format(generate_stop_monitor_signature(block_model)))
  cc.write('\t\t{\n')

  # generate the code that stops the monitors
  for monitor_model in block_model.monitor_models:
    if monitor_model.stop_method:
      cc.write('\t\t\tmonitor::{0}.{1}('.format(monitor_model.monitor_name, monitor_model.stop_method))
      possible_arguments = [x.var_name for x in monitor_model.stop_parameters if x.var_name]
      possible_arguments.extend([str(x.param_value) for x in monitor_model.stop_parameters if x.param_value])
      cc.write(','.join(possible_arguments))
      cc.write(');\n')

  # if we have agora, we need to generate also the code that sends the information
  if not block_model.agora_model is None:
      cc.write('\n')

      # get the list of knobs, features and metrics
      knobs = sorted([x for x in block_model.agora_model.knobs_values])
      features = sorted([x for x in block_model.agora_model.features_values])
      metrics = sorted([x for x in block_model.agora_model.metrics_monitors])

      # start to compose the list of terms to send to
      knob_terms = []
      for knob in knobs:
          for knob_model in block_model.software_knobs:
              if knob_model.name == knob:
                  knob_var_name = knob_model.var_name
                  break
          knob_terms.append('std::to_string(knobs::{0})'.format(knob_var_name))
      knob_string = ' + "," + '.join(knob_terms)
      feature_terms = []
      for feature in features:
          feature_terms.append('std::to_string(features::{0})'.format(feature))
      feature_string = ' + "," + '.join(feature_terms)
      metric_terms = []
      for metric in metrics:
          related_monitor = block_model.agora_model.metrics_monitors[metric]
          metric_terms.append('std::to_string(monitor::{0}.last())'.format(related_monitor))
      metric_string = ' + "," + '.join(metric_terms)
      if feature_terms:
          send_string = ' + " " + '.join([knob_string, feature_string, metric_string])
      else:
          send_string = ' + " " + '.join([knob_string, metric_string])
      cc.write('\t\t\tmanager.send_observation({0});\n'.format(send_string))

  cc.write('\n')
  cc.write('\t\t}')


  # write the log function
  cc.write('\n\n\t\tvoid log( void )\n')
  cc.write('\t\t{\n')

  # at first update the exposed variables
  exposed_variables = []
  for monitor_model in block_model.monitor_models:
    for exposed_metric_name in monitor_model.exposed_metrics:
      exposed_variables.append(monitor_model.exposed_metrics[exposed_metric_name])
      cc.write('\t\t\t{0} = monitor::{1}.{2}();\n'.format(monitor_model.exposed_metrics[exposed_metric_name], monitor_model.monitor_name, what_translator[exposed_metric_name.upper()]))
  cc.write('\n')


  # keep track of what we are printing (for the cout)
  what_we_are_printing = []

  # compute the getters for the data features
  cluster_feature_printer = []
  data_feature_names = sorted([ x.name for x in block_model.features ])
  for index, feature_name in enumerate(data_feature_names):
    what_we_are_printing.append('Cluster {0}'.format(feature_name))
    cluster_feature_printer.append('{0}::manager.get_selected_feature<{1}>()'.format(block_model.block_name, index))


  # compute the getters for software knobs
  software_knobs_printers = []
  for knob in block_model.software_knobs:
    what_we_are_printing.append('Knob {0}'.format(knob.name))
    if block_model.block_name in op_lists:
      if knob.name in op_lists[block_model.block_name][list(op_lists[block_model.block_name].keys())[0]].translator:
        software_knobs_printers.append('{0}::knob_{2}_int2str[static_cast<int>({0}::manager.get_mean<OperatingPointSegments::SOFTWARE_KNOBS, static_cast<std::size_t>({0}::Knob::{1})>()]'.format(block_model.block_name, knob.name.upper(), knob.name.lower()))
        continue
    software_knobs_printers.append('{0}::manager.get_mean<OperatingPointSegments::SOFTWARE_KNOBS, static_cast<std::size_t>({0}::Knob::{1})>()'.format(block_model.block_name, knob.name.upper()))


  # compute the getters for the metrics
  metrics_printers = []
  for metric in block_model.metrics:
    what_we_are_printing.append('Expected {0}'.format(metric.name))
    metrics_printers.append('{0}::manager.get_mean<OperatingPointSegments::METRICS, static_cast<std::size_t>({0}::Metric::{1})>()'.format(block_model.block_name, metric.name.upper()))


  # compute the getters for the goals
  goal_printers = []
  for goal in block_model.goal_models:
    what_we_are_printing.append('Goal {0}'.format(goal.name))
    goal_printers.append('{0}::goal::{1}.get()'.format(block_model.block_name, goal.name))

  # compute the getters for the monitors
  monitor_printers = []
  for monitor_model in block_model.monitor_models:
    for exposed_var_what in monitor_model.exposed_metrics:
      what_we_are_printing.append('{0}'.format(monitor_model.exposed_metrics[exposed_var_what]))
      monitor_printers.append('{0}::{1}'.format(block_model.block_name, monitor_model.exposed_metrics[exposed_var_what]))


  # compute the getters for the observed data features
  data_feature_printer = []
  data_feature_names = sorted([ x.name for x in block_model.features ])
  for feature_name in data_feature_names:
    what_we_are_printing.append('Input {0}'.format(feature_name))
    data_feature_printer.append('{0}::features::{1}'.format(block_model.block_name, feature_name))


  # ------- actually compose the log functin on file

  cc.write('\n\t\t\t#ifdef MARGOT_LOG_FILE\n')
  if (block_model.metrics and block_model.software_knobs ):
      cc.write('\t\t\tif (!(manager.is_application_knowledge_empty() || manager.in_design_space_exploration()))\n')
      cc.write('\t\t\t{\n')


      # if we have ops it's easythen print the stuff
      things_to_print = list(cluster_feature_printer)
      things_to_print.extend(software_knobs_printers)
      things_to_print.extend(metrics_printers)
      things_to_print.extend(goal_printers)
      things_to_print.extend(monitor_printers)
      things_to_print.extend(data_feature_printer)
      string_to_print = ',\n\t\t\t\t\t'.join(things_to_print)
      cc.write('\t\t\t\tfile_logger.write(')
      cc.write('{0});\n'.format(string_to_print))


      cc.write('\t\t\t}\n')
      cc.write('\t\t\telse\n')
      cc.write('\t\t\t{\n')

      # if we have no ops, we must made up the expected stuff
      software_knobs_printers_alternative = ['{1}::knobs::{0}'.format(x.var_name, block_model.block_name) for x in block_model.software_knobs]
      metrics_printers_alternative = ['"N/A"' for x in metrics_printers]
      cluster_feature_printer_alternative = ['"N/A"' for x in cluster_feature_printer]

      # then print the stuff
      things_to_print = list(cluster_feature_printer_alternative)
      things_to_print.extend(software_knobs_printers_alternative)
      things_to_print.extend(metrics_printers_alternative)
      things_to_print.extend(goal_printers)
      things_to_print.extend(monitor_printers)
      things_to_print.extend(data_feature_printer)
      string_to_print = ',\n\t\t\t\t\t'.join(things_to_print)
      cc.write('\t\t\t\tfile_logger.write(')
      cc.write('{0});\n'.format(string_to_print))



      cc.write('\t\t\t}\n')
  else:
      things_to_print = list(goal_printers)
      things_to_print.extend(monitor_printers)
      things_to_print.extend(data_feature_printer)
      string_to_print = ',\n\t\t\t\t\t'.join(things_to_print)
      cc.write('\t\t\t\tfile_logger.write(')
      cc.write('{0});\n'.format(string_to_print))
  cc.write('\t\t\t#endif // MARGOT_LOG_FILE\n')



  # ------- actually compose the log functin on ostream


  cc.write('\n\t\t\t#ifdef MARGOT_LOG_STDOUT\n')

  if (block_model.metrics and block_model.software_knobs ):
      cc.write('\t\t\tif (!(manager.is_application_knowledge_empty() || manager.in_design_space_exploration()))\n')
      cc.write('\t\t\t{\n')

      # then print the stuff
      things_to_print = list(cluster_feature_printer)
      things_to_print.extend(software_knobs_printers)
      things_to_print.extend(metrics_printers)
      things_to_print.extend(goal_printers)
      things_to_print.extend(monitor_printers)
      things_to_print.extend(data_feature_printer)

      # set the endl from arguments
      endl_indexes = [len(cluster_feature_printer)]
      endl_indexes.append( endl_indexes[-1] + len(software_knobs_printers))
      endl_indexes.append( endl_indexes[-1] + len(metrics_printers))
      endl_indexes.append( endl_indexes[-1] + len(goal_printers))
      endl_indexes.append( endl_indexes[-1] + len(monitor_printers))
      endl_indexes.append( endl_indexes[-1] + len(data_feature_printer))

      # write the print statement
      composed_elements = []
      for index, content in enumerate(things_to_print):
        if index in endl_indexes:
          composed_elements.append('std::endl')
        string = '"[ {0} = " << {1} << "] "'.format(what_we_are_printing[index], content)
        composed_elements.append(string)
      string_to_print = '\n\t\t\t\t\t<< '.join(composed_elements)
      cc.write('\t\t\t\tstd::cout <<')
      cc.write('{0} << std::endl;\n'.format(string_to_print))


      cc.write('\t\t\t}\n')
      cc.write('\t\t\telse\n')
      cc.write('\t\t\t{\n')

      # then print the stuff
      things_to_print = list(cluster_feature_printer_alternative)
      things_to_print.extend(software_knobs_printers_alternative)
      things_to_print.extend(metrics_printers_alternative)
      things_to_print.extend(goal_printers)
      things_to_print.extend(monitor_printers)
      things_to_print.extend(data_feature_printer)

      # set the endl from arguments
      endl_indexes = [len(cluster_feature_printer_alternative)]
      endl_indexes.append( endl_indexes[-1] + len(software_knobs_printers_alternative))
      endl_indexes.append( endl_indexes[-1] + len(metrics_printers))
      endl_indexes.append( endl_indexes[-1] + len(goal_printers))
      endl_indexes.append( endl_indexes[-1] + len(monitor_printers))
      endl_indexes.append( endl_indexes[-1] + len(data_feature_printer))

      # write the print statement
      composed_elements = []
      for index, content in enumerate(things_to_print):
        if index in endl_indexes:
          composed_elements.append('std::endl')
        string = '"[ {0} = " << {1} << "] "'.format(what_we_are_printing[index], content)
        composed_elements.append(string)
      string_to_print = '\n\t\t\t\t\t<< '.join(composed_elements)
      cc.write('\t\t\t\tstd::cout <<')
      cc.write('{0} << std::endl;\n'.format(string_to_print))


      cc.write('\t\t\t}\n')
  else:
      things_to_print = list(goal_printers)
      things_to_print.extend(monitor_printers)
      things_to_print.extend(data_feature_printer)

      # set the endl from arguments
      endl_indexes = [len(goal_printers)]
      endl_indexes.append( endl_indexes[-1] + len(monitor_printers))
      endl_indexes.append( endl_indexes[-1] + len(data_feature_printer))

      # write the print statement
      composed_elements = []
      for index, content in enumerate(things_to_print):
        if index in endl_indexes:
          composed_elements.append('std::endl')
        string = '"[ {0} = " << {1} << "] "'.format(what_we_are_printing[index], content)
        composed_elements.append(string)
      string_to_print = '\n\t\t\t\t\t<< '.join(composed_elements)
      cc.write('\t\t\t\tstd::cout <<')
      cc.write('{0} << std::endl;\n'.format(string_to_print))
  cc.write('\t\t\t#endif // MARGOT_LOG_STDOUT\n')


  # close the log method
  cc.write('\t\t}\n')

  # write the end of the namespace
  cc.write('\n\t}} // namespace {0}\n'.format(block_model.block_name))









def generate_margot_cc( block_models, op_lists, output_folder ):
  """
  This function generates the actual margot source files
  """


  # open the output file
  with open(os.path.join(output_folder, 'margot.cc'), 'w') as cc:

    # write the include
    cc.write('#include <string>\n')
    cc.write('#include "margot.hpp"\n')
    cc.write('#include "margot_op_struct.hpp"\n')
    cc.write('#ifdef MARGOT_LOG_STDOUT\n')
    cc.write('#include <iostream>\n')
    cc.write('#endif // MARGOT_LOG_STDOUT\n')
    cc.write('#ifdef MARGOT_LOG_FILE\n')
    cc.write('#include "margot_logger.hpp"\n')
    cc.write('#endif // MARGOT_LOG_FILE\n')

    # write the disclaimer
    cc.write('\n\n\n/**\n')
    cc.write(' * WARNING:\n')
    cc.write(' * This file is autogenerated from the "margotcli" utility function.\n')
    cc.write(' * Any changes to this file might be overwritten, thus in order to \n')
    cc.write(' * perform a permanent change, please update the configuration file \n')
    cc.write(' * and re-generate this file. \n')
    cc.write(' */ \n\n\n')


    # write the margot namespace begin
    cc.write('namespace margot {\n')


    # generate the code block-specific code
    for block_name in block_models:
      generate_block_body(block_models[block_name], op_lists, cc)




    # ----------- generate the init function signature
    cc.write('\n\n\tvoid init( ')

    # get all the creation parameters
    creation_parameters = []
    monitor_models = []
    for block_name in block_models:
      monitor_models.extend( block_models[block_name].monitor_models )
    for monitor in monitor_models:
      creation_parameters.extend( monitor.creation_parameters )

    # compose the parameter list
    signature = ', '.join(['{0} {1}'.format(x.var_type, x.var_name) for x in creation_parameters if x.var_name])
    if not signature:
      signature = 'void'

    cc.write('{0} )\n'.format(signature))



    # ----------- generate the init function body
    cc.write('\t{\n')

    # loop over the blocks
    for block_name in block_models:

      # writing a preamble
      cc.write('\n\n\t\t// --------- Initializing the block "{0}"\n'.format(block_name.upper()))

      # get the reference to the block
      block_model = block_models[block_name]

      # generate the monitor initialization
      for monitor_model in block_model.monitor_models:
        cc.write('\t\t{0}::monitor::{1} = {2}('.format(block_name,monitor_model.monitor_name, monitor_model.monitor_class))
        creation_params = [x.var_name for x in monitor_model.creation_parameters if x.var_name]
        creation_params.extend( [str(x.param_value) for x in monitor_model.creation_parameters if x.param_value] )
        cc.write('{0});\n'.format(', '.join(creation_params)))

      # set the goal to their actual values
      for goal_model in block_model.goal_models:
        cc.write('\t\t{0}::goal::{1}.set({2});\n'.format(block_name, goal_model.name, goal_model.value))



      # check if we have any Operating Points
      if block_name in op_lists:
        cc.write('\n\t\t// Adding the application knowledge\n')

        # get all the op list for data cluster
        op_list_ids = sorted(op_lists[block_name].keys())

        # loop over them to create Operating Points
        for index, op_list_feature_id in enumerate(op_list_ids):

          # make a tuple out of the id
          data_feature = '{{{{{0}}}}}'.format(op_list_feature_id.replace('|',', '))

          # check if we actually have to create a data cluster
          if block_model.features:

            # insert the call that creates the feature cluster
            cc.write('\t\t{0}::manager.add_feature_cluster({1});\n'.format(block_name, data_feature))

            # insert the call which selects the feature cluster
            cc.write('\t\t{0}::manager.select_feature_cluster({1});\n'.format(block_name, data_feature))

          # eventually, insert the call which adds the Operating Points
          cc.write('\t\t{0}::manager.add_operating_points({0}::op_list{1});\n'.format(block_name, index))
      else:

        # if we have no Operating Point lists, but we do have data features,
        # we need to create a dummy cluster
        if block_model.features:
            dummy_feature = '{{{{{0}}}}}'.format(','.join(['0' for x in range(len(block_model.features))]))
            cc.write('\t\t{0}::manager.add_feature_cluster({1});\n'.format(block_name, dummy_feature))
            cc.write('\t\t{0}::manager.select_feature_cluster({1});\n'.format(block_name, dummy_feature))
      cc.write('\n')


      # check if we need to insert runtime information providers
      if block_model.field_adaptor_models:
        cc.write('\n\t\t// Adding runtime information provider(s)\n')
        for field_adaptor_model in block_model.field_adaptor_models:

          # check if the adaptor is in the metric
          if field_adaptor_model.metric_name:

            # get the index of the metric (TODO check if we can remove the cast)
            metric_index = 'static_cast<std::size_t>({0}::Metric::{1})'.format(block_name, field_adaptor_model.metric_name.upper())

            # write the instruction to the file
            cc.write('\t\t{0}::manager.add_runtime_knowledge<OperatingPointSegments::METRICS, {1}, {2}>({0}::monitor::{3});\n'.format(
              block_name, metric_index, field_adaptor_model.inertia, field_adaptor_model.monitor_name
              ))

          # check if the adaptor is in the software knob
          if field_adaptor_model.knob_name:

            # get the index of the metric
            knob_index = 'static_cast<std::size_t>({0}::Knob::{1})'.format(block_name, field_adaptor_model.knob_name.upper())

            # write the instruction to the file
            cc.write('\t\t{0}::manager.add_runtime_knowledge<OperatingPointSegments::SOFTWARE_KNOBS, {1}, {2}({0}::monitor::{3});\n'.format(
              block_name, metric_index, field_adaptor_model.inertia, field_adaptor_model.monitor_name
              ))


      # writing a preamble
      cc.write('\n\n\t\t// --------- Defining the application requirements for block "{0}"\n'.format(block_name.upper()))

      # loop over the states
      for state_model in block_model.state_models:

        # add & switch to the current state
        cc.write('\n\t\t// Defining the state "{0}"\n'.format(state_model.name))
        cc.write('\t\t{0}::manager.create_new_state("{1}");\n'.format(block_name, state_model.name))
        cc.write('\t\t{0}::manager.change_active_state("{1}");\n'.format(block_name, state_model.name))

        # i'm going to define the rank
        rank_objective = state_model.rank_available_directions[state_model.rank_direction]
        rank_composition = state_model.rank_available_combination_types[state_model.rank_type]

        # get the list of the rank fields
        rank_fields = []
        for rank_field_model in state_model.rank_fields:
          if rank_field_model.metric_name:
            metric_index = 'static_cast<std::size_t>({0}::Metric::{1})'.format(block_name, rank_field_model.metric_name.upper())
            rank_fields.append('OPField<OperatingPointSegments::METRICS,BoundType::LOWER,{0},0>'.format(metric_index))
          if rank_field_model.knob_name:
            knob_index = 'static_cast<std::size_t>({0}::Knob::{1})'.format(block_name, rank_field_model.knob_name.upper())
            rank_fields.append('OPField<OperatingPointSegments::SOFTWARE_KNOBS,BoundType::LOWER,{0},0>'.format(knob_index))

        # get the rank coefs
        rank_coefs = [str(float(x.coefficient)) for x in state_model.rank_fields]
        funcion_coefs = ', '.join(rank_coefs)

        # set the template arguments
        template_args = '{0}, {1}, {2} '.format(
          state_model.rank_available_directions[state_model.rank_direction],
          state_model.rank_available_combination_types[state_model.rank_type],
          ', '.join(rank_fields)
          )

        # print the template args
        if rank_fields:
            cc.write('\n\t\t // Defining the application rank\n')
            cc.write('\t\t{0}::manager.set_rank<{1}>({2});\n'.format(block_name, template_args, funcion_coefs))

        # loop over the constraints
        for constraint_model in state_model.constraint_list:
          if constraint_model.target_knob:
            cc.write('\t\t{0}::manager.add_constraint<OperatingPointSegments::SOFTWARE_KNOBS,{1},{2}>({0}::goal::{3}, {4});\n'.format(
            block_name,
            'static_cast<std::size_t>({0}::Knob::{1})'.format(block_name, constraint_model.target_knob.upper()),
            constraint_model.confidence,
            constraint_model.goal_ref,
            constraint_model.priority
            ))
          if constraint_model.target_metric:
            cc.write('\t\t{0}::manager.add_constraint<OperatingPointSegments::METRICS,{1},{2}>({0}::goal::{3}, {4});\n'.format(
            block_name,
            'static_cast<std::size_t>({0}::Metric::{1})'.format(block_name, constraint_model.target_metric.upper()),
            constraint_model.confidence,
            constraint_model.goal_ref,
            constraint_model.priority
            ))


      # switch to the active state
      if block_model.state_models:
        active_state = block_model.state_models[0]
        for state_model in block_model.state_models:
          if state_model.starting:
            active_state = state_model
            break
        cc.write('\n\t\t// Switch to the starting active state\n')
        cc.write('\t\t{0}::manager.change_active_state("{1}");\n'.format(block_name, active_state.name))


      # store the information to print
      cc.write('\n\t\t// Initialize the log file\n')
      cc.write('\t\t#ifdef MARGOT_LOG_FILE\n')
      if (block_model.metrics and block_model.software_knobs ):
          data_feature_names = sorted([x.name for x in block_model.features])
          things_to_print = ['\t\t\t"Cluster_{0}"'.format(x.upper()) for x in data_feature_names]
          things_to_print.extend(['\t\t\t"Knob_{0}"'.format(x.name.upper()) for x in block_model.software_knobs])
          things_to_print.extend(['\t\t\t"Known_Metric_{0}"'.format(x.name.upper()) for x in block_model.metrics])
          things_to_print.extend(['\t\t\t"Goal_{0}"'.format(x.name.upper()) for x in block_model.goal_models])
          for monitor_model in block_model.monitor_models:
            for exposed_var_what in monitor_model.exposed_metrics:
              things_to_print.append('\t\t\t"{1}_{0}"'.format(exposed_var_what, monitor_model.monitor_name.upper()))
          things_to_print.extend(['\t\t\t"Input_{0}"'.format(x.upper()) for x in data_feature_names])
          cc.write('\t\t{0}::file_logger.open("{0}.log", margot::Format::PLAIN,\n{1});\n'.format(block_name, ',\n'.join(things_to_print)))
      else:
          data_feature_names = sorted([x.name for x in block_model.features])
          things_to_print = ['\t\t\t"Goal_{0}"'.format(goal.name.upper()) for x in block_model.goal_models]
          for monitor_model in block_model.monitor_models:
            for exposed_var_what in monitor_model.exposed_metrics:
              things_to_print.append('\t\t\t"{1}_{0}"'.format(exposed_var_what, monitor_model.monitor_name.upper()))
          things_to_print.extend(['\t\t\t"Input_{0}"'.format(x.upper()) for x in data_feature_names])
          cc.write('\t\t{0}::file_logger.open("{0}.log", margot::Format::PLAIN,\n{1});\n'.format(block_name, ',\n'.join(things_to_print)))
      cc.write('\t\t#endif // MARGOT_LOG_FILE\n')


      # if we have the agora application local handler, we have to spawn the support thread
      if not block_model.agora_model is None:

          # get all the knobs, metrics and features of the block
          knob_list = sorted([x.name for x in block_model.software_knobs])
          metric_list = sorted([x.name for x in block_model.metrics])
          feature_list = sorted([x.name for x in block_model.features])

          # define the type of a metric
          metrics_type = []
          for metric_model in block_model.metrics:
            metrics_type.append(metric_model.type)
          if 'int' in metrics_type:
            metric_type_pod = 'float'
          if 'float' in metrics_type:
            metric_type_pod = 'float'
          if 'double' in metrics_type:
            metric_type_pod = 'double'

          # define the type of a software knobs
          knobs_type = []
          for knob_model in block_model.software_knobs:
            knobs_type.append(knob_model.var_type)
          if 'int' in knobs_type:
            knob_type_pod = 'int'
          if 'float' in knobs_type:
            knob_type_pod = 'float'
          if 'double' in knobs_type:
            knob_type_pod = 'double'

          # define the type of a feature
          features_type = []
          for feature_model in block_model.features:
            features_type.append(feature_model.type)
          if 'int' in features_type:
            feature_type_pod = 'int'
          if 'float' in features_type:
            feature_type_pod = 'float'
          if 'double' in features_type:
            feature_type_pod = 'double'

          # compose the descrition of the application
          description_terms = []
          for knob_name in knob_list:
               description_terms.append('knob      {0} {1} {2}'.format(knob_name, knob_type_pod, block_model.agora_model.knobs_values[knob_name]))
          for feature_name in feature_list:
               description_terms.append('feature   {0} {1} {2}'.format(feature_name, feature_type_pod, block_model.agora_model.features_values[feature_name]))
          for metric_name in metric_list:
               description_terms.append('metric    {0} {1} {2}'.format(metric_name, metric_type_pod, block_model.agora_model.metrics_predictors[metric_name]))
          description_terms.append('doe       {0}'.format(block_model.agora_model.doe))
          description_terms.append('num_obser {0}'.format(block_model.agora_model.number_observation))
          description_string = '@'.join(description_terms)

          # generate the argument of the initialization
          app_name = block_model.agora_model.application_name
          broker_url = block_model.agora_model.broker_url
          broker_username = block_model.agora_model.username
          broker_password = block_model.agora_model.password
          broker_ca = block_model.agora_model.broker_ca
          client_cert = block_model.agora_model.client_cert
          client_key = block_model.agora_model.client_key
          mqtt_qos = block_model.agora_model.qos
          parameter_string = '"{0}","{1}","{2}","{3}",{4},"{5}","{6}","{7}","{8}"'.format(app_name,broker_url,broker_username,broker_password,mqtt_qos,description_string,broker_ca,client_cert,client_key )

          # eventually emit the code that starts the agora application local handler
          cc.write('\n\t\t// Start the agora pocal application handler thread\n')
          cc.write('\t\t{0}::manager.start_support_thread<{0}::operating_point_parser_t>({1});\n'.format(block_name, parameter_string))

    cc.write('\t}\n')

    # write some trailer spaces
    cc.write('\n\n')

    # write the margot namespace end
    cc.write('} // namespace margot\n\n')
