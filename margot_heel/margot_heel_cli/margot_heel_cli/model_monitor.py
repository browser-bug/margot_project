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
    self.output_available_metrics = ['AVERAGE', 'STDDEV', 'MAX', 'MIN']


    # The monitor specs
    self.monitor_name = 'my_default_monitor'
    self.monitor_class = 'margot::Monitor<float>'
    self.monitor_header = '<margot/monitor.hpp>'
    self.monitor_type = 'float'
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

    # Error monitor specific parameters
    self.frequency = ''
    self.period = ''


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

    if self.frequency:
      dump_string = '{0}\n    Error monitor frequency: {1}'.format(dump_string, self.frequency)
    if self.period:
      dump_string = '{0}\n    Error monitor period: {1}'.format(dump_string, self.period)

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
  'COLLECTOR',
  'ERROR'
]

available_frequencies = ['never', 'always', 'periodic', 'auto']


def get_monitor_spec( monitor_type ):
  """
  Provides the list of known spec
  """

  if monitor_type.upper() == 'FREQUENCY':
    # create the model
    my_monitor = MonitorModel()

    # set the spec
    my_monitor.monitor_class = 'margot::FrequencyMonitor'
    my_monitor.monitor_header = '<margot/frequency_monitor.hpp>'
    my_monitor.monitor_type = 'margot::FrequencyMonitor::value_type'
    my_monitor.stop_method = 'measure'
    my_monitor.start_method = ''

    return my_monitor

  if monitor_type.upper() == 'MEMORY':
    # create the model
    my_monitor = MonitorModel()

    # set the spec
    my_monitor.monitor_class = 'margot::MemoryMonitor'
    my_monitor.monitor_header = '<margot/memory_monitor.hpp>'
    my_monitor.monitor_type = 'margot::MemoryMonitor::value_type'
    my_monitor.stop_method = 'extractMemoryUsage'
    my_monitor.start_method = ''

    return my_monitor

  if monitor_type.upper() == 'PAPI':
    # create the model
    my_monitor = MonitorModel()

    # set the spec
    my_monitor.monitor_class = 'margot::PapiMonitor'
    my_monitor.monitor_header = '<margot/papi_monitor.hpp>'
    my_monitor.monitor_type = 'margot::PapiMonitor::value_type'
    my_monitor.stop_method = 'stop'
    my_monitor.start_method = 'start'
    my_monitor.required_creation_methods = ['event']

    return my_monitor

  if monitor_type.upper() == 'CPUPROCESS':
    # create the model
    my_monitor = MonitorModel()

    # set the spec
    my_monitor.monitor_class = 'margot::ProcessCpuMonitor'
    my_monitor.monitor_header = '<margot/process_cpu_usage_monitor.hpp>'
    my_monitor.monitor_type = 'margot::ProcessCpuMonitor::value_type'
    my_monitor.stop_method = 'stop'
    my_monitor.start_method = 'start'

    return my_monitor

  if monitor_type.upper() == 'CPUSYSTEM':
    # create the model
    my_monitor = MonitorModel()

    # set the spec
    my_monitor.monitor_class = 'margot::SystemCpuMonitor'
    my_monitor.monitor_header = '<margot/system_cpu_usage_monitor.hpp>'
    my_monitor.monitor_type = 'margot::SystemCpuMonitor::value_type'
    my_monitor.stop_method = 'stop'
    my_monitor.start_method = 'start'

    return my_monitor

  if monitor_type.upper() == 'TEMPERATURE':
    # create the model
    my_monitor = MonitorModel()

    # set the spec
    my_monitor.monitor_class = 'margot::TemperatureMonitor'
    my_monitor.monitor_header = '<margot/temperature_monitor.hpp>'
    my_monitor.monitor_type = 'margot::TemperatureMonitor::value_type'
    my_monitor.stop_method = 'measure'
    my_monitor.start_method = ''

    return my_monitor

  if monitor_type.upper() == 'THROUGHPUT':
    # create the model
    my_monitor = MonitorModel()

    # set the spec
    my_monitor.monitor_class = 'margot::ThroughputMonitor'
    my_monitor.monitor_header = '<margot/throughput_monitor.hpp>'
    my_monitor.monitor_type = 'margot::ThroughputMonitor::value_type'
    my_monitor.stop_method = 'stop'
    my_monitor.start_method = 'start'
    my_monitor.required_stop_methods = ['num_data_elaborated']

    return my_monitor

  if monitor_type.upper() == 'TIME':
    # create the model
    my_monitor = MonitorModel()

    # set the spec
    my_monitor.monitor_class = 'margot::TimeMonitor'
    my_monitor.monitor_header = '<margot/time_monitor.hpp>'
    my_monitor.monitor_type = 'margot::TimeMonitor::value_type'
    my_monitor.stop_method = 'stop'
    my_monitor.start_method = 'start'

    return my_monitor

  if monitor_type.upper() == 'ENERGY':
    # create the model
    my_monitor = MonitorModel()

    # set the spec
    my_monitor.monitor_class = 'margot::EnergyMonitor'
    my_monitor.monitor_header = '<margot/energy_monitor.hpp>'
    my_monitor.monitor_type = 'margot::EnergyMonitor::value_type'
    my_monitor.stop_method = 'stop'
    my_monitor.start_method = 'start'

    return my_monitor

  if monitor_type.upper() == 'COLLECTOR':
    # create the model
    my_monitor = MonitorModel()

    # set the spec
    my_monitor.monitor_class = 'margot::CollectorMonitor'
    my_monitor.monitor_header = '<margot/collector_monitor.hpp>'
    my_monitor.monitor_type = 'margot::CollectorMonitor::value_type'
    my_monitor.stop_method = 'stop'
    my_monitor.start_method = 'start'

    return my_monitor

  if monitor_type.upper() == 'ODROID_POWER':
    # create the model
    my_monitor = MonitorModel()

    # set the spec
    my_monitor.monitor_class = 'margot::OdroidPowerMonitor'
    my_monitor.monitor_header = '<margot/odroid_power_monitor.hpp>'
    my_monitor.monitor_type = 'margot::OdroidPowerMonitor::value_type'
    my_monitor.stop_method = 'stop'
    my_monitor.start_method = 'start'

    return my_monitor

  if monitor_type.upper() == 'ODROID_ENERGY':
    # create the model
    my_monitor = MonitorModel()

    # set the spec
    my_monitor.monitor_class = 'margot::OdroidEnergyMonitor'
    my_monitor.monitor_header = '<margot/odroid_energy_monitor.hpp>'
    my_monitor.monitor_type = 'margot::OdroidEnergyMonitor::value_type'
    my_monitor.stop_method = 'stop'
    my_monitor.start_method = 'start'

    return my_monitor

  if monitor_type.upper() == 'ERROR':
    # create the model
    my_monitor = MonitorModel()

    # set the spec
    my_monitor.monitor_class = 'margot::ErrorMonitor'
    my_monitor.monitor_header = '<margot/error_monitor.hpp>'
    my_monitor.stop_method = 'stop'


    return my_monitor


  # no known spec found
  return None
