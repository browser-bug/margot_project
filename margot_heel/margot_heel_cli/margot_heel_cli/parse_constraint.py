from .parser_utility import get_elements
from .parser_utility import get_parameter
from . import model_constraint



def parse_constraint( constraint_xml_element, namespace = ''):
	"""
	Parse a constraint from the XML description
	"""

	# create a new model
	my_constraint_model = model_constraint.ConstraintModel()

	# parse the goal reference
	my_constraint_model.goal_ref = get_parameter(constraint_xml_element, 'to')

	# parse the priority
	my_constraint_model.priority = get_parameter(constraint_xml_element, 'priority', my_target_type = int)

	# parse the target metric name
	target_name = get_parameter(constraint_xml_element, 'metric_name', required = False)

	if target_name:
		my_constraint_model.target_metric = target_name
		my_constraint_model.target_knob = ''

	# parse the target knob name
	target_name = get_parameter(constraint_xml_element, 'knob_name', required = False)

	if target_name:
		my_constraint_model.target_metric = ''
		my_constraint_model.target_knob = target_name

	return my_constraint_model
