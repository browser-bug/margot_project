class LearningKnobModel:
	"""
	This class represents a software knob of the application that should be learned at run-time, and it inclues:
		- the type of the variable in the application
		- the name of the variable in the application
		- the name of the parameter
		- the coefficient for the rank of the param
		- the available values of the software knob
	"""

	def __init__(self):
		self.var_name = ""
		self.var_type = ""
		self.name = ""
		self.coefficient = ""
		self.values = []


	def __str__(self):
		"""
		Dump the content of the class
		"""

		dump_string = ''

		dump_string = '{0}\n\n  Learning knob specification'.format(dump_string)
		dump_string = '{0}\n    Name:          {1}'.format(dump_string, self.name)
		dump_string = '{0}\n    Variable name: {1}'.format(dump_string, self.var_name)
		dump_string = '{0}\n    Variable type: {1}'.format(dump_string, self.var_type)
		dump_string = '{0}\n    Coefficient:   {1}'.format(dump_string, self.coefficient)
		dump_string = '{0}\n    Values:        [{1}]'.format(dump_string, ', '.join(self.values))

		return dump_string
