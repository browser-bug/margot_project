from .parser_utility import get_elements
from .parser_utility import get_parameter
from .model_learn import LearningModel
from .parse_learn_knob import parse_learn_knob
import sys


def parse_learn_model( xml_learn_root, namespace = '' ):
	"""
	This function parse the learn element looking for software knob that should be learned online
	"""

	# create a new learn model
	my_learn_model = LearningModel()

	# parse the framework to be used
	my_learn_model.framework_type = get_parameter(xml_learn_root, 'method', prefixed_values = my_learn_model.available_framework)

	# check if it is been specified a sigma coefficient
	window_size = get_parameter(xml_learn_root, 'window_size', required=False)
	if window_size:

		# set it in the learning model
		my_learn_model.window_size = window_size

		# check if it is been specified the window coefficient
		sigma_value = get_parameter(xml_learn_root, 'sigma', required=False)
		if sigma_value:

			# set it in the learning model
			my_learn_model.sigma = sigma_value

			# check if it is been specified the balance coefficient
			balance_coef = get_parameter(xml_learn_root, 'balance_coef', required=False)
			if balance_coef:

				# set it in the learning model
				my_learn_model.balance_coef = balance_coef

	# get all the software-knob that should be learned dynamically
	xml_learn_knobs_elements = get_elements(xml_learn_root, 'knob', namespace = namespace, required = True)

	# loop over them
	for xml_learn_knobs_element in xml_learn_knobs_elements:
		my_learn_model.knobs.append(parse_learn_knob(xml_learn_knobs_element, namespace))

	return my_learn_model
