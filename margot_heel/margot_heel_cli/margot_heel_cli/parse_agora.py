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
  agora_model.broker_url = get_parameter(agora_xml_element, 'address')
  agora_model.username = get_parameter(agora_xml_element, 'username')
  agora_model.password = get_parameter(agora_xml_element, 'password')
  agora_model.broker_ca = get_parameter(agora_xml_element, 'broker_ca')
  agora_model.client_cert = get_parameter(agora_xml_element, 'client_cert')
  agora_model.client_key = get_parameter(agora_xml_element, 'client_key')
  agora_model.qos = get_parameter(agora_xml_element, 'qos')
  agora_model.doe = get_parameter(agora_xml_element, 'doe', prefixed_values = agora_model.available_does).lower()
  agora_model.number_observation = get_parameter(agora_xml_element, 'observations')
  
  # get (all) the "pause" element, even though it should be just one element
  pause_xml_elements = get_elements(agora_xml_element, 'pause', namespace = namespace, unique = True)

  #parse the pause information
  if pause_xml_elements:

      # parse it
      agora_model.pause_polling = get_parameter(pause_xml_elements[0], 'polling_time', required = True)
      if int(agora_model.pause_polling)<0:
        raise Exception("Invalid pause polling value. Please insert an integer > 0!")
      agora_model.pause_timeout = get_parameter(pause_xml_elements[0], 'timeout', required = True)
      if int(agora_model.pause_timeout)<-1:
        raise Exception("Invalid pause timeout value. Please insert an integer > -2!")

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
