from .parser_utility import get_elements
from .parser_utility import get_parameter
from .model_parameter import ParameterModel
import sys


def parse_parameter( parameter_xml_element, namespace = ''):
  # get the name
  my_param_name = get_parameter(parameter_xml_element, 'name')

  # declare the list of parameters
  defined_parameters_list = []

  # parse all the fixed parameters
  fixed_parameters_xml = get_elements(parameter_xml_element, 'fixed', namespace = namespace)
  for param_xml in fixed_parameters_xml:

    # read the value
    my_param_value = get_parameter(param_xml, 'value')

    # create the param model
    my_param_model = ParameterModel().create_from_fixed(my_param_value)
    my_param_model.param_name = my_param_name

    # append it
    defined_parameters_list.append(my_param_model)

  # parse all the local parameters
  local_parameters_xml = get_elements(parameter_xml_element, 'local_var', namespace = namespace)
  for param_xml in local_parameters_xml:

    # read the value
    my_param_var_name = get_parameter(param_xml, 'name')
    my_param_var_type = get_parameter(param_xml, 'type')

    # create the param model
    my_param_model = ParameterModel().create_from_local(my_param_var_name, my_param_var_type)
    my_param_model.param_name = my_param_name

    # append it
    defined_parameters_list.append(my_param_model)


  # check if there is at least one parameter
  if not defined_parameters_list:
    print('[LOGIC_ERROR]: Unable to find any actual definition of a parameters!')
    sys.exit(-1)


  # check if we have at most one parameter
  if len(defined_parameters_list) > 1:
    print('[WARNING]: Defined more than one parameters in a tag <{0}>, pretending that there is just one!'.format(parameter_xml_element.tag))

  return defined_parameters_list[0]
