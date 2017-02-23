class Configuration:
	"""
	This class represents a configuration that should be profiled at runtime

	Attributes:
		- name        (str)  -> A readable name of the configuration
		- executable  (str)  -> The path of the related executable
		- flags       (list) -> The list of the flags to run this configuration
		- cwd         (str)  -> The configuration working directory
		- log_file    (str)  -> The name of the generated log file
		- description (dic)  -> The description of the configuration in plain
		                        key   -> (str) the name of the knob
		                        value -> (str) the value of the knob
	"""

	def __init__(self):
		self.name = ''
		self.executable = ''
		self.flags = ''
		self.cwd = ''
		self.log_file = ''


	def __str__(self):
		string = 'Configuration "{0}"\n'.format(self.name)
		string = '{0}\t-- flags: "{1}"\n'.format(string, ' '.join(self.flags))
		string = '{0}\t-- executable: "{1}"\n'.format(string, self.executable)
		string = '{0}\t-- cwd: "{1}"\n'.format(string, self.cwd)
		string = '{0}\t-- log_file: "{1}"\n'.format(string, self.log_file)
		return string
