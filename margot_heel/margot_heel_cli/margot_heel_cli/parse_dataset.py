from .parser_utility import get_elements
from .parser_utility import get_parameter
from .parse_input_data import parse_input_data
from . import model_dataset




def parse_dataset( dataset_xml_element, namespace = ''):
  """
  Parse the datasets information from XML and generate its model
  """
  #initialize a new data structure for the model of the dataset
  dataset_model = model_dataset.Dataset()

  #get the domain for the dataset types
  available_dataset_types = dataset_model.available_dataset_types

  #parse XML for the dataset type and assign it to the model (training/production)
  dataset_type = get_parameter(dataset_xml_element, 'type', required=True, prefixed_values = available_dataset_types)
  dataset_model.type = dataset_type

  #parse XML for the structure of the specific dataset data and add them to the model
  input_data_xml_elements = get_elements(dataset_xml_element, 'input_data', required = True, namespace = namespace)
  for input_data_xml_element in input_data_xml_elements:
    input_data_model = parse_input_data(input_data_xml_element, namespace = namespace )
    dataset_model.input_data_models.append(input_data_model)

  # check if the number of values for each input data in a dataset is the same
  valid = True
  for x in range(1,len(dataset_model.input_data_models)):
      if (len(dataset_model.input_data_models[0].values) != len(dataset_model.input_data_models[x].values)):
          valid = False
          break
  if not valid:
      raise Exception("The number of values for each input data in a dataset must be the same!")

  return dataset_model
