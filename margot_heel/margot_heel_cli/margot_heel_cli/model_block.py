import sys

class BlockModel:
	"""
	This class represents the model of a tunable block of code,
	in particular it includes:
		- the list of monitor models
		- the list of goal models
		- the list of Operating Points model
		- the list of manager model
		"""

	"""
	The name of the modeled block
	"""
	block_name = "no_name"



	"""
	The list of models included in the block
	"""
	monitor_models = []
	goal_models = []
	software_knobs = []
	state_models = []
	configure_function = ''
	learn_models = []

	def __init__(self):
		self.block_name = "no_name"
		self.monitor_models = []
		self.goal_models = []
		self.software_knobs = []
		self.state_models = []
		self.configure_function = ''
		self.learn_models = []


	def get_learned_software_knobs(self):
		result = []
		if self.learn_models:
			result = self.learn_models[0].get_learned_knob_list()
		return result



	def __init__(self, block_name):
		self.block_name = block_name
		self.monitor_models = []
		self.goal_models = []
		self.software_knobs = []
		self.state_models = []
		self.configure_function = ''
		self.learn_models = []



	def __str__(self):
		"""
		Dump this model as collection of others models
		"""

		dump_string = ''

		dump_string = '{0}\n\n######### Block specification #########'.format(dump_string)
		dump_string = '{0}\n##  Name: {1}'.format(dump_string, self.block_name)
		dump_string = '{0}\n#######################################'.format(dump_string)

		dump_string2 = '\n\n-----------  MONITORS -----------'
		for monitor_model in self.monitor_models:
			dump_string2 = '{0}\n{1}'.format(dump_string2, monitor_model)

		dump_string2 = '{0}\n\n-------------  GOALS ------------'.format(dump_string2)
		for goal_model in self.goal_models:
			dump_string2 = '{0}\n{1}'.format(dump_string2, goal_model)

		dump_string2 = '{0}\n\n--------  SOFTWARE KNOBS --------'.format(dump_string2)
		for knob_model in self.software_knobs:
			dump_string2 = '{0}\n{1}'.format(dump_string2, knob_model)

		dump_string2 = '{0}\n\n--------  LEARNING KNOBS --------'.format(dump_string2)
		for learn_model in self.learn_models:
			dump_string2 = '{0}\n{1}'.format(dump_string2, learn_model)

		dump_string2 = '{0}\n\n------------  STATES ------------'.format(dump_string2)
		for state_model in self.state_models:
			dump_string2 = '{0}\n{1}'.format(dump_string2, state_model)
		dump_string2 = '{0}\n'.format(dump_string2)

		dump_string2 = dump_string2.replace('\n', '\n# ')
		dump_string = '{0}{1}\n#######################################'.format(dump_string,dump_string2)
		dump_string = '{0}\n#######################################\n\n'.format(dump_string)

		return dump_string


	def postprocess(self):
		"""
		This method is meant to be used to perform the following operations:
			- propagate the name of the field in a static goal toward the state
			- check if all the dynamic goals refer to a monitor
			- check if all the static goals refer to a knob
		"""

		# ----- propagate the target field for a static constraint

		# loop over the states
		for state_model in self.state_models:

			# loop over the constraints
			for constraint_model in state_model.constraint_list:

				# get the goal referene
				goal_ref = constraint_model.goal_ref

				# loop over the goals
				for goal_model in self.goal_models:

					# check if it is the target goal
					if goal_model.name == goal_ref:

						# check if the goal is static
						if goal_model.goal_type == 'STATIC':

							# propagate the target field
							if goal_model.metric_name_ref:
								constraint_model.target_metric = goal_model.metric_name_ref
							if goal_model.parameter_name_ref:
								constraint_model.target_knob = goal_model.parameter_name_ref
							break

		# ----- check the goals

		# loop over the goals
		for goal_model in self.goal_models:

			# check if it is dynamic goal
			if goal_model.goal_type == 'DYNAMIC':

				# get the name of the reference monitor
				ref_monitor = goal_model.monitor_name_ref
				found = False

				# loop over the monitors
				for monitor_model in self.monitor_models:

					# check if it is the target monitor and set the type
					if monitor_model.monitor_name == ref_monitor:
						found = True
						goal_model.stored_type_value = monitor_model.monitor_type
						break

				# check if we found a monitor
				if not found:
					print('[CONSISTENCY ERROR] The dynamic goal "{0}" has a reference to an unknown monitor "{1}"'.format(goal_model.name, ref_monitor))
					print('                    Available monitors: "{0}"'.format('", "'.join([x.monitor_name for x in self.monitor_models])))
					sys.exit(-1)

			# check if it is a static goal
			if goal_model.goal_type == 'STATIC':

				# check if the static goal is on a parameter
				knob_name = goal_model.parameter_name_ref
				if knob_name:

					# set the type of the goal
					goal_model.stored_type_value = 'margot::parameter_t'

					found = False

					# loop through the knobs
					for knob_model in self.software_knobs:
						if knob_model.name == knob_name:
							found = True
							break


					# check if we found a monitor
					if not found:
						print('[CONSISTENCY ERROR] The static goal "{0}" has a reference to an unknown knob "{1}"'.format(goal_model.name, knob_name))
						print('                    Available monitors: "{0}"'.format('", "'.join([x.name for x in self.software_knobs])))
						sys.exit(-1)

				else:

					# set the type of the goal
					goal_model.stored_type_value = 'margot::metric_t'
