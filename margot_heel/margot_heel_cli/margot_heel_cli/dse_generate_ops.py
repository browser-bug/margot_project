#!/bin/env python


################################
###### Import system packages
#################################
from __future__ import print_function
import argparse                          # processing the input
import inspect                           # getting the path of this script
import os                                # for creating the path
import errno                             # for checking before creating a path
import sys                               # for exiting the script
import statistics                        # for trace analysis
import pickle                            # for read the doe file



def generate_application_flags(application_model):

  # the container of the flags for this application
  command_flags = {}

  # prepare the translators for the non numeric values
  knob_names = sorted(application_model.knob_values.keys())
  translator = {}
  reverse_translator = {}
  for knob_name in knob_names:
    numeric = True
    for knob_value in application_model.knob_values[knob_name]:
      try:
        float(knob_value)
      except:
        numeric = False
        break
    if not numeric:
      translator[knob_name] = {}
      reverse_translator[knob_name] = {}
      sorted_values = sorted(application_model.knob_values[knob_name])
      for counter, value in enumerate(sorted_values):
        translator[knob_name][value] = counter
        reverse_translator[knob_name][counter] = value
  command_flags.update({'translator':translator})   #extend(['--translator', '"{0}"'.format(str(translator))])
  command_flags.update({'reverse_translator':reverse_translator})    #extend(['--reverse_translator', '"{0}"'.format(str(reverse_translator))])

  # prepare the flags for the remainder of the application fields
  command_flags.update({'added_metrics':application_model.compute})  #extend(['--added_metrics', '"{0}"'.format(str(application_model.compute))])
  command_flags.update({'observed_metrics':application_model.metrics})  #extend(['--observed_metrics', '"{0}"'.format(str(application_model.metrics))])
  command_flags.update({'block_name':application_model.block})  #extend(['--block_name', application_model.block])

  return command_flags


def generate_profile_flags(profile_files):
  return {'doe':profile_files}


def generate_outfile_flag(output_file):
  return {'outfile':output_file}




if __name__ == "__main__":


  #################################
  ###### Import argo library
  #################################
  try:
    import margot_heel_cli
  except ImportError:

    # Add the python scripts folder
    src_folder = os.path.realpath(os.path.join(os.path.split(inspect.getfile( inspect.currentframe() ))[0]))
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


  # ------- Create the argument parser for the application

  # create the main argument parser
  arg_parser = argparse.ArgumentParser(description='Utility script that generates a the Operating Point list')
  arg_parser.add_argument('--version',
                          action = 'version',
                          version = 'ARGO op gen 1.0',
                          help = 'Print the version of the tools and exit')

  # create the parser arguments
  arg_parser.add_argument('--data',
                          metavar = 'DA',
                          dest = 'data_filename',
                          type = str,
                          required = True,
                          default = "",
                          help='The pickle file with the doe dictionaty')
# arg_parser.add_argument('--translator',
#                         metavar = 'TR',
#                         dest = 'translator_str',
#                         type = str,
#                         required = False,
#                         default = "translator = {}",
#                         help='The representation of the translator for the op list')
# arg_parser.add_argument('--reverse_translator',
  #                        metavar = 'RTR',
  #                        dest = 'reverse_translator_str',
  #                        type = str,
  #                        required = False,
  #                        default = "reverse_translator = {}",
  #                        help='The representation of the reverse_translator for the op list')
  #arg_parser.add_argument('--added_metrics',
  #                        metavar = 'CMs',
  #                        dest = 'added_metrics',
  #                        type = str,
  #                        required = False,
  #                        default = {},
  #                        help = 'The description of the metric that should be derived from the observed ones')
  #arg_parser.add_argument('--block_name',
  #                        metavar = 'BLOCK',
  #                        dest = 'block_name',
  #                        type = str,
  #                        required = False,
  #                        default = 'elaboration',
  #                        help = 'The name of the target block')
  #arg_parser.add_argument('--doe',
  #                        metavar = 'DOE',
  #                        dest = 'doe',
  #                        type = str,
  #                        required = False,
  #                        default = '{}',
  #                        help = 'The description of the DoE to reconstruct the Operating Point')
  #arg_parser.add_argument('--observed_metrics',
  #                        metavar = 'OBS',
  #                        dest = 'metrics',
  #                        type = str,
  #                        required = False,
  #                        default = '{}',
  #                        help = 'The description of the profiled metrics')
  #arg_parser.add_argument('--outfile',
  #                        metavar = 'OUT',
  #                        dest = 'out_file',
  #                        type = str,
  #                        required = True,
  #                        default = 'oplist.conf',
  #                        help = 'The filename of the generated Operating Points list')



  # ------- Parse the arguments
  args = arg_parser.parse_args()

  with open (args.data_filename, 'rb') as f:
    dse_data = pickle.load(f)

  # ------- Build the oplist model
  from margot_heel_cli import model_op_list
  from margot_heel_cli import model_op
  my_op_list_model = model_op_list.OperatingPointListModel()
  my_op_list_model.name = dse_data['block_name']


  # ------- Populate the fields translator
  my_op_list_model.translator = dse_data['translator']
  my_op_list_model.reverse_translator = dse_data['reverse_translator']

  # ------- Get the metrics that should be completed
  metric_to_add = dse_data['added_metrics']

  # ------- Compose the Operating Point List
  this_file_path = os.path.realpath(os.path.join(os.path.split(inspect.getfile( inspect.currentframe() ))[0]))
  from margot_heel_cli import model_trace
  from margot_heel_cli import op_utils
  doe_plan = dse_data['doe']
  print('**************************************************')
  print('************        DSE REPORT        ************')
  print('**************************************************')
  for trace_index, trace_file in enumerate(doe_plan):

    # get the normalized path
    trace_file_path = os.path.realpath(os.path.join(this_file_path, trace_file))


    print()
    print('Evaluating the Operating Point {0}'.format(trace_index))
    print('\t-- Trace file : {0}'.format(trace_file_path))


    # get the knob description
    description = doe_plan[trace_file]

    # creates an Operating Point model
    my_op_model = model_op.OperatingPointModel()

    # add the knob values
    print('\t-- Configuration:')
    for knob_name in description:
      print('\t\t"{0}" -> "{1}"'.format(knob_name, description[knob_name]), end = '')
      if knob_name in my_op_list_model.translator.keys():
        my_op_model.knobs[knob_name] = str(my_op_list_model.translator[knob_name][description[knob_name]])
        print(' ({0})'.format(my_op_model.knobs[knob_name]))
      else:
        my_op_model.knobs[knob_name] = description[knob_name]
        print()

    # add the metric values
    my_trace_model = model_trace.TraceModel(trace_file_path)
    profile_metric_description = dse_data['observed_metrics']

    print('\t-- Profiled information:')
    for metric_name in profile_metric_description:

      # get the Data
      values = my_trace_model.get_column(profile_metric_description[metric_name])

      # compute the average
      mean_value = statistics.mean(values)
      my_op_model.metrics[metric_name] = mean_value

      # compute the standard deviations
      stddev = 0
      if (len (values) > 1):
        stddev = statistics.stdev(values) / 2
      if stddev != 0:
        stddev_perc = float(int(1000 * stddev / mean_value)) / 10

      # add the standard deviation to the Operating Point model
      my_op_model.metrics_std[metric_name] = stddev

      # print the infomation
      if stddev != 0:
        print('\t\t"{0}" -> {1} +/- {2} ({3}%)'.format(metric_name, mean_value, stddev, stddev_perc), end='')
      else:
        print('\t\t"{0}" -> {1} +/- {2}'.format(metric_name, mean_value, stddev), end='')
      print(' over {0} observations'.format(len(values)))

    # add the Operating Point to the list
    my_op_list_model.ops.append(my_op_model)
  print('\n**************************************************')
  print('**************************************************')
  print('**************************************************')

  # compute the derived metrics
  sorted_metrics_to_add = sorted(metric_to_add.keys())
  for metric_name in sorted_metrics_to_add:
    op_utils.add_metric(my_op_list_model, metric_name, metric_to_add[metric_name])

  # print the list of Operating Points
  from contextlib import redirect_stdout
  with open(dse_data['outfile'], 'w') as f, redirect_stdout(f):
    op_utils.print_op_list_xml(my_op_list_model)
