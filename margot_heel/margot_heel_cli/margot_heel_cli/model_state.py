import sys

class StateModel:
	"""
	This class represents a state, which is an optimization problem.
	In particular, it is composed by the following information:
		- the name of the state
		- the rank definition
		- the list of constraints
		- if this is the starting state
	"""

	def __init__(self):
		self.rank_available_types = ['GEOMETRIC', 'LINEAR']
		self.rank_available_directions = ['maximize', 'minimize']
		self.rank_available_components = ['knob', 'metric'] # if you change that, you should change the parser as well
		self.name = ""
		self.rank_element_param = {}  # key -> name of the parameter, value -> coefficient
		self.rank_element_metric = {} # similar to the previous one
		self.rank_type = 'GEOMETRIC'
		self.rank_direction = 'minimize'
		self.starting = True
		self.constraint_list = []


	def set_rank_direction(self, direction):
		"""
		check to assign the correct direction to the rank
		"""
		if direction == 'maximize':
			self.rank_direction = 'maximize'
			return

		if direction == 'minimize':
			self.rank_direction = 'minimize'
			return

		self.rank_direction = 'unknown (unreachable statement)'


	def __str__(self):
		"""
		Dump the content of the class
		"""

		dump_string = ''

		dump_string = '{0}\n\n  State specification'.format(dump_string)
		dump_string = '{0}\n    Name: {1}'.format(dump_string, self.name)

		# print the rank
		rank_string = '{0}'.format(self.rank_direction)
		for param_name in self.rank_element_param:
			if self.rank_type == 'GEOMETRIC':
				rank_string = '{0} ({1}^{2}) *'.format(rank_string, param_name, self.rank_element_param[param_name])
			if self.rank_type == 'LINEAR':
				rank_string = '{0} {1}*{2} +'.format(rank_string, self.rank_element_param[param_name], param_name)
		for metric_name in self.rank_element_metric:
			if self.rank_type == 'GEOMETRIC':
				rank_string = '{0} ({1}^{2}) *'.format(rank_string, metric_name, self.rank_element_metric[metric_name])
			if self.rank_type == 'LINEAR':
				rank_string = '{0} {1}*{2} +'.format(rank_string, self.rank_element_metric[metric_name], metric_name)
		rank_string = rank_string[:-1]
		dump_string = '{0}\n    {1}'.format(dump_string, rank_string)
		dump_string = '{0}\n    subject to:'.format(dump_string)

		for index, constraint in enumerate(sorted(self.constraint_list, key=lambda x: int(x.priority))) :
			dump_string = '{0}\n      c{1}: "{2}"'.format(dump_string, index + 1, constraint.goal_ref)
			if constraint.target_metric:
				dump_string = '{0} on metric "{1}"'.format(dump_string, constraint.target_metric)
			if constraint.target_knob:
				dump_string = '{0} on knob "{1}"'.format(dump_string, constraint.target_knob)

		if self.starting:
			dump_string = '{0}\n    This is the starting state!'.format(dump_string)


		return dump_string
