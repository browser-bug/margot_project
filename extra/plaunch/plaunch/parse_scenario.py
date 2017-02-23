from .parser_utility import get_elements as get_elements
from .parser_utility import get_parameter as get_parameter
from .parser_utility import parse_xml_file as parse
import model_scenario
from .application import Application as Application
import sys





def parse_scenario( path_scenario ):
	"""
	This function parse a scenario and returns it model
	"""

	# parse the scenario xml file
	xml_root, namespace = parse(path_scenario)

	# creates a new scenario model
	my_scenario_model = model_scenario.ScenarioModel()




	# ------ From the schedule check all the application involved




	# get the schedule element
	schedule_element_xml =  get_elements(xml_root, 'schedule', required = True, namespace = namespace, unique = True)[0]

	# get all the running instances
	run_elements_xml = get_elements(schedule_element_xml, 'run', required = True, namespace = namespace)

	# parse all the isntance specific options
	for index, run_element_xml in enumerate(run_elements_xml):

		# parse the delay
		parsed_delay = get_parameter(run_element_xml, 'delay', my_target_type=int )

		# parse the application name
		application_name = get_parameter(run_element_xml, 'application')

		# creates a new application
		my_application = Application()
		my_application.delay = parsed_delay
		my_application.name = '{0}.{1}'.format(application_name, index)
		my_application.application_name = application_name
		my_application.id = str(index)

		# get all the application specific flags
		opt_elements_xml = get_elements(run_element_xml, 'opt', namespace = namespace)
		for opt_element_xml in opt_elements_xml:

			# parse the flag
			flag_name = get_parameter(opt_element_xml, 'name')
			flag_value = get_parameter(opt_element_xml, 'value')

			# add it to the application
			my_application.run_flags[flag_name] = flag_value

		# append the application model to the scenario
		my_scenario_model.applications.append(my_application)




	# ------ Check the details of all the involved applications

	# get all the application elements
	application_elements_xml = get_elements(xml_root, 'application', namespace = namespace)

	# loop over the required applications
	for application in my_scenario_model.applications:

		# assume that the description is not found
		desc_found = False

		# loop over the known applications
		for application_element_xml in application_elements_xml:

			# parse the name
			app_name = get_parameter(application_element_xml, 'name')

			# check if it is the one that we are looking for
			if not app_name == application.application_name:
				continue

			# parse the path to the binary
			bin_element_xml = get_elements(application_element_xml, 'executable', required = True, namespace = namespace, unique = True)[0]
			application.binary_file = get_parameter(bin_element_xml, 'path')

			# parse the app flags
			argument_element_xml = get_elements(application_element_xml, 'arguments', namespace = namespace, unique = True)
			if argument_element_xml:
				argument_element_xml = argument_element_xml[0]
				opt_elements_xml = get_elements(argument_element_xml, 'opt', required = True, namespace = namespace)
				for opt_element_xml in opt_elements_xml:
					# parse the flag
					flag_name = get_parameter(opt_element_xml, 'name')
					flag_value = get_parameter(opt_element_xml, 'value')

					# add it to the application
					application.run_flags[flag_name] = flag_value

			# parse the dependecies
			dependencies_element_xml = get_elements(application_element_xml, 'dependency', namespace = namespace, unique = True)
			if dependencies_element_xml:
				dependencies_element_xml = dependencies_element_xml[0]
				file_elements_xml = get_elements(dependencies_element_xml, 'file', namespace = namespace)
				for dep_element_xml in file_elements_xml:
					# parse the dep
					path = get_parameter(dep_element_xml, 'path')
					dest = get_parameter(dep_element_xml, 'dest')

					# add it to the application
					application.input_files[path] = dest
				dirs_elements_xml = get_elements(dependencies_element_xml, 'folder', namespace = namespace)
				for dep_element_xml in dirs_elements_xml:
					# parse the dep
					path = get_parameter(dep_element_xml, 'path')
					dest = get_parameter(dep_element_xml, 'dest')

					# add it to the application
					application.input_dirs[path] = dest

			# parse the log files
			logs_element_xml = get_elements(application_element_xml, 'log', namespace = namespace, unique = True)
			if logs_element_xml:
				logs_element_xml = logs_element_xml[0]
				log_elements_xml = get_elements(logs_element_xml, 'file', required = True, namespace = namespace)
				for log_element_xml in log_elements_xml:
					application.log_files.append( get_parameter(log_element_xml, 'value') )

			# set to true the found flag
			desc_found = True

		# check if the application is known
		if not desc_found:
			print('[LOGIC_ERROR] No description found for the application "{0}"'.format(application.name))
			sys.exit(-1)


	# return the parsed scenario model
	return my_scenario_model
