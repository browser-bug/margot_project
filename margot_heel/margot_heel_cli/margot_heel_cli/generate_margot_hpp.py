import os                                # for creating the path
import errno                             # for checking before creating a path


from .generate_utility import generate_start_monitor_signature
from .generate_utility import generate_stop_monitor_signature
from .generate_utility import generate_update_signature



from .model_data_feature import DataFeatureModel



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
      cc.write('\t\t\textern {0} {1};\n'.format(monitor_model.monitor_class, monitor_model.monitor_name))

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
      cc.write('\t\t\textern {1} {0};\n'.format(goal_model.name, goal_model.get_c_goal_type(goal_type)))

    # close the goal namespace
    cc.write('\t\t} // namespace goal\n')

  # write the observed data features variables (if any)
  if (block_model.features):

    # open the data feature namespace
    cc.write('\n\t\tnamespace features {\n')

    # loop over the features
    for feature in block_model.features:
      cc.write('\t\t\textern {1} {0};\n'.format(feature.name, feature.type))

    # close the feature namespace
    cc.write('\t\t} // namespace features\n')


  # open the software knobs namespace
  cc.write('\n\t\tnamespace knobs {\n')

  # loop over the knobs
  for knob_model in block_model.software_knobs:
    cc.write('\t\t\textern {1} {0};\n'.format(knob_model.var_name, knob_model.var_type))

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

        # loop over the data feature fields to compose the da asrtm type
        features_cf = []
        for name in names:
          # get the corresponding data feature comparison function
          feature_cf = [ x.cf for x in block_model.features if x.name == name][0]
          features_cf.append(cfun_feature_translator[feature_cf])

        # print the new type of the asrtm
        cc.write('\n\n\t\textern DataAwareAsrtm< Asrtm< MyOperatingPoint >, {0}, margot::FeatureDistanceType::{1}, {2} > manager;\n\n\n'.format(features_type, block_model.feature_distance, ', '.join(features_cf)))
      else:
        cc.write('\n\n\t\textern Asrtm< MyOperatingPoint > manager;\n\n\n')

  # write the list of operating points (one for each data feature)
  for op_list_name in op_lists:
    if block_model.block_name == op_list_name:

      # get all the op list for data cluster
      op_list_ids = sorted(op_lists[op_list_name].keys())

      # loop over the op list for data features
      cc.write('\n')
      for index, data_feature_id in enumerate(op_list_ids):
        cc.write('\n\t\textern std::vector< MyOperatingPoint > op_list{0};'.format(index))
      cc.write('\n\n\n')

  # write all the exposed variables from the monitors
  for monitor_model in block_model.monitor_models:
    for exposed_var_what in monitor_model.exposed_metrics:
      cc.write('\n\t\textern {1}::statistical_type {0};\n'.format(monitor_model.exposed_metrics[exposed_var_what], monitor_model.monitor_class))

  # write the update function
  cc.write('\n\n\t\tbool {0};\n'.format(generate_update_signature(block_model)))


  # write the start_monitor function
  cc.write('\n\n\t\tvoid {0};\n'.format(generate_start_monitor_signature(block_model)))


  # write the stop_monitor function
  cc.write('\n\n\t\tvoid {0};\n'.format(generate_stop_monitor_signature(block_model)))


  # write the log function
  cc.write('\n\n\t\tvoid log( void );\n')


  # write the block macro
  cc.write('\n\n#ifndef MARGOT_MANAGED_BLOCK_{0}\n'.format(block_model.block_name.upper()))
  cc.write('#define MARGOT_MANAGED_BLOCK_{0} \\\n'.format(block_model.block_name.upper()))
  if (block_model.metrics and block_model.software_knobs ):
      cc.write('if (margot::{0}::{1}) {{\\\n'.format(block_model.block_name, generate_update_signature(block_model, True)))
      if block_model.state_models:
        cc.write('margot::{0}::manager.configuration_applied();}}\\\n'.format(block_model.block_name))
      else:
        cc.write('}\\\n')
  cc.write('margot::{0}::{1};\\\n'.format(block_model.block_name, generate_start_monitor_signature(block_model, True)))
  cc.write('for(bool flag = true; flag == true; margot::{0}::{1}, margot::{0}::log(), flag = false )\n'.format(block_model.block_name, generate_stop_monitor_signature(block_model, True)))
  cc.write('#endif // MARGOT_MANAGED_BLOCK_')
  # put some ending spacing
  cc.write('\n\n')


  # write the end of the namespace
  cc.write('\n\t}} // namespace {0}\n'.format(block_model.block_name))









def generate_margot_hpp( block_models, op_lists, output_folder ):
  """
  This function generates the actual margot source files
  """


  # open the output file
  with open(os.path.join(output_folder, 'margot.hpp'), 'w') as cc:


    # write the include guard
    cc.write('#ifndef MARGOT_CC_HEADER_H\n')
    cc.write('#define MARGOT_CC_HEADER_H\n')

    # get all the required headers
    required_headers = [ '<margot/asrtm.hpp>', '<margot/da_asrtm.hpp>', '<cstddef>', '<vector>', '"margot_op_struct.hpp"' ]
    for block_name in block_models:
      required_headers.extend( x.monitor_header for x in block_models[block_name].monitor_models )
    required_headers = reversed(sorted(list(set(required_headers))))
    for h in required_headers:
      cc.write('#include {0}\n'.format(h))


    # write the output flags
    cc.write('\n\n// decomment/comment these macros to enable/disable features\n')
    cc.write('#define MARGOT_LOG_STDOUT\n')
    cc.write('#define MARGOT_LOG_FILE\n')

    # put some new lines
    cc.write('\n\n')

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

    cc.write('{0} );\n'.format(signature))




    # write some trailer spaces
    cc.write('\n\n')

    # write the margot namespace end
    cc.write('} // namespace margot\n\n')
    cc.write('#endif // MARGOT_CC_HEADER_H\n\n')
