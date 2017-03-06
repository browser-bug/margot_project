import os
import inspect
import sys


#################################
###### Import argo library
#################################
try:
	import margot_heel_cli
except ImportError:

	# Add the python scripts folder
	src_folder = os.path.realpath(os.path.join(os.path.split(inspect.getfile( inspect.currentframe() ))[0], "..", "..", "margot_heel", "margot_heel_cli"))
	sys.path.insert(1, src_folder)

	# Import again the library
	try:
		import margot_heel_cli

	except ImportError:
		print('[ERROR]: Unable to find the margot_heel_cli library!')
		print('         Possible causes:')
		print('             - argotool is not installed as python package')
		print('             - This file has been moved wrt the library position')
		print(src_folder)
		sys.exit(-1)

from margot_heel_cli import parser_utility as p


#################################
###### The application real class
#################################


class Application:
	"""
	This class represent the application that must be profiled

	Arguments:
		- name        (str)   -> The name of the application
		- log_file    (str)   -> The name of the logfile produced from the application
		- block       (str)   -> The name of the block involved in the DSE
		- flags       (list)  -> The list of application-wide flags
		- knob_values (dic)   -> The dictionary of the knobs to explore
		                         key   (str)   -> knob name
		                         value (list)  -> the list of all the values of the knob
		- knob_flags  (dic)   -> The dictionary of the knobs to explore
		                         key   (str)   -> knob name
		                         value (str)   -> the flag used to drive the dse
		- metrics     (dic)   -> The dictionary of the metric to read from the execution trace
		                         key   (str)   -> metric name
		                         value (str)   -> the name of the field in the log file
		- compute     (dic)   -> The dictionary of the metric that derives from the observed ones
		                         key   (str)   -> metric name
		                         value (str)   -> the formula that should be used to compute the metric
	"""


	def __init__(self, xml_path):
		"""
		Parse the XML file and extracts the required information
		"""

		# open the XML file
		xml_root, namespace = p.parse_xml_file(xml_path)

		# initialize application-wide configurations
		self.block = p.get_parameter(xml_root, 'block')
		self.name = p.get_parameter(xml_root, 'name', required = False)
		if not self.name:
			self.name = 'generic-app'
		self.log_file = p.get_parameter(xml_root, 'out_file', required = False)
		if not self.log_file:
			self.log_file = '{0}.log'.format(self.block)

		# get all the flags parameters
		self.flags = []
		flags_xml = p.get_elements(xml_root, 'flag', namespace = namespace)
		for flag_xml in flags_xml:
			flag_value = p.get_parameter(flag_xml, 'value')
			self.flags.append(flag_value)

		# get all the knob description
		self.knob_values = {}
		self.knob_flags = {}
		knobs_xml = p.get_elements(xml_root, 'parameter', namespace = namespace, required = True)
		for knob_xml in knobs_xml:
			knob_values = []
			knob_type = p.get_parameter(knob_xml, 'type', prefixed_values = ['int', 'float', 'enum'])
			knob_name = p.get_parameter(knob_xml, 'name')
			if knob_type == 'int' or knob_type == 'float':
				if knob_type == 'float':
					actual_type = float
				else:
					actual_type = int
				start_value = p.get_parameter(knob_xml, 'start_value', my_target_type = actual_type)
				stop_value = p.get_parameter(knob_xml, 'stop_value', my_target_type = actual_type)
				step_value = p.get_parameter(knob_xml, 'step', my_target_type = actual_type)
				index = start_value
				while( index <= stop_value):
					knob_values.append(str(index))
					index += step_value
			else:
				if knob_type != 'enum':
					print('[DEFENSIVE ERROR] This should not happens! humm... 3324231412')
					sys.exit(-1)
				values_str = p.get_parameter(knob_xml, 'values')
				knob_values = [ x for x in values_str.split(' ') if x]
			self.knob_values[knob_name] = knob_values
			self.knob_flags[knob_name] = p.get_parameter(knob_xml, 'flag')

		# get all the observed metric description
		self.metrics = {}
		metrics_xml = p.get_elements(xml_root, 'metric', namespace = namespace)
		for metric_xml in metrics_xml:
			metric_name = p.get_parameter(metric_xml, 'name')
			metric_field = p.get_parameter(metric_xml, 'field')
			self.metrics[metric_name] = metric_field

		# get all the metric description that should be computed
		self.compute = {}
		computed_metrics_xml = p.get_elements(xml_root, 'compute', namespace = namespace)
		for computed_metric_xml in computed_metrics_xml:
			metric_name = p.get_parameter(computed_metric_xml, 'name')
			metric_field = p.get_parameter(computed_metric_xml, 'formula')
			self.compute[metric_name] = metric_field



	def __str__(self):
		string = '***** {0} *****\n'.format(self.name.upper())
		string = '{0}\tBlock:    "{1}"\n'.format(string, self.block)
		string = '{0}\tLog file: "{1}"\n'.format(string, self.log_file)
		string = '{0}\tFlags:    "{1}"\n'.format(string, '", "'.join(self.flags))
		string = '{0}\tKnobs:\n'.format(string)
		for knob_name in self.knob_values:
			values = '"{0}"'.format('", "'.join(self.knob_values[knob_name]))
			string = '{0}\t\t{1} ("{3}") : {2}\n'.format(string, knob_name, values, self.knob_flags[knob_name])
		string = '{0}\tObserved Metrics:\n'.format(string)
		for metric_name in self.metrics:
			string = '{0}\t\t{1} <- "{2}"\n'.format(string, metric_name, self.metrics[metric_name])
		string = '{0}\tComputed Metrics:\n'.format(string)
		for metric_name in self.compute:
			string = '{0}\t\t{1} <- "{2}"\n'.format(string, metric_name, self.compute[metric_name])
		return string



	def get_translator_dictionaries(self):
		knob_names = sorted(self.knob_values.keys())
		translator_eval_string = 'translator = {'
		reverse_translator_eval_string = 'reverse_translator = {'
		terms = []
		terms_reverse = []
		for knob_name in knob_names:
			numeric = True
			for knob_value in self.knob_values[knob_name]:
				try:
					float(knob_value)
				except:
					numeric = False
					break
			if not numeric:
				sorted_values = sorted(self.knob_values[knob_name])
				for counter, value in enumerate(sorted_values):
					terms.append('"{0}":{1}'.format(value, counter))
					terms_reverse.append('{0}:"{1}"'.format(counter, value))
		translator_eval_string = '{0}{1}}}'.format(translator_eval_string, ','.join(terms))
		reverse_translator_eval_string = '{0}{1}}}'.format(reverse_translator_eval_string, ','.join(terms_reverse))
		return translator_eval_string, reverse_translator_eval_string