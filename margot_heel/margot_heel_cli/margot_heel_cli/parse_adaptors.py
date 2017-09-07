from .parser_utility import get_elements
from .parser_utility import get_parameter
from . import model_adaptors



def parse_adaptor( adaptor_xml_element, namespace = ''):
  """
  Parse a field adaptor from XML and generate its model
  """

  # create a new model of the field adaptor
  adaptor_model = model_adaptors.FieldAdaptorModel()

  # parse the name of the target monitor
  adaptor_model.monitor_name = get_parameter(adaptor_xml_element, 'using')

  # parse the inertia
  inertia = get_parameter(adaptor_xml_element, 'inertia', required = False, my_target_type = int )
  if inertia:
    adaptor_model.inertia = max(inertia,1)
  else:
    adaptor_model.inertia = 1

  # check if it targets a metric
  metric_name = get_parameter(adaptor_xml_element, 'metric_name', required=False)
  if metric_name:
    adaptor_model.metric_name = metric_name
    return adaptor_model


  # check if it targets a software knob
  knob_name = get_parameter(adaptor_xml_element, 'knob_name', required=False)
  if knob_name:
    adaptor_model.knob_name = knob_name
    return adaptor_model

  # we should not reach this point if everything is ok
  print('[LOGIC_ERROR] Unable to understand the adaptor for monitor "{0}"'.format(adaptor_model.monitor_name))
  print('              The list of attributes:')
  print_node_attributes(adaptor_xml_element)
  sys.exit(-1)
  return adaptor_model
