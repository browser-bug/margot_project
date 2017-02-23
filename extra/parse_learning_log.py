#!/bin/env python


#################################
###### Import system packages
#################################
from __future__ import print_function
import argparse                          # processing the input
import os                                # for path related stuff
import csv                               # for reading the experiment overall data
import errno                             # for getting meaningfull error messages
import sys                               # for exiting
import subprocess                        # for running gnuplot


def mkdir_p(path):
	try:
		os.makedirs(path)
	except OSError as exc:
		if exc.errno == errno.EEXIST and os.path.isdir(path):
			pass
		else: raise





class MabArm:
	"""
	This class represents a mab arm

	It defines:
		- arm_index: the index of the considered arm
		- rewards:   the list of rewards:
			- [reward_factor, uncertainty_factor, reward]
	"""

	def __init__(self, arm_index):
		self.arm_index = arm_index
		self.rewards = []



	def print_data( self ):

		# write the header
		print('# Considering the reward from the ARM {0}'.format(self.arm_index))
		print('# Time index | Reward factor | Uncertainty factor | Total reward')

		# loop over the arms
		for data_index, data in enumerate(self.rewards):
			print('{0} {1} {2} {3}'.format(data_index, data[0], data[1], data[2]))

		print('e')





class Parsing:
	begin = 1
	arm = 2
	actual_arm_reward = 3




class MabTrace:
	"""
	This class basically parse a given log file to capture the status of the mab
	framework

	Attributes
		- arms [[]]     -> The list of arms that compose the mab trace
		- observed_reward -> The list of the reward of the used arm
	"""

	def __init__(self, path_log_file):

		# define the state attributes
		self.arms = []
		self.observed_reward = []

		# actually read the file
		with open(path_log_file, 'r') as infile:
			file_string = list(infile)

		# pop lines until you get the number of arms
		found_arms = False
		for index, line in enumerate(file_string):
			tokenized_line = line.split(' ')
			if tokenized_line:
				if tokenized_line[0] == 'Generated':
					try:
						number_arms = int(tokenized_line[1])
						found_arms = True
						break
					except ValueError as err:
						print('[ERROR] Unknown number of generated lines: "{0}"'.format(tokenized_line[1]))
						raise
		if not found_arms:
			print('[ERROR] Reached the end of file ({0} lines) without knowing the number of generated arms'.format(index+1))
			sys.exit(-1)
		else:
			file_string = file_string[index+1:]

		# creates the required arms
		for i in range(number_arms):
			self.arms.append(MabArm(i))

		# clean the log file from useless character
		file_string = [x.replace('\t','').replace('\n', '') for x in file_string if x != '\n']



		# loop over the result
		state = Parsing.begin
		for line in file_string:

			# skip the best message
			if line == 'Is the new best!':
				continue

			# save the stored reward
			if line[:6] == 'Stored':

				# split the line and tokenize it
				tokenized_line = line.split(' ')
				tokenized_line = [x for x in tokenized_line if x]

				# store the reward
				self.observed_reward.append(float(tokenized_line[2].replace('"','')))

				# done, parse the remainder of the document
				continue



			# skip the check on the number of elements
			if line[:17] == 'Need to eliminate':
				continue

			# handle the begin state
			if state == Parsing.begin:
				if line != 'Requested to retrieve the best configuration':
					raise ValueError(line)
				arm_counter = 0
				state = Parsing.arm


			# handle the case of the arm state
			elif state == Parsing.arm:
				if line != 'Evaluating Arm {0}'.format(arm_counter):
					raise ValueError('{0}'.format(line))
				actual_reward = []
				state = Parsing.actual_arm_reward


			# parse the real reward
			elif state == Parsing.actual_arm_reward:

				# split the line and clean it
				tokenized_line = line.split(' ')
				tokenized_line = [x for x in tokenized_line if x]

				# check if it is the total reward
				if (tokenized_line[0] == 'Reward') and (tokenized_line[1] == ':'):

					# store the reward
					if actual_reward:
						actual_reward.append(float(tokenized_line[2]))
						self.arms[arm_counter].rewards.append([str(x) for x in actual_reward])
					else:
						self.arms[arm_counter].rewards.append(['N/A', 'N/A', '{0}'.format(float(tokenized_line[2]))])

					# increment the arm counter
					arm_counter += 1

					# change the state
					if arm_counter == len(self.arms):
						state = Parsing.begin
					else:
						state = Parsing.arm


				# check if it is the reward factor
				elif (tokenized_line[0] == 'Reward') and (tokenized_line[1] == 'factor'):

					# append the reward
					actual_reward.append(float(tokenized_line[3]))


				elif (tokenized_line[0] == 'Uncertainty') and actual_reward:
					# append the reward
					actual_reward.append(float(tokenized_line[2]))

				else:
					raise ValueError('Unexpected line "{0}"'.format(line))


	def print_gnuplot_trace(self):

		script_preamble = """set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set key above                          # set the legend

set terminal postscript eps enhanced color font ',20'

# Line style for axes and grid
set style line 80 lt 1 lw 2
set style line 80 lc rgb "#000000" lw 2
set style line 81 lt 3 lw 1  # dashed
set style line 81 lt rgb "#808080" lw 2  # grey


# Beautify the grid and the axis
set grid back linestyle 81
set border 3 back linestyle 80
set xtics nomirror
set ytics nomirror

# Line styles for the data traces
set style line 1 lt rgb "#1f78b4" lw 4 pt 7
set style line 2 lt 1 lc rgb "#33a02c" lw 4 pt 9
set style line 3 lt rgb "#e31a1c" lw 4 pt 5
set style line 4 lt rgb "#ff7f00" lw 4 pt 13
set style line 5 lt rgb "#6a3d9a" lw 4 pt 7
set style line 6 lt rgb "#b15928" lw 4 pt 9
set style line 7 lt rgb "#a6cee3" lw 4 pt 5
set style line 8 lt rgb "#b2df8a" lw 4 pt 13
set style line 9 lt rgb "#fb9a99" lw 4 pt 7
set style line 10 lt rgb "#fdbf6f" lw 4 pt 9
set style line 11 lt rgb "#cab2d6" lw 4 pt 5
set style line 12 lt rgb "#ffff99" lw 4 pt 13
\n\n
"""

		# print the header
		print(script_preamble)

		# set the range for plotting
		print('set yrange [0:2000]')

		# define the nan terminator
		print('set datafile missing "N/A"')

		print('\n')

		# print the command to plot the actual reward
		print('plot "-" u 1:2 title "actual reward" with linespoints ls 1,\\')

		# loop over the arms
		for arm_index, arm in enumerate(self.arms):
			print('\t"-" u 1:4 title "Total reward arm {0}" with linespoints ls {1}'.format(arm.arm_index, arm.arm_index + 2), end="")
			if arm_index + 1 < len(self.arms):
				print(',\\')
			else:
				print('')

		# print the reward series
		for time_index, reward in enumerate(self.observed_reward):
			print('{0} {1}'.format(time_index, reward))
		print('e')

		# print the
		for arm in self.arms:
			arm.print_data()








if __name__ == "__main__":


	# ------- Create the argument parser for the application

	# create the main argument parser
	arg_parser = argparse.ArgumentParser(description='Utility script that plots the learning stuff from the output log')
	arg_parser.add_argument('--version', action='version', version='MAB printer 1.0', help='Print the version of the tools and exit')
	arg_parser.add_argument('where', metavar='PATH', type=str, nargs=1, help='The root path of the experiment')


	# ------- Parse the argument
	args = arg_parser.parse_args()


	# Find the real path of the log file
	log_path = os.path.realpath(os.path.abspath(args.where[0]))

	# Create the representation of the mab evolution
	model = MabTrace(log_path)

	# Print the data trace
	model.print_gnuplot_trace()
