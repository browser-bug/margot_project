from .parser_utility import get_elements
from .parser_utility import get_parameter
from .parse_monitor import parse_monitor
from .parse_goal import parse_goal
from .parse_knob import parse_knob
from .parse_state import parse_state
from .parse_learn_model import parse_learn_model
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

	# get all the learning model
	learning_xml_elements = get_elements(xml_block_root, 'learn', namespace = namespace, unique = True )
	if learning_xml_elements:
		model.learn_models.append(parse_learn_model(learning_xml_elements[0], namespace = namespace))

	# get the configuration function
	conf_function_xml = get_elements(xml_block_root, 'set', namespace = namespace, unique = True )
	if conf_function_xml:
		conf_function_xml = conf_function_xml[0]

		# get the name of the configure function
		model.configure_function = get_parameter(conf_function_xml, 'configure_function')

	# get all the states
	state_xml_elements = get_elements(xml_block_root, 'state', namespace = namespace)

	# parse each state
	for state_xml_element in state_xml_elements:

		# get the state model
		state_model = parse_state(state_xml_element, namespace = namespace )

		# add the state model
		model.state_models.append(state_model)



	return model
