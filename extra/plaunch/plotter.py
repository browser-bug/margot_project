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


class TraceModel:
	"""
	This class represents a single trace of the application. It takes for granted that the first line
	is the header that gives a name to to the column.

	Attributes
		- data [[]]   -> The parsed data structure as in the csv file
		- header {}   -> A dictionary to correlate the name of the column with its index
		                   key   -> name of the column
		                   value -> index of the column
		- name        -> The name of the source file
	"""

	def __init__(self):
		"""
		Parse the csv and initialize the internal data structure

		Arguments:
			- path_to_file -> The path of the target trace file
			- delimiter    -> The character used to split the columns
		"""

		# initialize the internal data structer
		self.data = []
		self.header = {}
		self.name = ''
		self.file_path = ''
		self.start_time = ''
		self.stop_time = ''
		self.minimum_timestamp = 0


	def load(self, my_delimiter = ' '):

		# parse the whole csv file
		csvmatrix = []
		with open(self.file_path, 'r') as csvfile:
			tracereader = csv.reader(csvfile, delimiter = my_delimiter)
			for row in tracereader:
				csvmatrix.append(row)

		# compute the header
		header_raw = csvmatrix.pop(0)
		for index_col, name_col in enumerate(header_raw):
			self.header[name_col] = index_col

		# convert the data
		for row in csvmatrix:
			self.data.append([float(x) for x in row])


	def __nonzero__(self):
		"""
		Python 2.x check for emptyness
		"""
		if self.data:
			return True
		else:
			return False


	def __bool__(self):
		"""
		Python 3.x check for emptyness
		"""
		if self.data:
			return True
		else:
			return False


	def get_column(self, field_name):
		"""
		Return up to two lists of data
			- the list of values (monitored if any, otherwise statics)
			- the list of goals (if any)
		"""

		static_name = ''
		observed_name = ''
		goal_name = ''

		# loop over the metric stored in the trace
		for header_name in self.header:

			# split it to tokenize
			name_tokenized = [ x.upper() for x in header_name.split('_')]

			# check if field is in there
			if field_name.upper() in name_tokenized:

				# check if it is a field
				if 'KNOB' in name_tokenized:
					static_name = header_name
				elif 'AVERAGE' in name_tokenized:
					observed_name = header_name
				elif 'KNOWN' in name_tokenized:
					static_name = header_name
				elif 'GOAL' in name_tokenized:
					goal_name = header_name

		# if there is no static nor dynamic name, boil out
		if (not static_name) and (not observed_name):
			print('[ERROR] unknown field "{0}"'.format(field_name))
			sys.exit(-1)

		if not observed_name:
			observed_name = static_name

		if goal_name:
			return [x[self.header[observed_name]] for x in self.data], [x[self.header[goal_name]] for x in self.data]
		else:
			return [x[self.header[observed_name]] for x in self.data], []


	def get_timestamp(self):
		"""
		Return the list of the timestamp.

		Arguments
			- normalize  -> If true it removes the initial offset from the data
		"""

		# get the values
		timestamps = [x[self.header['Timestamp']] for x in self.data]

		# normalize them according to the experiment
		timestamps = [x - self.minimum_timestamp for x in timestamps]

		return timestamps




class DataSerie:
	"""
	This class is basically a matrix with additional information about its content
	"""

	def __init__(self):
		self.name = ''
		self.application_name = ''
		self.data = {}
		self.data_header = []
		self.data_size = 0

	def insert( self, column_data, column_name):

		# append the data
		self.data_header.append(column_name)
		self.data[column_name] = column_data

		# check if all the data has the same size
		min_size = min(len(self.data[x]) for x in self.data)
		max_size = max(len(self.data[x]) for x in self.data)
		if max_size != min_size:
			print('[ERROR] The data size load doesn\'t match')
			sys.exit(-1)
		self.data_size = min_size

	def write_data(self, where):
		with open(os.path.join(where, '{0}.dat'.format(self.name)) , 'w') as data_file:
			for data_index in range(self.data_size):
				row = []
				for header in self.data_header:
					row.append(self.data[header][data_index])
				data_file.write('{0}\n'.format(', '.join(str(x) for x in row)))



def write_gnuplot_script_header( out_file ):
	out_file.write(
"""set xtic auto                          # set xtics automatically
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
	)





def generate_data_files( trace_models, root_plot_path):
	"""
	Generate the data file to be plotted
	"""

	# declare the list of data files
	data_series = []

	# loop over the models
	for trace_model_name in trace_models:

		# creates the directory of the data model
		path_trace_model = os.path.join(root_plot_path, trace_model_name)
		mkdir_p(path_trace_model)

		# loop over the field of interest
		for field_interest in args.fields.split(','):

			# skip null values
			if not field_interest:
				continue

			# creates the data series
			my_data_serie = DataSerie()
			my_data_serie.name = field_interest
			my_data_serie.application_name = trace_model_name

			# get the data and goal from the trace
			data, goal =  trace_models[trace_model_name].get_column(field_interest)

			# append the timestamp
			my_data_serie.insert(trace_models[trace_model_name].get_timestamp(), 'Timestamp')

			# append the data
			my_data_serie.insert(data, '{0} value'.format(field_interest))

			# optionally append the goal value
			if goal:
				my_data_serie.insert(goal, '{0} goal'.format(field_interest))

			# write the data
			my_data_serie.write_data(path_trace_model)

			# append the data serie
			data_series.append(my_data_serie)

	return data_series






def generate_plot_single( data_serie, out_file ):
	# write the header
	write_gnuplot_script_header(out_file)

	# write the labels
	out_file.write('set xlabel "Timestamp"\n')
	out_file.write('set ylabel "{0}"\n\n\n'.format(data_serie.name))

	# write the plot commands
	if len(data_serie.data_header) == 1:
		out_file.write('unset key\nplot ')
		out_file.write('"{0}" using 1:2 with linespoints ls 1\n')
	else:
		out_file.write('plot ')
		for index, line in enumerate(data_serie.data_header):
			if (index > 0):
				out_file.write('"{0}.dat" using 1:{1} title "{2}" with linespoints ls {1}'.format(data_serie.name, index + 1, line))
				if index + 1 < len(data_serie.data_header):
					out_file.write(', \\\n\t')

	# write the last line
	out_file.write('\n')


def generate_plot_multi( data_series, out_file ):
	# write the header
	write_gnuplot_script_header(out_file)

	# write the labels
	out_file.write('set xlabel "Timestamp"\n')
	out_file.write('set ylabel "{0}"\n\n\nplot '.format(data_series[0].name))


	# loop over the data serie
	counter = 1
	for index_data_serie, data_serie in enumerate(data_series):

		# write the plot commands
		for index, line in enumerate(data_serie.data_header):
			if (index > 0):
				out_file.write('"{0}.dat" using 1:{1} title "{2}" with linespoints ls {3}'.format('{0}/{1}'.format(data_serie.application_name, data_serie.name), index + 1, '{0}.{1}'.format(data_serie.application_name, line), counter))
				if index + 1 < len(data_serie.data_header):
					out_file.write(', \\\n\t')
				counter += 1

		if index_data_serie + 1 < len(data_series):
			out_file.write(', \\\n\t')


	# write the last line
	out_file.write('\n')




def plot_experiment( experiments, where ):
	"""
	This function generate the plot for a given experiment
	"""

	# create the directory
	mkdir_p(where)

	# generate the data file
	data_series_original = generate_data_files(experiments, where)

	# store all the gnuplot files
	gnuplot_files = []

	# plot the single images
	for data_serie in data_series_original:
		file_name = os.path.join(where, data_serie.application_name, '{0}.gnuplot'.format(data_serie.name))
		with open(file_name, 'w') as single_out_file:
			generate_plot_single(data_serie, single_out_file)
		gnuplot_files.append(file_name)


	# plot the multiple images
	for field in args.fields.split(','):
		# get all the data_serie involved
		my_data_series = [ x for x in data_series_original if x.name == field]

		# write the gnuplot file
		file_name = os.path.join(where, '{0}.gnuplot'.format(field))
		with open(file_name, 'w') as multi_out_file:
			generate_plot_multi( my_data_series, multi_out_file)

		# store the file
		gnuplot_files.append(file_name)

	# generate all the images
	for gnuplot_file_name in gnuplot_files:

		# get the basename
		gnuplot_script_name, extension = os.path.splitext(gnuplot_file_name)

		# get the working directory
		working_directory = os.path.dirname(gnuplot_file_name)

		# open the input and output files
		with open(gnuplot_file_name, 'r') as input_file:
			with open('{0}.eps'.format(gnuplot_script_name), 'w') as output_file:

				# run the gnuplot command
				process = subprocess.Popen(['gnuplot'], cwd=working_directory, stdout=output_file, stdin=input_file)
				process.communicate()




if __name__ == "__main__":


	# ------- Create the argument parser for the application

	# create the main argument parser
	arg_parser = argparse.ArgumentParser(description='Utility script that plots all the experiments from plaunch')
	arg_parser.add_argument('--version', action='version', version='PLAUNCH printer 1.0', help='Print the version of the tools and exit')
	arg_parser.add_argument('where', metavar='PATH', type=str, nargs=1, help='The root path of the experiment')
	arg_parser.add_argument('--log_file', metavar='LOGFILE', dest='log_file', type=str, required=True, help='The name of the log file for the application')
	arg_parser.add_argument('--plot', metavar='FIELDS', dest='fields', type=str, required=True, help='The name of the target fields for the y axis (coma separated)')
	arg_parser.add_argument('--delimiter', metavar='DEL', dest='delimiter', type=str, required=False, default=' ', help='The delimiter character fpr the traces')


	# ------- Parse the argument
	args = arg_parser.parse_args()


	# Define all the path related to the script
	path_root_experiments = os.path.realpath(os.path.abspath(args.where[0]))
	path_argo_cli = os.path.realpath(os.path.join(os.path.abspath('..'), 'argo', 'argo_gangway', 'argo_gangway_cli', 'bin'))

	# Compose the first part of the plotting command
	command_generate_gnuplot_script_base = [ 'python', os.path.join(path_argo_cli, 'argo_cli')]
	command_generate_eps_base = ['gnuplot']

	# Walk to the file system and look for the experiment
	for root, dirs, files in os.walk(path_root_experiments, topdown = True):

		# assume that the folder is actually an experiment
		try:

			# look for the experiment root log file
			experiments_dictionary = {}
			with open(os.path.join(root, 'experiment.log')) as log_file_experiment:

				# parse it from csv
				csvreader = csv.reader(log_file_experiment, delimiter=args.delimiter)

				# skip the header
				next(csvreader)

				# loop over all the traces of execution
				for row in csvreader:

					# read the start_time, stop_time and name
					application_full_name = row[0]
					application_start_time = float(row[1])
					application_stop_time = float(row[2])
					application_id = application_full_name.split('.')[-1]

					# load the trace for the application
					my_trace_model = TraceModel()
					my_trace_model.name = application_full_name
					my_trace_model.file_path = os.path.join(root, application_id, args.log_file)
					my_trace_model.start_time = application_start_time
					my_trace_model.stop_time = application_stop_time
					my_trace_model.load(args.delimiter)

					# add the trace to the dictionary
					experiments_dictionary[application_full_name] = my_trace_model

			# normalize the start_time globally
			min_value_timestamp = min(experiments_dictionary[x].start_time for x in experiments_dictionary)
			for application_name in experiments_dictionary:
				experiments_dictionary[application_name].minimum_timestamp = min_value_timestamp

			# plot the whole experiment
			plot_experiment(experiments_dictionary, os.path.join(root, 'plot'))



		# handle any IO exception on opening the file
		except IOError as e:
			if e.errno == errno.ENOENT:
				# it is ok, it is just not the root path
				pass
			else:
				raise
