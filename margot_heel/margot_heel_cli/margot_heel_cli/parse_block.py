from .parser_utility import get_elements
from .parser_utility import get_parameter
from .parse_monitor import parse_monitor
from .parse_goal import parse_goal
from .parse_knob import parse_knob
from .parse_state import parse_state
from .parse_adaptors import parse_adaptor
from .parse_metrics import parse_metric
from .parse_data_feature import parse_data_feature
from .parse_agora import parse_agora
from .parse_dataset import parse_dataset
from . import model_block


def parse_block_xml( xml_block_root, namespace = '' ):
  """
  This function parse the XML block element and all its subelements
  """

  # read the block name
  block_name = get_parameter(xml_block_root, 'name')

  # build the block model
  model = model_block.BlockModel(block_name)

  # get all the monitors
  monitor_xml_elements = get_elements(xml_block_root, 'monitor', namespace = namespace)

  # parse each monitor
  for monitor_xml_element in monitor_xml_elements:

    # get the monitor model
    monitor_model = parse_monitor( monitor_xml_element, namespace = namespace )

    # add the monitor model
    model.monitor_models.append(monitor_model)


  # get all the goals
  goal_xml_elements = get_elements(xml_block_root, 'goal', namespace = namespace)

  # parse each goal
  for goal_xml_element in goal_xml_elements:

    # get the goal model
    goal_model = parse_goal( goal_xml_element, namespace = namespace )

    # add the goal model
    model.goal_models.append(goal_model)


  # get all the software knobs
  knob_xml_elements = get_elements(xml_block_root, 'knob', namespace = namespace)

  # parse each knob
  for knob_xml_element in knob_xml_elements:

    # get the knob model
    knob_model = parse_knob(knob_xml_element, namespace = namespace)

    # add the knob model
    model.software_knobs.append(knob_model)


  # get the possible data feature element
  feature_xml_element = get_elements(xml_block_root, 'features', unique = True, namespace = namespace)
  if feature_xml_element:

    # get the first element of the data features
    feature_xml_element = feature_xml_element[0]

    # get the model of data features
    data_feature_model = parse_data_feature(feature_xml_element, namespace = namespace)

    # append the model to the block
    model.features = data_feature_model.features
    model.feature_distance = data_feature_model.distance

  # get all the field adaptors
  adaptor_xml_elements = get_elements(xml_block_root, 'adapt', namespace = namespace)

  # parse each field adaptors
  for adaptor_xml_element in adaptor_xml_elements:

    # get the field adaptors model
    adaptor_model = parse_adaptor(adaptor_xml_element, namespace = namespace)

    # add the field adaptors model
    model.field_adaptor_models.append(adaptor_model)

  # get all the metrics
  metric_xml_elements = get_elements(xml_block_root, 'metric', namespace = namespace)

  # parse each metric
  for metric_xml_element in metric_xml_elements:

    # get the metric model
    metric_model = parse_metric(metric_xml_element, namespace = namespace)

    # add the metric model
    model.metrics.append(metric_model)

  # get all the states
  state_xml_elements = get_elements(xml_block_root, 'state', namespace = namespace)

  # get the agora xml elements
  agora_xml_elements = get_elements(xml_block_root, 'agora', namespace = namespace, unique = True)

  #parse the agora information
  if agora_xml_elements:

      # parse it
      model.agora_model = parse_agora(agora_xml_elements[0], namespace = namespace )

  # get the datasets xml elements
  datasets_xml_elements = get_elements(xml_block_root, 'datasets', namespace = namespace, unique = True)

  #parse the datasets element
  if datasets_xml_elements:
    dataset_xml_elements = get_elements(datasets_xml_elements[0], 'dataset', required=True, namespace = namespace)
    
    #if the user does not provide exactly 2 datasets (training/production) then an exception is raised
    if len(dataset_xml_elements)!=2:
      raise Exception("Wrong number of datasets provided. Insert just one training and one production dataset!")
    
    #parse each dataset element and add it to the model
    for dataset_xml_element in dataset_xml_elements:
      dataset_model = parse_dataset(dataset_xml_element, namespace = namespace )
      model.datasets_model.append(dataset_model)

    # datasets validity checker
    valid = True
    if not (len(model.datasets_model[0].input_data_models) == len(model.datasets_model[1].input_data_models)):
        valid = False
    if not valid:
        raise Exception("The training and production datasets must have the same structure in terms of parameters quantity!")
    for x in range(len(model.datasets_model[0].input_data_models)):
        if not (model.datasets_model[0].input_data_models[x].name == model.datasets_model[1].input_data_models[x].name):
            valid = False
            break
    if not valid:
        raise Exception("The training and production datasets must have the same structure in terms of parameters name!")
    for x in range(len(model.datasets_model[0].input_data_models)):
        if not (model.datasets_model[0].input_data_models[x].type == model.datasets_model[1].input_data_models[x].type):
            valid = False
            break
    if not valid:
        raise Exception("The training and production datasets must have the same structure in terms of parameters type!")

  # parse each state
  for state_xml_element in state_xml_elements:

    # get the state model
    state_model = parse_state(state_xml_element, namespace = namespace )

    # add the state model
    model.state_models.append(state_model)



  return model
