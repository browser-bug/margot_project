from .parser_utility import get_elements
from .parser_utility import get_parameter
from . import model_metric



def parse_metric( metric_xml_element, namespace = ''):
  """
  Parse a metric from XML and generate its model
  """

  # create a new model of the knob
  metric_model = model_metric.MetricModel()

  # parse the param name
  metric_model.name = get_parameter(metric_xml_element, 'name')

  # parse the variable name
  metric_model.type = get_parameter(metric_xml_element, 'type', prefixed_values = metric_model.available_types).lower()

  # parse the variable type
  distribution = get_parameter(metric_xml_element, 'distribution', required = False)
  if distribution:
    if distribution.upper() == 'YES' or distribution == '1' or distribution.upper() == 'ON': 
      metric_model.distribution = True
    else:
      metric_model.distribution = False
  else:
    metric_model.distribution = True

  return metric_model
