import os                                # for creating the path
import errno                             # for checking before creating a path


from .generate_utility import generate_start_monitor_signature
from .generate_utility import generate_stop_monitor_signature
from .generate_utility import generate_update_signature



def generate_block_body( block_model, op_list, cc ):
  """
  Generates the per block code
  """

  cc.write('\n\n\t// The interface for the managed block "{0}"\n'.format(block_model.block_name))

  # write the update function
  cc.write('\n\n\tint margot_{0}_{1}\n'.format(block_model.block_name, generate_update_signature(block_model, c_language = True)))
  cc.write('\t{\n')
  cc.write('\t\treturn static_cast<int>(margot::{0}::{1});\n'.format(block_model.block_name, generate_update_signature(block_model, True, c_language = True)))
  cc.write('\t}')


  # write the start_monitor function
  cc.write('\n\n\tvoid margot_{0}_{1}\n'.format(block_model.block_name, generate_start_monitor_signature(block_model)))
  cc.write('\t{\n')
  cc.write('\t\tmargot::{0}::{1};\n'.format(block_model.block_name, generate_start_monitor_signature(block_model, True)))
  cc.write('\t}')


  # write the log function
  cc.write('\n\n\tvoid margot_{0}_log( void )\n'.format(block_model.block_name))
  cc.write('\t{\n')
  cc.write('\t\tmargot::{0}::log();\n'.format(block_model.block_name))
  cc.write('\t}')


  # write the stop_monitor function
  cc.write('\n\n\tvoid margot_{0}_{1}\n'.format(block_model.block_name, generate_stop_monitor_signature(block_model)))
  cc.write('\t{\n')
  cc.write('\t\tmargot::{0}::{1};\n'.format(block_model.block_name, generate_stop_monitor_signature(block_model, True)))
  cc.write('\t}')
  
  # write the "has_model()" function
  cc.write('\n\n\tint margot_{0}_has_model( void )\n'.format(block_model.block_name))
  cc.write('\t{\n')
  cc.write('\t\t return static_cast<int>(margot::{0}::has_model());\n'.format(block_model.block_name))
  cc.write('\t}')  

  # write the configuration applied function
  if (block_model.metrics and block_model.software_knobs ):
      cc.write('\n\n\tvoid margot_{0}_configuration_applied( void )\n'.format(block_model.block_name))
      cc.write('\t{\n')
      cc.write('\t\tmargot::{0}::manager.configuration_applied();\n'.format(block_model.block_name))
      cc.write('\t}')

      # write the change state function
      cc.write('\n\n\tvoid margot_{0}_change_state( const char* new_state )\n'.format(block_model.block_name))
      cc.write('\t{\n')
      cc.write('\t\tmargot::{0}::manager.change_active_state(std::string(new_state));\n'.format(block_model.block_name))
      cc.write('\t}')

  cc.write('\n\n')


  # write the function that set the value of the goal
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
    cc.write('\n\tvoid margot_{0}_goal_{1}_set_value(const {2} new_value)\n'.format(
    block_model.block_name,
    goal_model.name,
    goal_type
    ))
    cc.write('\t{\n')
    cc.write('\t\tmargot::{0}::goal::{1}.set(new_value);\n'.format(block_model.block_name, goal_model.name))
    cc.write('\t}')


  cc.write('\n\n')


  # write the translating function

  # write the translating function
  if op_list:
    for string_param_name in op_list[list(op_list.keys())[0]].translator:

      # write the function to convert from str to int
      cc.write('\n\tvoid margot_{0}_knob_{1}_int2str(const int numeric_value, char const ** string_value)\n'.format(block_model.block_name, string_param_name.lower()))
      cc.write('\t{\n')
      cc.write('\t\t*string_value = margot::{0}::knob_{1}_int2str.at(numeric_value).c_str();\n'.format(block_model.block_name, string_param_name.lower()))
      cc.write('\t}\n')
      cc.write('\tvoid margot_{0}_knob_{1}_str2int(int* numeric_value, const char* string_value)\n'.format(block_model.block_name, string_param_name.lower()))
      cc.write('\t{\n')
      cc.write('\t\t*numeric_value = margot::{0}::knob_{1}_str2int.at(std::string(string_value));\n'.format(block_model.block_name, string_param_name.lower()))
      cc.write('\t}\n')



  # put some ending spacing
  cc.write('\n\n')











def generate_margot_c( block_models, op_lists, output_folder ):
  """
  This function generates the actual margot source files
  """


  # open the output file
  with open(os.path.join(output_folder, 'margot_c_interface.cc'), 'w') as cc:

    # add the required headers
    cc.write('#include <string>\n')
    cc.write('extern "C" {\n')
    cc.write('\t#include "margot.h"\n')
    cc.write('}\n')
    cc.write('#include "margot.hpp"\n')


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


    cc.write('extern "C" {\n')


    # generate the code block-specific code
    for block_name in block_models:
      if block_name in op_lists:
        generate_block_body(block_models[block_name], op_lists[block_name], cc)
      else:
        generate_block_body(block_models[block_name], None, cc)


    cc.write('\n\n\t// The global initialization method\n')


    # ----------- generate the init function signature
    cc.write('\n\n\tvoid margot_init( ')

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
    cc.write('\t{\n')
    cc.write('\t\tmargot::init(')
    signature = ', '.join(['{1}'.format(x.var_type, x.var_name) for x in creation_parameters if x.var_name])
    if not signature:
      signature = ''
    cc.write('{0} );\n'.format(signature))
    cc.write('\t}\n')




    cc.write('}\n')


    # write some trailer spaces
    cc.write('\n\n')
