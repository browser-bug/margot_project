import os, errno
import threading
import time
import subprocess
import shutil


###############################
#### GET REAL CLOCK FROM C
###############################

__all__ = ["monotonic_time"]

import ctypes, os

CLOCK_MONOTONIC     = 1 # see <linux/time.h>
CLOCK_MONOTONIC_RAW = 4 # see <linux/time.h>

class timespec(ctypes.Structure):
    _fields_ = [
        ('tv_sec', ctypes.c_long),
        ('tv_nsec', ctypes.c_long)
    ]

librt = ctypes.CDLL('librt.so.1', use_errno=True)
clock_gettime = librt.clock_gettime
clock_gettime.argtypes = [ctypes.c_int, ctypes.POINTER(timespec)]


# you get it in SECONDS
def monotonic_time():
    t = timespec()
    if clock_gettime(CLOCK_MONOTONIC , ctypes.pointer(t)) != 0:
        errno_ = ctypes.get_errno()
        raise OSError(errno_, os.strerror(errno_))
    return t.tv_sec + t.tv_nsec * 1e-9




###############################
#### UTILITY FUNCTIONS
###############################


def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc:
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise


###############################
#### REAL APPLICATION CLASS
###############################


class Application(threading.Thread):
	"""
	This class represents the application instance that must be executed
	It is composed by the following information:
		- the execution folder
		- the list of dependencies to be copied in the execution folder
		- the list of output files to be copied in the output folder
		- the binary file of the application
		- the list of flag of the application
		- the name of the log files (wrt the application binary)
		- the delay of the execution
	"""

	def __init__(self):

		# init the thread class
		threading.Thread.__init__(self)

		# scenario dependent attributes
		self.input_files = {} # key -> original_path, value -> dest_path
		self.input_dirs = {} # key -> original_path, value -> dest_path
		self.binary_file = ""
		self.run_flags = {} # key -> flag name, value -> flag value
		self.log_files = []
		self.delay = 1
		self.name = "" # the application name (override the thread name)
		self.application_name = "" # the actual name of the application


		# scenario independent attributes
		self.execution_folder = ""
		self.output_folder = ""
		self.start_time = time.time()
		self.stop_time = time.time()
		self.id = '0'
		self.command = []
		self.output_files = []



	def setup(self):
		"""
		Set-up the execution folder in a way that it is ok to execute the application
		"""

		# add the stdout and stderr to the output_files
		self.output_files.append('stdout.txt')
		self.output_files.append('stderr.txt')

		# add the log file to the output files
		for log_file in self.log_files:
			self.output_files.append(log_file)

		# attempt to create the execution folder
		mkdir_p(os.path.join(self.execution_folder, self.id))

		# copy all the input files
		for in_file in self.input_files:
			file_name = os.path.basename(os.path.normpath(in_file))
			destination_path = os.path.join(self.execution_folder, self.id, self.input_files[in_file], file_name)
			shutil.copyfile(in_file, destination_path)

		# copy all the input direcotries
		for in_dir in self.input_dirs:
			destination_path = os.path.join(self.execution_folder, self.id, self.input_dirs[in_dir])
			shutil.copytree(in_dir, destination_path)

		# Copy the executable
		exec_path, exec_name = os.path.split(self.binary_file)
		shutil.copy2(self.binary_file, os.path.join(self.execution_folder, self.id, exec_name))
                for flag_name in self.run_flags:
                        if flag_name == '--exec':
                                real_exec_name = os.path.basename(self.run_flags[flag_name])
                                shutil.copy2(self.run_flags[flag_name],os.path.join(self.execution_folder,self.id,real_exec_name))
		# Set the correct command line argument
		self.command = [str(os.path.join(self.execution_folder, self.id, exec_name))]
		for flag_name in self.run_flags:
                        #this is the normal behavior, the exec flag is needed for the routing dse and output analysis. the executable file is copied in the execution folder. use this if is the second level executable that need the isolation, since for this dse we are using double lvl scripting wrap.
                        if flag_name != '--exec':
                                self.command.append( flag_name )
            			self.command.append( self.run_flags[flag_name] )
                        else:
                                self.command.append( flag_name )
                                self.command.append( os.path.basename(self.run_flags[flag_name]))
	def commit(self):
		"""
		Copies all the outut file in the output folder
		"""

		# compute the output folder
		out_folder = os.path.join(self.output_folder, self.id)

		# creates the directory
		mkdir_p(out_folder)

		# copy the files
		for out_file in self.output_files:

			# compute the actual paths
			out_path = os.path.join(out_folder, out_file)
			source_file = os.path.join(self.execution_folder, self.id, out_file)

			# actually copy the file
			shutil.copyfile(source_file, out_path)


	def teardown(self):
		"""
		Tear-down the execution folder for the application
		"""
		shutil.rmtree(os.path.join(self.execution_folder, self.id))

	def run(self):
		"""
		This method actually execute the application
		"""
		# sleep for the delay
		if self.delay > 0:
			print('\t{0}: sleeping {1}s'.format(threading.current_thread().name, self.delay))
			time.sleep(self.delay)

		# actually run the application
		self.start_time = monotonic_time() * 1000000 # Argo uses microseconds in timestamps
		with open(os.path.join(self.execution_folder, self.id, 'stdout.txt'), "w") as outfile:
			with open(os.path.join(self.execution_folder, self.id, 'stderr.txt'), "w") as errfile:
				working_directory = os.path.join(self.execution_folder, self.id)
				print (self.command)
                                process = subprocess.Popen(' '.join(self.command), cwd=working_directory, stdout=outfile, stderr=errfile, shell=True)
				print('\t{0}: executing "{1}" (pid {2})'.format(threading.current_thread().name, ' '.join(self.command), process.pid))
				process.communicate()
		self.stop_time = monotonic_time() * 1000000 # Argo uses microseconds in timestamps

		# notify the execution time
		elapsed_time = self.stop_time - self.start_time
		print('\t{0}: terminated in {1} seconds'.format(threading.current_thread().name, elapsed_time))
