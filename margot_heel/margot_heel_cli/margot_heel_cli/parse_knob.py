from .parser_utility import get_elements
from .parser_utility import get_parameter
from . import model_knob



def parse_knob( knob_xml_element, namespace = ''):
  """
  Parse a monitor from XML and generate its model
  """

  # create a new model of the knob
  knob_model = model_knob.KnobModel()

  # parse the param name (make sure that the name is lowercase)
  knob_model.name = get_parameter(knob_xml_element, 'name').lower()

  # parse the variable name
  knob_model.var_name = get_parameter(knob_xml_element, 'var_name')

  # parse the variable type
  knob_model.var_type = get_parameter(knob_xml_element, 'var_type', prefixed_values = knob_model.available_var_types).lower()

  return knob_model
