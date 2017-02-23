from .parser_utility import get_elements
from .parser_utility import get_parameter
from . import model_learn_knob



def parse_learn_knob( knob_xml_element, namespace = ''):
	"""
	Parse a software knob that should be learned dynamically
	"""

	# create a new model of the knob
	knob_model = model_learn_knob.LearningKnobModel()

	# parse the param name
	knob_model.name = get_parameter(knob_xml_element, 'name')

	# parse the variable name
	knob_model.var_name = get_parameter(knob_xml_element, 'var_name')

	# parse the variable type
	knob_model.var_type = get_parameter(knob_xml_element, 'var_type')

	# parse the rank coefficient
	knob_model.coefficient = get_parameter(knob_xml_element, 'rank_coef')

	# get all the possible values
	xml_choices_elements = get_elements(knob_xml_element, 'choice', namespace = namespace, required = True)

	# loop over them
	for xml_choices_element in xml_choices_elements:

		# get the value
		knob_model.values.append(get_parameter(xml_choices_element, 'value'))

	return knob_model
