from .parser_utility import get_elements
from .parser_utility import get_parameter
from . import model_agora



def parse_agora( agora_xml_element, namespace = ''):
  """
  Parse the agora local handler information from XML and generate its model
  """

  # create a new model
  agora_model = model_agora.AgoraModel()

  # parse all the agora attributes
  broker_url = get_parameter(agora_xml_element, 'address', required = False)
  username = get_parameter(agora_xml_element, 'username', required = False)
  password = get_parameter(agora_xml_element, 'password', required = False )
  broker_ca = get_parameter(agora_xml_element, 'broker_ca', required = False )
  client_cert = get_parameter(agora_xml_element, 'client_cert', required = False)
  client_key = get_parameter(agora_xml_element, 'client_key', required = False)
  qos = get_parameter(agora_xml_element, 'qos', required = False)

  # update the model
  if broker_url:
      agora_model.broker_url = broker_url
  if username:
      agora_model.username = username
  if password:
      agora_model.password = password
  if broker_ca:
      agora_model.broker_ca = broker_ca
  if client_cert:
      agora_model.client_cert = client_cert
  if client_key:
      agora_model.client_key = client_key
  if qos:
      agora_model.qos = qos

  # get all the knobs
  knobs_xml_elements = get_elements(agora_xml_element, 'explore', namespace = namespace, required = True )

  # parse each knob to explore
  for knob_xml_element in knobs_xml_elements:

    # parse the knob characteristic
    knob_name = get_parameter(knob_xml_element, 'knob_name')
    knob_values = get_parameter(knob_xml_element, 'values')

    # make sure that all the knob names are lowercase
    knob_name = knob_name.lower()

    # validate the values
    knob_values = knob_values.replace(',', ' ')
    values = knob_values.split(' ')
    valid_values = [x for x in values if x and x.replace('.','',1).replace('-','',1).isdigit()]
    knob_values = ' '.join(valid_values)

    # add the information to the agora model
    agora_model.knobs_values[knob_name] = knob_values

  # get all the knobs
  features_xml_elements = get_elements(agora_xml_element, 'given', namespace = namespace )

  # parse each knob to explore
  for feature_xml_element in features_xml_elements:

    # parse the feature characteristic
    feature_name = get_parameter(feature_xml_element, 'feature_name')
    feature_values = get_parameter(feature_xml_element, 'values')

    # make sure that all the feature names are lowercase
    feature_name = feature_name.lower()

    # validate the values
    feature_values = feature_values.replace(',', ' ')
    values = feature_values.split(' ')
    valid_values = [x for x in values if x and x.replace('.','',1).replace('-','',1).isdigit()]
    feature_values = ' '.join(valid_values)

    # add the information to the agora model
    agora_model.features_values[feature_name] = feature_values


  # get all the metrics
  metrics_xml_elements = get_elements(agora_xml_element, 'predict', namespace = namespace )

  # parse each knob to explore
  for metric_xml_element in metrics_xml_elements:

    # parse the metric characteristic
    metric_name = get_parameter(metric_xml_element, 'metric_name')
    metric_predictor = get_parameter(metric_xml_element, 'prediction')
    metric_monitor = get_parameter(metric_xml_element, 'monitor')

    # add the information to the agora model
    agora_model.metrics_predictors[metric_name] = metric_predictor
    agora_model.metrics_monitors[metric_name] = metric_monitor

  return agora_model
