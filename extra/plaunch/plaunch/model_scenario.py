import os
import shutil
import time
import errno

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc:
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise



class ScenarioModel:
	"""
	This class represents the scenario to be executed
	It includes the following information
		- the list of the applications
		- experiment root folder
		- execution root folder
	"""

	def __init__(self):
		"""
		Initialize the scenario model
		"""
		self.applications = [] # the list of applications
		self.experiment_path = ""
		self.execution_path = ""


	def setup(self):
		"""
		Prepare the execution of the experiment
		"""

		# the experiment base directory is the working directory
		base_path = os.getcwd()

		# creates the result folder
		result_path = os.path.join(base_path, 'result')
		mkdir_p(result_path)

		# append the subfolder with the current date
		self.experiment_path = os.path.join(result_path, time.strftime("%Y.%m.%d-%H.%M.%S"))
		mkdir_p(self.experiment_path)

		# create the execution folder
		self.execution_path = os.path.join(self.experiment_path, 'exec')
		mkdir_p(self.execution_path)

		# setup the path for each application
		for application in self.applications:

			# set the application paths
			application.execution_folder = self.execution_path
			application.output_folder = self.experiment_path

			# perform the setup
			application.setup()



	def play(self):
		"""
		Actually execute the scenario
		"""

		# start the applications
		for application in self.applications:
			application.start()

		# wait until they are finished
		for application in self.applications:
			application.join()


	def teardown(self):
		"""
		Commit the results and delete the executions files
		"""

		# teardown each single application
		for application in self.applications:
			application.commit()
			application.teardown()

		# teardown the whole execution folder
		shutil.rmtree(self.execution_path)

		# dump the start/stop times
		with open(os.path.join(self.experiment_path, 'experiment.log'), 'w') as cc:

			# write the header
			cc.write('Application Start Stop\n')

			# populate the table
			for application in self.applications:
				cc.write('{0} {1} {2}\n'.format(application.name, application.start_time, application.stop_time))
