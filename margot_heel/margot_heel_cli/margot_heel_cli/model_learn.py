class LearningModel:
	"""
	This class represents a collection of software knobs that should be learned online, it includes:
		- the type of the framework to learn dynamically the best policy
		- the coefficient sigma for the learning process
		- the size of the window
		- the balance coefficient between the state reward and the arm rank
		- the list of all the software-knob that should be learned
	"""

	available_framework = ['SW_UCB']

	def __init__(self):
		self.framework_type = ""
		self.sigma = ""
		self.window_size = ""
		self.balance_coef = ""
		self.knobs = []


	def get_learned_knob_list(self):
		"""
		Return a sorted list of the learned software_knobs
		"""
		result = []
		for knob_model in self.knobs:
			result.append(knob_model.name)
		return result


	def __str__(self):
		"""
		Dump the content of the class
		"""

		dump_string = ''

		dump_string = '{0}\n\n  Learning framework specification'.format(dump_string)
		dump_string = '{0}\n    Type:          {1}'.format(dump_string, self.framework_type)
		dump_string = '{0}\n    Sigma value:   {1}'.format(dump_string, self.sigma)
		dump_string = '{0}\n    Window size:   {1}'.format(dump_string, self.window_size)
		dump_string = '{0}\n    Balance coeff: {1}'.format(dump_string, self.balance_coef)
		dump_string = '{0}\n    Num knobs:     {1}'.format(dump_string, len(self.knobs))
		for knob in self.knobs:
			dump_string = '{0}\n      {1}: [{2}] -> rank coef: {3}'.format(dump_string, knob.name, ', '.join(knob.values), knob.coefficient)

		return dump_string
