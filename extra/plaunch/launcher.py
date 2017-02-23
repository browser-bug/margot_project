#/bin/env python
# ----- IMPORT SECTION

# Python import
import sys, os, inspect
import argparse
import time
from plaunch import parse_scenario as p


#################################
###### Import plaunch library
#################################
try:
	import plaunch
except ImportError:

	# Add the python scripts folder
	src_folder = os.path.realpath(os.path.join(os.path.split(inspect.getfile( inspect.currentframe() ))[0], "plaunch"))
	if src_folder not in sys.path:
		sys.path.insert(1, src_folder)

	# Import again the library
	try:
		import plaunch

	except ImportError:
		print('[ERROR]: Unable to find the plaunch library!')
		print('         Possible causes:')
		print('             - plaunch is not installed as python package')
		print('             - This file is been moved wrt the library position')
		print(src_folder)
		sys.exit(-1)




#################################
###### Execute the scenario
#################################
if __name__ == "__main__":

	# Parse the input parameters
	parser = argparse.ArgumentParser(description='Launch a scenario')
	parser.add_argument('--scenario,-s,-S', type=str, default='conf/scenario.xml', dest='scenario',
                   help='The path of the scenario\'s configuration file', required=True)
	parser.add_argument('--iterations,-i,-I', type=int, default=1, dest='iterations',
                   help='How many times the scenario must be repeated', required=False)
	args = parser.parse_args()



	# creates a list of scenarios
	scenarios = []


	# loop over the iterations
	for iteration_index in range(args.iterations):

		# print header
		try:
			print('\n\nRunning iteration {0}/{1}, ETA {2} seconds'.format(iteration_index+1, args.iterations, eta))
		except NameError:
			print('Running iteration {0}/{1}'.format(iteration_index+1, args.iterations))


		# creates the scenario model
		scenario_model = p.parse_scenario(args.scenario)

		# setup the scenario
		scenario_model.setup()

		# play the scenario
		start = time.time()
		scenario_model.play()
		stop = time.time()
		print('Iteration done in {0} seconds'.format(stop-start))

		# Compute the eta
		eta = (stop - start)*(args.iterations - iteration_index - 1)

		# teardown the scenario
		scenario_model.teardown()

		# append it to the list
		scenarios.append(scenario_model)

