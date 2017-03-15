from . import dse_configuration as configuration

class DoE:
	"""
	This class is meant to generate all the configuration that should be generated
	according to a particular strategy

	Attributes:
		- supported_does (list)        -> the name of the implemented doe (str)
		- plan           (list)        -> the list of the generated configurations
		- application    (Application) -> the Application model
	"""

	supported_does = ['full-factorial']


	def generate_configuration_from_values( self, current_configuration_values ):
		# get the whole list of the knobs
		whole_knob_names = sorted(self.application.knob_values.keys())

		# create a new configuration
		new_configuration = configuration.Configuration()

		# create the configuration name and flags
		name_terms = []
		flag_terms = []
		new_configuration.description = {}
		for knob_index, temp_knob_name in enumerate(whole_knob_names):
			new_configuration.knob_map[temp_knob_name] = current_configuration_values[knob_index]
			name_terms.extend([temp_knob_name, current_configuration_values[knob_index]])
			flag_terms.extend([self.application.knob_flags[temp_knob_name], current_configuration_values[knob_index]])
			new_configuration.description[temp_knob_name] = current_configuration_values[knob_index]
		new_configuration.name = '_'.join(name_terms)
		new_configuration.log_file = self.application.log_file
		new_configuration.flags = flag_terms

		# append the application-wide configurations
		new_configuration.flags.extend(self.application.flags[self.input_params_index])
		return new_configuration


	def full_factorial(self, knob_names, current_configuration_values):
		"""
		Compute a full-factorial DoE
		"""

		# if we are the last then build the configuration
		if not knob_names:
			self.plan.append(self.generate_configuration_from_values(current_configuration_values))
			return

		# take first key
		this_knob_name = knob_names[0]

		# create a duplicate of the knobs
		fewer_knobs = list(knob_names)

		# delete the current knob
		del fewer_knobs[0]


		# loop over the this knob values
		for knob_value in self.application.knob_values[this_knob_name]:

			# duplicate and add the value to the configuration
			this_conf = list(current_configuration_values)

			# append the current value
			this_conf.append(knob_value)

			# go deeper
			self.full_factorial(fewer_knobs, this_conf)



	def __init__(self, doe_strategy, application_model, input_params_index):
		"""
		This class generates the configurations that must be profiled
		in the Design Space Exploration according to a specific DoE
		"""

		# initialize the plan
		self.plan = []
		self.application = application_model
		self.input_params_index = input_params_index

		# compute the doe
		if doe_strategy == 'full-factorial':
			knob_names = sorted(application_model.knob_values.keys())
			self.full_factorial(knob_names, [])
