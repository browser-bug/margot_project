from .parser_utility import get_elements
from .parser_utility import get_parameter
from .parse_constraint import parse_constraint
from . import model_state



def parse_state( state_xml_element, namespace = ''):
	"""
	Parse a state from the XML description
	"""

	# create a new state
	my_state_model = model_state.StateModel()

	# parse the name of the state
	my_state_model.name = get_parameter(state_xml_element, 'name')

	# try to parse if this is the starting state
	starting_state = get_parameter(state_xml_element, 'starting', required = False)

	# check if this is a starting state
	if starting_state:
		my_state_model.starting = True
	else:
		my_state_model.starting = False

	# parse the rank
	rank_xml_element = get_elements(state_xml_element, my_state_model.rank_available_directions, required = True, namespace = namespace, unique = True)[0]

	# check in which the direction is going
	my_state_model.set_rank_direction(rank_xml_element.tag)

	# check the type of combination
	my_state_model.rank_type = get_parameter(rank_xml_element, 'combination').upper()

	# get all the components of the rank
	rank_component_xml_elements = get_elements(rank_xml_element, my_state_model.rank_available_components, required = True, namespace = namespace)

	# parse each component
	for rank_component_xml_element in rank_component_xml_elements:

		# get the name of the field
		field_name = get_parameter(rank_component_xml_element, 'name')
		coef = get_parameter(rank_component_xml_element, 'coef')

		# check if it is a parameter component
		if rank_component_xml_element.tag == 'knob':
			my_state_model.rank_element_param[field_name] = coef
		if rank_component_xml_element.tag == 'metric':
			my_state_model.rank_element_metric[field_name] = coef

	# get all the constraints elements
	constraint_xml_elements = get_elements(state_xml_element, 'subject', namespace = namespace)

	# parse each constraint
	for constraint_xml_element in constraint_xml_elements:

		# parse it
		constraint_model = parse_constraint(constraint_xml_element, namespace = namespace)
		# add it to the list
		my_state_model.constraint_list.append(constraint_model)

	return my_state_model
