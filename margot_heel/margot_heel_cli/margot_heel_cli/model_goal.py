class GoalModel:
	"""
	This class represents a goal in the configuration file.
	In particular a goal is defined by the following values:
		- the type of the goal (static/dynamic)
		- the name of the goal
		- the related monitor (if dynamic)
		- the related metric (if static and targets a metric )
		- the related parameter (if static and targets a parameter )
		- the data function (if synamic)
		- the comparison function
		- the actual value of the goal
	"""


	def __init__(self):
		# generic goal boundaries
		self.available_types = ['STATIC', 'DYNAMIC']
		self.available_dfuns = ['AVARAGE', 'VARIANCE', 'MAX', 'MIN']
		self.available_cfuns = ['GT', 'GE', 'LT', 'LE']

		# goal parameters
		self.goal_type = 'STATIC'
		self.name = 'a_generic_goal'
		self.monitor_name_ref = ''
		self.dfun = ''
		self.cfun = ''
		self.metric_name_ref = ''
		self.parameter_name_ref = ''
		self.value = '0'
		self.stored_type_value = ''


	def __str__(self):
		"""
		dump the goal model
		"""
		dump_string = '\n'

		if self.goal_type == 'DYNAMIC':
			dump_string = '{0}\n  Dynamic goal specification'.format(dump_string)
			dump_string = '{0}\n    Name:           {1}'.format(dump_string, self.name)
			dump_string = '{0}\n    Monitor ref:    {1}'.format(dump_string, self.monitor_name_ref)
			dump_string = '{0}\n    Data fun:       {1}'.format(dump_string, self.dfun)
			dump_string = '{0}\n    Comparison fun: {1}'.format(dump_string, self.cfun)
			dump_string = '{0}\n    Target value:   {1}'.format(dump_string, self.value)
		if self.goal_type == 'STATIC':
			dump_string = '{0}\n  Static goal specification'.format(dump_string)
			dump_string = '{0}\n    Name:            {1}'.format(dump_string, self.name)
			if self.metric_name_ref:
				dump_string = '{0}\n    Metric field:    {1}'.format(dump_string, self.metric_name_ref)
				dump_string = '{0}\n    Comparison fun:  {1}'.format(dump_string, self.cfun)
				dump_string = '{0}\n    Target value:    {1}'.format(dump_string, self.value)
			if self.parameter_name_ref:
				dump_string = '{0}\n    Parameter field: {1}'.format(dump_string, self.parameter_name_ref)
				dump_string = '{0}\n    Comparison fun:  {1}'.format(dump_string, self.cfun)
				dump_string = '{0}\n    Target value:    {1}'.format(dump_string, self.value)
		return dump_string
