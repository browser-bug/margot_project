from . import model_parameter
import sys




class MonitorModel:
	"""
	This class represents a monitor, in particular it defines:
		- the monitor class
		- the monitor header
		- the monitor type
		- the monitor name
		- the monitor stop method name
		- the monitor start method name
		- the list of creation parameters
		- the list of [required] start parameters
		- the list of [required] stop parameters
	"""


	def __init__(self):
		# The available output metrics
		self.output_available_metrics = ['AVERAGE', 'VARIANCE', 'MAX', 'MIN']


		# The monitor specs
		self.monitor_name = 'my_default_monitor'
		self.monitor_class = 'margot::monitor_t'
		self.monitor_header = '<margot/monitor.hpp>'
		self.monitor_type = 'int'
		self.stop_method = ''
		self.start_method = ''
		self.required_start_methods = [] # the name of the required parameters
		self.required_creation_methods = []
		self.required_stop_methods = []

		# The input parameters
		self.creation_parameters = [] # the list of the parsed parameters
		self.start_parameters = []
		self.stop_parameters = []

		# The output parameters key -> what, value -> var name
		self.exposed_metrics = {}


	def check_consistency(self):
		"""
		This method checks if all the parameters required are defined
		"""
		for pr in self.required_start_methods:
			found = False
			for p in self.start_parameters:
				if p.param_name == pr.param_name:
					found = True
					break
			if not found:
				print('[CONSISTENCY ERROR] The "{0}" monitor with name "{1}" has an incomplete definition:'.format(self.monitor_class, self.monitor_name))
				print('                    Unable to to find the required input parameter "{0}" '.format(pr.param_name))
				print('                    Defined start parameters -> ["{0}"]'.format('", "'.join([a.param_name for a in self.start_parameters])))
				sys.exit(-1)




	def __str__(self):
		"""
		dump the monitor model
		"""
		dump_string = '\n'

		dump_string = '{0}\n  Monitor specification'.format(dump_string)
		dump_string = '{0}\n    Name:   {1}'.format(dump_string, self.monitor_name)
		dump_string = '{0}\n    Class:  {1}'.format(dump_string, self.monitor_class)
		dump_string = '{0}\n    Header: {1}'.format(dump_string, self.monitor_header)
		dump_string = '{0}\n    Type:   {1}'.format(dump_string, self.monitor_type)

		dump_string = '{0}\n  Monitor usage (pseudocode)'.format(dump_string)
		dump_string = '{0}\n    Constructor:  {1}({2})'.format(dump_string, self.monitor_class, ','.join([p.param_name for p in self.creation_parameters]))
		if self.start_method:
			dump_string = '{0}\n    Start method: {1}({2})'.format(dump_string, self.start_method, ','.join([p.param_name for p in self.start_parameters]))
		else:
			dump_string = '{0}\n    Start method: Not defined'.format(dump_string)

		if self.stop_method:
			dump_string = '{0}\n    Stop method:  {1}({2})'.format(dump_string, self.stop_method, ','.join([p.param_name for p in self.stop_parameters]))
		else:
			dump_string = '{0}\n    Stop method:  Not defined'.format(dump_string)

		dump_string = '{0}\n  Exposed variables'.format(dump_string)
		for exposed_var in self.exposed_metrics:
			dump_string = '{0}\n    "{1}" in a variable with name "{2}"'.format(dump_string, exposed_var.upper(), self.exposed_metrics[exposed_var])

		return dump_string







know_monitor_spec = [
	'CUSTOM',
	'FREQUENCY',
	'MEMORY',
	'PAPI',
	'CPUPROCESS',
	'CPUSYSTEM',
	'TEMPERATURE',
	'THROUGHPUT',
	'TIME',
	'ENERGY',
	'ODROID_POWER',
	'ODROID_ENERGY',
	'COLLECTOR'
]


def get_monitor_spec( monitor_type ):
	"""
	Provides the list of known spec
	"""

	if monitor_type.upper() == 'FREQUENCY':
		# create the model
		my_monitor = MonitorModel()

		# set the spec
		my_monitor.monitor_class = 'margot::frequency_monitor_t'
		my_monitor.monitor_header = '<margot/frequency_monitor.hpp>'
		my_monitor.monitor_type = 'margot::frequency_monitor_t::value_type'
		my_monitor.stop_method = 'measure'
		my_monitor.start_method = ''

		return my_monitor

	if monitor_type.upper() == 'MEMORY':
		# create the model
		my_monitor = MonitorModel()

		# set the spec
		my_monitor.monitor_class = 'margot::memory_monitor_t'
		my_monitor.monitor_header = '<margot/memory_monitor.hpp>'
		my_monitor.monitor_type = 'margot::memory_monitor_t::value_type'
		my_monitor.stop_method = 'extractMemoryUsage'
		my_monitor.start_method = ''

		return my_monitor

	if monitor_type.upper() == 'PAPI':
		# create the model
		my_monitor = MonitorModel()

		# set the spec
		my_monitor.monitor_class = 'margot::papi_monitor_t'
		my_monitor.monitor_header = '<margot/papi_monitor.hpp>'
		my_monitor.monitor_type = 'margot::papi_monitor_t::value_type'
		my_monitor.stop_method = 'stop'
		my_monitor.start_method = 'start'
		my_monitor.required_creation_methods = ['event']

		return my_monitor

	if monitor_type.upper() == 'CPUPROCESS':
		# create the model
		my_monitor = MonitorModel()

		# set the spec
		my_monitor.monitor_class = 'margot::process_cpu_usage_monitor_t'
		my_monitor.monitor_header = '<margot/process_cpu_usage_monitor.hpp>'
		my_monitor.monitor_type = 'margot::process_cpu_usage_monitor_t::value_type'
		my_monitor.stop_method = 'stop'
		my_monitor.start_method = 'start'

		return my_monitor

	if monitor_type.upper() == 'CPUSYSTEM':
		# create the model
		my_monitor = MonitorModel()

		# set the spec
		my_monitor.monitor_class = 'margot::system_cpu_usage_monitor_t'
		my_monitor.monitor_header = '<margot/system_cpu_usage_monitor.hpp>'
		my_monitor.monitor_type = 'margot::system_cpu_usage_monitor_t::value_type'
		my_monitor.stop_method = 'stop'
		my_monitor.start_method = 'start'

		return my_monitor

	if monitor_type.upper() == 'TEMPERATURE':
		# create the model
		my_monitor = MonitorModel()

		# set the spec
		my_monitor.monitor_class = 'margot::temperature_monitor_t'
		my_monitor.monitor_header = '<margot/temperature_monitor.hpp>'
		my_monitor.monitor_type = 'margot::temperature_monitor_t::value_type'
		my_monitor.stop_method = 'measure'
		my_monitor.start_method = ''

		return my_monitor

	if monitor_type.upper() == 'THROUGHPUT':
		# create the model
		my_monitor = MonitorModel()

		# set the spec
		my_monitor.monitor_class = 'margot::throughput_monitor_t'
		my_monitor.monitor_header = '<margot/throughput_monitor.hpp>'
		my_monitor.monitor_type = 'margot::throughput_monitor_t::value_type'
		my_monitor.stop_method = 'stop'
		my_monitor.start_method = 'start'
		my_monitor.required_stop_methods = ['num_data_elaborated']

		return my_monitor

	if monitor_type.upper() == 'TIME':
		# create the model
		my_monitor = MonitorModel()

		# set the spec
		my_monitor.monitor_class = 'margot::time_monitor_t'
		my_monitor.monitor_header = '<margot/time_monitor.hpp>'
		my_monitor.monitor_type = 'margot::time_monitor_t::value_type'
		my_monitor.stop_method = 'stop'
		my_monitor.start_method = 'start'

		return my_monitor

	if monitor_type.upper() == 'ENERGY':
		# create the model
		my_monitor = MonitorModel()

		# set the spec
		my_monitor.monitor_class = 'margot::energy_monitor_t'
		my_monitor.monitor_header = '<margot/energy_monitor.hpp>'
		my_monitor.monitor_type = 'margot::energy_monitor_t::value_type'
		my_monitor.stop_method = 'stop'
		my_monitor.start_method = 'start'

		return my_monitor

	if monitor_type.upper() == 'COLLECTOR':
		# create the model
		my_monitor = MonitorModel()

		# set the spec
		my_monitor.monitor_class = 'margot::collector_monitor_t'
		my_monitor.monitor_header = '<margot/collector_monitor.hpp>'
		my_monitor.monitor_type = 'margot::collector_monitor_t::value_type'
		my_monitor.stop_method = 'stop'
		my_monitor.start_method = 'start'

		return my_monitor

	if monitor_type.upper() == 'ODROID_POWER':
		# create the model
		my_monitor = MonitorModel()

		# set the spec
		my_monitor.monitor_class = 'margot::odroid_power_monitor_t'
		my_monitor.monitor_header = '<margot/odroid_power_monitor.hpp>'
		my_monitor.monitor_type = 'margot::odroid_power_monitor_t::value_type'
		my_monitor.stop_method = 'stop'
		my_monitor.start_method = 'start'

		return my_monitor

	if monitor_type.upper() == 'ODROID_ENERGY':
		# create the model
		my_monitor = MonitorModel()

		# set the spec
		my_monitor.monitor_class = 'margot::odroid_energy_monitor_t'
		my_monitor.monitor_header = '<margot/odroid_energy_monitor.hpp>'
		my_monitor.monitor_type = 'margot::odroid_energy_monitor_t::value_type'
		my_monitor.stop_method = 'stop'
		my_monitor.start_method = 'start'

		return my_monitor


	# no known spec found
	return None
