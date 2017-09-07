import os                                # for creating the path
import errno                             # for checking before creating a path


from .generate_utility import generate_start_monitor_signature
from .generate_utility import generate_stop_monitor_signature
from .generate_utility import generate_update_signature



dfun_translator = {
  "AVERAGE"  : "margot::DataFunction::Average",
  "VARIANCE" : "margot::DataFunction::Variance",
  "MAX"      : "margot::DataFunction::Max",
  "MIN"      : "margot::DataFunction::Min",
}


cfun_translator = {
  "GT"  : "margot::ComparisonFunction::Greater",
  "GE"  : "margot::ComparisonFunction::GreaterOrEqual",
  "LT"  : "margot::ComparisonFunction::Less",
  "LE"  : "margot::ComparisonFunction::LessOrEqual",
}

what_translator = {
  "AVERAGE"  : "average",
  "VARIANCE" : "variance",
  "MAX"      : "max",
  "MIN"      : "min",
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
  cc.write('\t\t\t\treturn conf_changed;\n')
  cc.write('\t\t\t}\n')
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
  for monitor_model in block_model.monitor_models:
    if monitor_model.stop_method:
      cc.write('\t\t\tmonitor::{0}.{1}('.format(monitor_model.monitor_name, monitor_model.stop_method))
      possible_arguments = [x.var_name for x in monitor_model.stop_parameters if x.var_name]
      possible_arguments.extend([str(x.param_value) for x in monitor_model.stop_parameters if x.param_value])
      cc.write(','.join(possible_arguments))
      cc.write(');\n')
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


  # compute the getters for software knobs
  software_knobs_printers = []
  for knob in block_model.software_knobs:
    what_we_are_printing.append('Knob {0}'.format(knob.name))
    if block_model.block_name in op_lists:
      if knob.name in op_lists[block_model.block_name].translator:
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


  # ------- actually compose the log functin on file

  cc.write('\n\t\t\t#ifdef MARGOT_LOG_FILE\n')
  cc.write('\t\t\tif (!manager.is_application_knowledge_empty())\n')
  cc.write('\t\t\t{\n')


  # if we have ops it's easythen print the stuff
  things_to_print = list(software_knobs_printers)
  things_to_print.extend(metrics_printers)
  things_to_print.extend(goal_printers)
  things_to_print.extend(monitor_printers)
  string_to_print = ',\n\t\t\t\t\t'.join(things_to_print)
  cc.write('\t\t\t\tfile_logger.write(')
  cc.write('{0});\n'.format(string_to_print))


  cc.write('\t\t\t}\n')
  cc.write('\t\t\telse\n')
  cc.write('\t\t\t{\n')

  # if we have no ops, we must made up the expected stuff
  software_knobs_printers_alternative = ['"N/A"' for x in software_knobs_printers]
  metrics_printers_alternative = ['"N/A"' for x in metrics_printers]

  # then print the stuff
  things_to_print = list(software_knobs_printers_alternative)
  things_to_print.extend(metrics_printers_alternative)
  things_to_print.extend(goal_printers)
  things_to_print.extend(monitor_printers)
  string_to_print = ',\n\t\t\t\t\t'.join(things_to_print)
  cc.write('\t\t\t\tfile_logger.write(')
  cc.write('{0});\n'.format(string_to_print))



  cc.write('\t\t\t}\n')
  cc.write('\t\t\t#endif // MARGOT_LOG_FILE\n')



  # ------- actually compose the log functin on ostream


  cc.write('\n\t\t\t#ifdef MARGOT_LOG_STDOUT\n')
  cc.write('\t\t\tif (!manager.is_application_knowledge_empty())\n')
  cc.write('\t\t\t{\n')

  # then print the stuff
  things_to_print = list(software_knobs_printers)
  things_to_print.extend(metrics_printers)
  things_to_print.extend(goal_printers)
  things_to_print.extend(monitor_printers)

  # set the endl from arguments
  endl_indexes = [len(software_knobs_printers)]
  endl_indexes.append( endl_indexes[-1] + len(metrics_printers))
  endl_indexes.append( endl_indexes[-1] + len(goal_printers))
  endl_indexes.append( endl_indexes[-1] + len(monitor_printers))

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


  # if we have no ops, we must made up the expected stuff
  software_knobs_printers_alternative = ['"N/A"' for x in software_knobs_printers]
  metrics_printers_alternative = ['"N/A"' for x in metrics_printers]

  # then print the stuff
  things_to_print = list(software_knobs_printers_alternative)
  things_to_print.extend(metrics_printers_alternative)
  things_to_print.extend(goal_printers)
  things_to_print.extend(monitor_printers)

  # set the endl from arguments
  endl_indexes = [len(software_knobs_printers)]
  endl_indexes.append( endl_indexes[-1] + len(metrics_printers))
  endl_indexes.append( endl_indexes[-1] + len(goal_printers))
  endl_indexes.append( endl_indexes[-1] + len(monitor_printers))

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


      # check if we need to add Operating Points
      ops_key = op_lists.keys()
      if block_name in ops_key:
        cc.write('\n\t\t{0}::manager.add_operating_points({0}::op_list);\n'.format(block_name))


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
            rank_fields.append('OPField<OperatingPointSegments::SOFTWARE_KNOBS,BoundType::LOWER,{0},0>'.format(metric_index))
          if rank_field_model.knob_name:
            knob_index = 'static_cast<std::size_t>({0}::Knob::{1})'.format(block_name, rank_field_model.knob_name.upper())
            rank_fields.append('OPField<OperatingPointSegments::METRICS,BoundType::LOWER,{0},0>'.format(metric_index))

        # get the rank coefs
        rank_coefs = [str(float(x.coefficient)) for x in state_model.rank_fields]

        # set the template arguments
        template_args = '{0}, {1}, {2} '.format(
          state_model.rank_available_directions[state_model.rank_direction],
          state_model.rank_available_combination_types[state_model.rank_type],
          ', '.join(rank_fields)
          )

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

      # initialize the file logger
      cc.write('\n\n\t\t#ifdef MARGOT_LOG_FILE\n')
      cc.write('\t\t{0}::file_logger.open("{0}.log", margot::Format::PLAIN,\n'.format(block_name))
      if block_model.block_name in op_lists.keys():
        for knob in block_model.software_knobs:
          cc.write('\t\t\t"Knob_{0}",\n'.format(knob.name.upper()))
        for metric in block_model.metrics:
          cc.write('\t\t\t"Known_Metric_{0}",\n'.format(metric.name.upper()))
      for goal in block_model.goal_models:
        cc.write('\t\t\t"goal_{0}",\n'.format(goal.name))
      for monitor_model in block_model.monitor_models:
        for exposed_var_what in monitor_model.exposed_metrics:
          cc.write('\t\t\t"{1}_{0}",\n'.format(exposed_var_what, monitor_model.monitor_name))
      cc.seek(-2, 1)
      cc.write(');\n')
      cc.write('\t\t#endif // MARGOT_LOG_FILE\n')

    cc.write('\t}\n')

    # write some trailer spaces
    cc.write('\n\n')

    # write the margot namespace end
    cc.write('} // namespace margot\n\n')
