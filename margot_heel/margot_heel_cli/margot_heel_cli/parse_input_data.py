from .parser_utility import get_elements
from .parser_utility import get_parameter
from . import model_input_data



def parse_input_data( input_data_xml_element, namespace = ''):
  """
  Parse the input data information from XML and generate its model
  """
  #initialize a new data structure for the model of the input data
  input_data_model = model_input_data.InputData()

  #get the name of the input data and add it to the model
  input_data_name = get_parameter(input_data_xml_element, 'name', required=True)
  input_data_model.name = input_data_name

  #get the domain for the input data types
  available_input_data_types = input_data_model.available_var_types

  #get the type of the input data and add it to the model
  input_data_type = get_parameter(input_data_xml_element, 'type', required=True, prefixed_values = available_input_data_types)
  input_data_model.type = input_data_type

  #get the values of the input data
  input_data_values = get_parameter(input_data_xml_element, 'values', required=True)
  data_values = input_data_values.replace(',', ' ')
  values = data_values.split(' ')
  #data_values = ' '.join(values)
  #input_data_model.data_values[input_data_name] = data_values
  input_data_model.values = values;
  
  return input_data_model
