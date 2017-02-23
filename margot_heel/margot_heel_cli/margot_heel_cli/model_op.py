class OperatingPointModel:
	"""
	This class represents an Operating Point.
	It includes the following information:
		- the map that describes the software knobs
		- the map that describes the metrics
	"""

	def __init__(self):
		self.metrics = {} # key -> metric_name, value -> metric_value
		self.knobs = {}   # key -> knob_name, value -> metric_value

	def __str__(self):
		string = '*** POINT ***\n'
		string = '{0}\tParameters:\n'.format(string)
		for knob_name in self.knobs:
			string = '{0}\t\t{1} = {2}\n'.format(string, knob_name, self.knobs[knob_name])
		string = '{0}\tMetrics:\n'.format(string)
		for metric_name in self.metrics:
			string = '{0}\t\t{1} = {2}\n'.format(string, metric_name, self.metrics[metric_name])
		return string


	def similar(self, other_op):
		"""
		Return true if other_op has the same fields wrt self
		"""

		my_metrics = self.metrics.keys()
		my_knobs = self.knobs.keys()

		other_metrics = other_op.metrics.keys()
		other_knobs = other_op.knobs.keys()

		return (my_metrics == other_metrics) and (my_knobs == other_knobs)

	def add(self, other_op):
		"""
		add the metrics of the two ops. have to be similar
		"""
		if self.similar(other_op):
			for key in self.metrics.keys():
				self.metrics[key]=float(other_op.metrics[key])+float(self.metrics[key])

		else:
			print ("attempt to add metrics of non similar ops: aborting")
			sys.exit (-1)

	def avg(self, num_values, key):
		"""
		divides the metric identified by key for the num_values.
		"""
		if key in self.metrics.keys():
			self.metrics[key]=float(self.metrics[key])/int(num_values)
