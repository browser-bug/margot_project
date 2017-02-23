from .parser_utility import get_elements
from .parser_utility import get_parameter
from .parse_param import parse_parameter
from . import model_monitor


def parse_monitor( monitor_xml_element, namespace = ''):
	"""
	Parse a monitor from XML and generate its model
	"""

	###################################
	## PARSING THE SPEC AND PROPERTIES
	###################################

	# create the monitor model
	my_monitor_model = model_monitor.MonitorModel()

	# get the monitor name
	my_monitor_model.monitor_name = get_parameter(monitor_xml_element, 'name')

	# get the known monitor type
	known_monitor_types = model_monitor.know_monitor_spec

	# get the monitor type
	monitor_type = get_parameter(monitor_xml_element, 'type', prefixed_values = known_monitor_types)

	# retrieve the spec of the monitor
	monitor_spec = model_monitor.get_monitor_spec(monitor_type)

	# check if it is not known
	if not monitor_spec:


		# get the spec element
		spec_element_xml = get_elements(monitor_xml_element, 'spec', required = True, namespace = namespace, unique = True)[0]

		# parsing the class name
		class_name_xml = get_elements(spec_element_xml, 'class', required = True, namespace = namespace, unique = True)[0]
		class_name = get_parameter(class_name_xml, 'name', required = True)
		my_monitor_model.monitor_class = class_name

		# parsing the haeder that defines the monitor
		header_xml = get_elements(spec_element_xml, 'header', required = True, namespace = namespace, unique = True)[0]
		header_name = '<{0}>'.format(get_parameter(header_xml, 'reference', required = True))
		my_monitor_model.monitor_header = header_name

		# parsing the type of elements stored in the monitor
		type_xml = get_elements(spec_element_xml, 'type', required = True, namespace = namespace, unique = True)[0]
		type_name = get_parameter(type_xml, 'name', required = True)
		my_monitor_model.monitor_type = type_name

		# parsing the name of the stop method
		stop_method_xml = get_elements(spec_element_xml, 'stop_method', namespace = namespace, unique = True)
		if stop_method_xml:
			stop_method_name = get_parameter(stop_method_xml[0], 'name', required = True)
			my_monitor_model.stop_method = stop_method_name

		# parsing the name of the start method
		start_method_xml = get_elements(spec_element_xml, 'start_method', namespace = namespace, unique = True)
		if start_method_xml:
			start_method_name = get_parameter(start_method_xml[0], 'name', required = True)
			my_monitor_model.start_method = start_method_name
	else:
		monitor_spec.monitor_name = my_monitor_model.monitor_name
		my_monitor_model = monitor_spec


	###################################
	## PARSING THE IN/OUT PARAMETERS
	###################################

	# parse the creation parameters
	creation_parameters_xml = get_elements(monitor_xml_element, 'creation', namespace = namespace, unique = True)
	if creation_parameters_xml:
		param_list_xml =  get_elements(creation_parameters_xml[0], 'param', namespace = namespace)
		for param_xml in param_list_xml:
			param_model = parse_parameter(param_xml, namespace = namespace)
			my_monitor_model.creation_parameters.append(param_model)

	# parse the start parameters
	start_parameters_xml = get_elements(monitor_xml_element, 'start', namespace = namespace, unique = True)
	if start_parameters_xml:
		param_list_xml =  get_elements(start_parameters_xml[0], 'param', namespace = namespace)
		for param_xml in param_list_xml:
			param_model = parse_parameter(param_xml, namespace = namespace)
			my_monitor_model.start_parameters.append(param_model)

	# parse the stop parameters
	stop_parameters_xml = get_elements(monitor_xml_element, 'stop', namespace = namespace, unique = True)
	if stop_parameters_xml:
		param_list_xml =  get_elements(stop_parameters_xml[0], 'param', namespace = namespace)
		for param_xml in param_list_xml:
			param_model = parse_parameter(param_xml, namespace = namespace)
			my_monitor_model.stop_parameters.append(param_model)


	# parse the exposed metrics
	list_of_exposed_metrics_xml = get_elements(monitor_xml_element, 'expose', namespace = namespace, unique = True)
	if list_of_exposed_metrics_xml:
		for exposed_metric_xml in list_of_exposed_metrics_xml:
			exposed_var_name = get_parameter(exposed_metric_xml, 'var_name')
			exposed_var_what = get_parameter(exposed_metric_xml, 'what', prefixed_values = my_monitor_model.output_available_metrics )
			my_monitor_model.exposed_metrics[exposed_var_what] = exposed_var_name

	# check the consistency for the monitor
	my_monitor_model.check_consistency()

	# return it
	return my_monitor_model
