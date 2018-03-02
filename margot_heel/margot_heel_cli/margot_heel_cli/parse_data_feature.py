from .parser_utility import get_elements
from .parser_utility import get_parameter
from . import model_data_feature



def parse_data_feature( data_feature_xml_element, namespace = ''):
  """
  Parse a monitor from XML and generate its model
  """

  # create a new model of the knob
  feature_model = model_data_feature.DataFeatureModel()

  # get the distance type
  feature_model.distance = get_parameter(data_feature_xml_element, 'distance', prefixed_values = feature_model.available_feature_combination).upper()

  # get all the data features
  feature_xml_elements = get_elements(data_feature_xml_element, 'feature', namespace = namespace)

  # parse each data feature
  for feature_xml_element in feature_xml_elements:

    # parse the feature name (make sure that the name is lowercase)
    feature_name = get_parameter(feature_xml_element, 'name').lower()

    # parse the feature type
    feature_type = get_parameter(feature_xml_element, 'type', prefixed_values = feature_model.available_var_types).lower()

    # parse the feature comparison function
    feature_cf = get_parameter(feature_xml_element, 'comparison', prefixed_values = feature_model.available_comparison_functions).upper()

    # add the information to the feature model
    feature_model.add_feature(feature_name, feature_type, feature_cf)

  return feature_model
