import os
import sys
import errno
import shutil
import inspect
from . import makefile_generator
from . import generate_ops


#################################
###### Utility functions
#################################

def mkdir_p(p):
	"""
	Helper function that emulate the command "mkdir -p"
		p -> the path to be created
	"""
	try:
		os.makedirs(p)
	except OSError as exc: # Python >2.5
		if exc.errno == errno.EEXIST and os.path.isdir(p):
			pass
		else:
			print('[SYS ERROR]: Unable to create the path {0}'.format(p))
			sys.exit(-1)


def safe_copy_file( source, destination ):
	try:
		shutil.copy(source, destination)
	except (IOError, os.error, shutil.Error) as why:
		print('[SYS_ERROR]: Unable to copy a file in the workspace')
		print('\t-- Source      path: "{0}"'.format(source))
		print('\t-- Destination path: "{0}"'.format(destination))
		print('\t-- Why:              "{0}"'.format(str(why)))
		sys.exit(-1)


def safe_copy_dir( source, destination ):
	try:
		shutil.copytree(source, destination)
	except (IOError, os.error, shutil.Error) as why:
		print('[SYS_ERROR]: Unable to copy a directory in the workspace')
		print('\t-- Source      path: "{0}"'.format(source))
		print('\t-- Destination path: "{0}"'.format(destination))
		print('\t-- Why:              "{0}"'.format(str(why)))
		sys.exit(-1)


#################################
###### Import argo library
#################################


class Workspace:
	"""
	This class holds all the information regarding the structure
	of the workspace folder

	Attributes:
		- working_root           -> the path to the workspace root
		- path_executable_origin -> the path to the original executable (from arguments)
		- configuration_path     -> the path of the common configuration folder
		                            that store the common files (exec + scripts)
	"""

	global_dir_name = 'required_stuff'
	launchpard_dir_name = 'launchpad'
	exec_link_name = 'exec'


	def __init__(self, path_workspace_directory, path_executable):

		# store the information regarding the path of the DSE
		self.working_root = os.path.realpath(path_workspace_directory)
		self.path_executable_origin = os.path.realpath(path_executable)
		self.configuration_path = os.path.join(self.working_root, self.global_dir_name)
		self.executable = os.path.join(self.configuration_path, os.path.basename(path_executable))


	def setup( self, doe, dest_flags, dependencies ):
		"""
		This method creates the common folder of the application which
		holds all the files required to perform the DSE
			- creates the folder structure
			- copy the gangway_cli library
			- copy the original executable and creates symlinks for each configuration
			- updates the configuration paths
			- generates the makefile
			- update the flags for the script that parse the results
		"""

		# check if there is already a dse folder
		if os.path.isdir(self.working_root):
			print('[WARNING] The workspace path "{0}" exists!'.format(self.working_root))
			print("\t-- Select a different workspace path")
			print("\t-- Remove/Rename the target workspace path")
			sys.exit(-1)

		# create the main folder
		mkdir_p(self.working_root)
		mkdir_p(self.configuration_path)

		# copy the executable file
		safe_copy_file(self.path_executable_origin, self.executable)

		# copy the dependencies
		for dependency in dependencies:
			dest_path = os.path.join(self.configuration_path, os.path.basename(dependency))
			safe_copy_file(dependency, dest_path)

		# copy the op generator script file
		current_path = os.path.split(inspect.getfile( inspect.currentframe() ))[0]
		py_ops_generator_path_src = os.path.join(current_path, 'generate_ops.py')
		py_ops_generator_path_dst = os.path.join(self.configuration_path, 'generate_ops.py')
		safe_copy_file(py_ops_generator_path_src, py_ops_generator_path_dst)

		# generate a position independent path for python script
		common_path = os.path.commonpath([self.working_root, py_ops_generator_path_dst])
		relative_path = os.path.relpath(py_ops_generator_path_dst, common_path)
		py_ops_generator_path_dst_rel = os.path.join('.', relative_path)

		# copy the argo gangway library
		gangway_path = os.path.realpath(os.path.join(current_path, '..', '..', "margot_heel", "margot_heel_cli"))
		safe_copy_dir(os.path.join(gangway_path, "margot_heel_cli"), os.path.join(self.configuration_path, 'margot_heel_cli'))

		# loop over the the plan and generate the single runner
		profile_files = {}
		for index_configuration, configuration in enumerate(doe.plan):

			# set the correct folders
			configuration.cwd = os.path.join(self.working_root, self.launchpard_dir_name, configuration.name)
			configuration.executable = os.path.join(configuration.cwd, self.exec_link_name)

			# create the folder
			mkdir_p(configuration.cwd)

			# link the executable (requires python 3.5)
			try:
				os.link(self.executable, configuration.executable)
			except os.Error as why:
				print('[SYS_ERROR] Unable to link the executable in the configuration folder')
				print('\t-- Source : "{0}"'.format(self.executable))
				print('\t-- Link:    "{0}"'.format(configuration.executable))
				sys.exit(-1)

			# link the dependencies
			for dependency in dependencies:
				try:
					dep_name = os.path.basename(dependency)
					os.link(dependency, os.path.join(configuration.cwd, dep_name))
				except os.Error as why:
					print('[SYS_ERROR] Unable to link the executable in the configuration folder')
					print('\t-- Source : "{0}"'.format(dependency))
					print('\t-- Link:    "{0}"'.format(os.path.join(configuration.cwd, dep_name)))
					sys.exit(-1)


			# generate the configuration specific makefile
			percentage = int(float(index_configuration+1) / float(len(doe.plan))*100)

			# get the path to the profile file
			path_profile_file = makefile_generator.generate_configuration_makefile(configuration, percentage)
			common_path = os.path.commonpath([path_profile_file, py_ops_generator_path_dst])
			rebased_path_log = os.path.relpath(path_profile_file, common_path)
			rebased_path_script = os.path.relpath(py_ops_generator_path_dst, common_path)
			script_path = os.path.dirname(rebased_path_script)
			rebased_root = '{0}'.format(os.path.sep).join(['..' for x in script_path.split(os.path.sep)])
			profile_files[os.path.join(rebased_root, rebased_path_log)] = configuration.description

		# compose the flags for the python script that generates the Operating Points list
		command_flags = generate_ops.generate_profile_flags(profile_files)
		command_flags.extend(dest_flags)

		# compose the relative path between the python script and the oplist
		command_flags.extend(generate_ops.generate_outfile_flag(os.path.join('.', 'oplist.conf')))

		# generate the global makefile
		makefile_generator.generate_global_makefile(doe, self.working_root, py_ops_generator_path_dst_rel, command_flags)
