#!/bin/env python3




#################################
###### Import system packages
#################################
from __future__ import print_function
import argparse                          # processing the input
import os.path as path                   # handling the paths
import inspect                           # getting the path of this script
import os                                # for creating the path
import errno                             # for checking before creating a path
import sys                               # for exiting the script


#################################
###### Import local packages
#################################
from dse_gen import doe
from dse_gen import workspace
from dse_gen import application
from dse_gen import generate_ops



#################################
###### Import argo library
#################################
try:
	import margot_heel_cli
except ImportError:

	# Add the python scripts folder
	src_folder = os.path.realpath(os.path.join(os.path.split(inspect.getfile( inspect.currentframe() ))[0], "..", "margot_heel", "margot_heel_cli"))
	sys.path.insert(1, src_folder)

	# Import again the library
	try:
		import margot_heel_cli

	except ImportError:
		print('[ERROR]: Unable to find the margot_heel_cli library!')
		print('         Possible causes:')
		print('             - argotool is not installed as python package')
		print('             - This file has been moved wrt the library position')
		print(src_folder)
		sys.exit(-1)


if __name__ == "__main__":


	# ------- Create the argument parser for the application

	# create the main argument parser
	arg_parser = argparse.ArgumentParser(description='Utility script that generates a dse campaing')
	arg_parser.add_argument('--version',
	                        action = 'version',
	                        version = 'mARGOt dse 1.0',
	                        help = 'Print the version of the tools and exit')

	# create the parser arguments
	arg_parser.add_argument('application',
	                        metavar = 'APP',
	                        type = str,
	                        nargs = 1,
	                        help = 'The path to the application description, in XML format')
	arg_parser.add_argument('--strategy',
	                        metavar = 'DOE',
	                        dest = 'doe',
	                        type = str,
	                        required = False,
	                        choices = doe.DoE.supported_does,
	                        default = 'full-factorial',
	                        help='The used Design Of Experiments')
	arg_parser.add_argument('--executable',
	                        metavar = 'BIN',
	                        dest = 'executable',
	                        type = str,
	                        required = True,
	                        help='The path to the application executable')
	arg_parser.add_argument('--workspace',
	                        metavar = 'OUT',
	                        dest = 'out_folder',
	                        type = str,
	                        required = False,
	                        default = './dse_campaign',
	                        help = 'The path to write the DSE campaign')



	# ------- Parse the argument
	args = arg_parser.parse_args()
	path_application_description = args.application[0]
	path_workspace_directory = args.out_folder
	path_executable = args.executable
	doe_strategy = args.doe
	print('Preparing DSE with the following parameters:')
	print(' -- Path app description: "{0}"'.format(path_application_description))
	print(' -- Workspace directory:  "{0}"'.format(path_workspace_directory))
	print(' -- Path executable:      "{0}"'.format(path_executable))
	print(' -- DoE strategy:         "{0}"'.format(doe_strategy.upper()))


	# ------- Parse the application description
	my_application = application.Application(path_application_description)


	# ------- Generate the flags for the generator that will be called later
	application_flags = generate_ops.generate_application_flags(my_application)


	#Workspace setup:
	my_workspace = workspace.Workspace(path_workspace_directory, path_executable)
	my_workspace.setup(my_application.dependencies, len(my_application.flags.keys()))
	for index_folder_ID in my_application.flags.keys():
	# ------- Build the DoE plan
		my_doe_plan = doe.DoE(doe_strategy, my_application, index_folder_ID)
	# ------- Generate the workspace
		my_workspace.doe_setup(my_doe_plan, application_flags, my_application.dependencies, index_folder_ID)
	#build the global makefile. The oplist reported is the output of the first DOE (launchpad/0 if default naming is maintained). Other oplists are in the other folders, ready for post process.
	my_workspace.finalize_makelists(application_flags)

