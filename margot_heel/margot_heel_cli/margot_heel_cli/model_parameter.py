class ParameterModel:
	"""
	This class has the following attributes:
		- the name of the parameter (if a variable)
		- the type of the parameter (if a variable)
		- the value of the parameter (if a fixed value)
		- the name of the parameter
	"""

	def __init__(self):
		self.var_name = ''
		self.var_type = ''
		self.param_value = ''
		self.param_name = ''


	def create_from_local(self, var_name, var_type):
		temp_param_model = ParameterModel()
		temp_param_model.var_name = var_name
		temp_param_model.var_type = var_type
		return temp_param_model

	def create_from_fixed(self, param_value):
		temp_param_model = ParameterModel()
		param_value = str(param_value)
		try:
			temp_param_model.param_value = int(param_value)
		except ValueError:
			try:
				temp_param_model.param_value = float(param_value)
			except ValueError:
				temp_param_model.param_value = param_value
		return temp_param_model

	def __str__(self):

		if var_name == '' and var_type == '':
			return 'Fixed Parameter -> name = "{0}"  value = "{1}"'.format(self.param_name, self.param_value)

		if param_value == '':
			return 'Local Parameter -> name = "{0}"  var_name = "{1}"  var_type = "{2}"'.format(self.param_name, self.var_name, self.var_type)

		return 'Unkown Parameter -> name = "{0}"'.format(self.param_name)
