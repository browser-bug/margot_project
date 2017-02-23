from .parser_utility import get_elements
from .parser_utility import get_parameter
from .parser_utility import print_node_attributes
from . import model_goal
import sys



def parse_goal( goal_xml_element, namespace = ''):

	# create a Goal model
	my_goal_model = model_goal.GoalModel()

	# parse the name of the goal
	my_goal_model.name = get_parameter(goal_xml_element, 'name')

	# parse the comparison function
	my_goal_model.cfun = get_parameter(goal_xml_element, 'cFun', prefixed_values = my_goal_model.available_cfuns )

	# parse the goal value
	my_goal_model.value = get_parameter(goal_xml_element, 'value')

	# try to retrieve a monitor reference
	monitor_ref = get_parameter(goal_xml_element, 'monitor', required=False)

	# check if it is a dynimc goal
	if monitor_ref:

		# set the right attributes
		my_goal_model.monitor_name_ref = monitor_ref
		my_goal_model.goal_type = 'DYNAMIC'

		# parse the data function
		my_goal_model.dfun = get_parameter(goal_xml_element, 'dFun')

		return my_goal_model

	# check if it is static goal on a metric
	metric_name_ref = get_parameter(goal_xml_element, 'metric_name', required=False)
	if metric_name_ref:
		my_goal_model.metric_name_ref = metric_name_ref
		my_goal_model.goal_type = 'STATIC'
		return my_goal_model


	# check if it is static goal on a parameter
	parameter_name_ref = get_parameter(goal_xml_element, 'knob_name', required=False)
	if parameter_name_ref:
		my_goal_model.parameter_name_ref = parameter_name_ref
		my_goal_model.goal_type = 'STATIC'
		return my_goal_model



	# we should not reach this point if everything is ok
	print('[LOGIC_ERROR] Unable to understand the goal "{0}"'.format(my_goal_model.name))
	print('              The list of attributes:')
	print_node_attributes(goal_xml_element)
	sys.exit(-1)
	return None
