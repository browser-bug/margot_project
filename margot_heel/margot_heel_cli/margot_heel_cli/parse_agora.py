import sys

from .parser_utility import get_elements
from .parser_utility import get_parameter
from . import model_agora


def my_to_str( float_value ):
    """
    Utility function to convert a value in string as float or integer
    """
    if abs(float(int(float_value)) - float_value) < 0.000000001:
        return str(int(float_value))
    else:
        return str(float_value)



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
  doe_name = get_parameter(agora_xml_element, 'doe_name', required = False, prefixed_values = agora_model.available_does )
  number_point_per_dimension = get_parameter(agora_xml_element, 'number_point_per_dimension', my_target_type=int, required = False)
  number_observations_per_point = get_parameter(agora_xml_element, 'number_observations_per_point', my_target_type=int, required = False)
  min_distance = get_parameter(agora_xml_element, 'min_distance', my_target_type=float, required = False)

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
  if doe_name:
      agora_model.doe_name = doe_name
  if number_point_per_dimension:
      agora_model.number_point_per_dimension = number_point_per_dimension
  if number_observations_per_point:
      agora_model.number_observations_per_point = number_observations_per_point
  if min_distance:
      if min_distance < 0 or min_distance > 1:
          print('[LOGIC ERROR] The minimum distance parameter of the agora tag must be [0..1]')
          print('                    Provided value: "{0}"'.format(min_distance))
          sys.exit(-1)
      agora_model.min_distance = min_distance

  # get all the knobs
  knobs_xml_elements = get_elements(agora_xml_element, 'explore', namespace = namespace, required = True )

  # parse each knob to explore
  for knob_xml_element in knobs_xml_elements:

    # parse the knob name (it could be whatever)
    knob_name = get_parameter(knob_xml_element, 'knob_name')

    # tries to guess that we have a list of values for the knob
    knob_values = get_parameter(knob_xml_element, 'values', required = False)

    # otherwise it could be a range of values
    if not knob_values:

        # parse them as a range of values
        knob_range = get_parameter(knob_xml_element, 'range', required = False)

        # make sure that we have something
        if not knob_range:
            print('[LOGIC ERROR] Unable to parse the knob "{0}" domain!'.format(knob_name))
            print('                    Available attributes: "{0}"'.format('", "'.join(knob_xml_element.attrib)))
            print('                    Required attribute "values" or "range"')
            sys.exit(-1)

        # parse the string
        try:
            min_value, max_value, step = knob_range.split(',')
            min_value = float(min_value)
            max_value = float(max_value)
            step = float(step)
        except ValueError as err:
            print('[LOGIC ERROR] Unable to understand the range "{0}"!'.format(knob_range))
            print('                    Range Syntax: "min_value,max_value,step"')
            sys.exit(-1)

        # generate the discrete knob values
        min_value = min(min_value,max_value)
        max_value = max(min_value,max_value)
        knob_value_list = [ min_value ]
        counter = min_value + step
        while counter <= max_value:
            knob_value_list.append(counter)
            counter = counter + step

        # join them for later processing
        knob_values = ' '.join([my_to_str(x) for x in knob_value_list])



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

    # parse the feature name
    feature_name = get_parameter(feature_xml_element, 'feature_name')

    # tries to guess that we have a list of values for the feature
    feature_values = get_parameter(feature_xml_element, 'values', required = False)

    # otherwise it could be a range of values
    if not feature_values:

        # parse them as a range of values
        feature_range = get_parameter(feature_xml_element, 'range', required = False)

        # make sure that we have something
        if not feature_range:
            print('[LOGIC ERROR] Unable to parse the feature "{0}" domain!'.format(feature_name))
            print('                    Available attributes: "{0}"'.format('", "'.join(feature_xml_element.attrib)))
            print('                    Required attribute "values" or "range"')
            sys.exit(-1)

        # parse the string
        try:
            min_value, max_value, step = feature_range.split(',')
            min_value = float(min_value)
            max_value = float(max_value)
            step = float(step)
        except ValueError as err:
            print('[LOGIC ERROR] Unable to understand the range "{0}"!'.format(feature_range))
            print('                    Range Syntax: "min_value,max_value,step"')
            sys.exit(-1)

        # generate the discrete knob values
        min_value = min(min_value,max_value)
        max_value = max(min_value,max_value)
        feature_value_list = [ min_value ]
        counter = min_value + step
        while counter <= max_value:
            feature_value_list.append(counter)
            counter = counter + step

        # join them for later processing
        feature_values = ' '.join([my_to_str(x) for x in feature_value_list])

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
