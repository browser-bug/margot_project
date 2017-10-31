import sys
import copy
import math


def get_field_value( op_model, field, std_dev_coef = 0 ):
  """
  Extract a paramter or a metric from a field
  of the Operating Points
  """
  # extract the value assuming that it is a metric
  try:
    metric_value = float(op_model.metrics[field])
    if op_model.metrics_std:
      metric_value += float(op_model.metrics_std[field]) * std_dev_coef
    return metric_value
  except KeyError:

    # extract the value assuming it is a knob
    try:
      return float(op_model.knobs[field])
    except KeyError:


      # extract the value assuming it is a data feature
      try:
        return float(op_model.features[field])
      except KeyError:

        # real error, exit
        print('[LOGIC ERROR] Unable to retrieve the field "{0}"'.format(field))
        print('\t- available metrics: "{0}"'.format('", "'.join(op_model.metrics.keys())))
        print('\t- available features: "{0}"'.format('", "'.join(op_model.features.keys())))
        print('\t- available knobs: "{0}"'.format('", "'.join(op_model.knobs.keys())))
        sys.exit(-1)


def op_equals( op_1, op_2 ):
  """
  This method tests whether the op_1 is equal to the op_2.
  Two Operating Points are equal if they have the same parameters
  and the same data features
  """

  # if they have a different number of knobs, they are not the same
  if len(op_1.knobs) != len(op_2.knobs):
    return False

  # loop over the ops of op1
  for param_name in op_1.knobs:

    # get the value of op1
    op_1_value = op_1.knobs[param_name]

    # get the value of op2
    try:
      op_2_value = op_2.knobs[param_name]
    except KeyError:
      # if they have different knob, they are not the same
      return False

    # if they have different values, they are not the same
    if not op_1_value == op_2_value:
      return False

  # they have the same software knob, we should check for the data features
  if len(op_1.features) != len(op_2.features):
    return False

  # loop over the ops of op1
  for feature_name in op_1.features:

    # get the value of op1
    op_1_value = op_1.features[feature_name]

    # get the value of op2
    try:
      op_2_value = op_2.features[feature_name]
    except KeyError:
      # if they have different knob, they are not the same
      return False

    # if they have different values, they are not the same
    if not op_1_value == op_2_value:
      return False

  # otherwise they are the same Operating Point
  return True



def dominate( op1, op2, directions ):
  """
  This function returns True iff op1 dominate op2 according to directions
  This definition make sense only if they have the same data features
  """

  # loop over the ops of op1
  for feature_name in op1.features:

    # get the value of op1
    op1_value = op1.features[feature_name]

    # get the value of op2
    try:
      op_2_value = op2.features[feature_name]
    except KeyError:
      # if they have different features, they cannot be compared
      return False

    # if they have different values, they cannot be compared
    if not op1_value == op_2_value:
      return False

  # reached this point, they refer to the same data feature

  # loop over the fields of interest
  for field_name in directions:

    # compare them
    if directions[field_name]:

      # get the values
      value_op_1 = get_field_value(op1, field_name, -1)
      value_op_2 = get_field_value(op2, field_name, -1)
      if value_op_1 < value_op_2:
        return False
    else:
      value_op_1 = get_field_value(op1, field_name, 1)
      value_op_2 = get_field_value(op2, field_name, 1)
      if value_op_1 > value_op_2:
        return False

  # if we reach this point, op1 dominate op2
  return True


def print_op( op ):
  """
  Pretty print of an Operating Point
  """
  print(str(op))


def pareto_filter( op_list, directions ):
  """
  Apply a Pareto filter on the given list of Operating Points
    - op_list is the list of the Operating Points
    - directions is a dictionary that define the Pareto optimality
      key -> metric name
      value -> bool, true if higher is better
  """

  # dominated_ops
  dominant_ops = []

  # loop over the list of Operating Points
  for op in op_list.ops:

    # get a copy of the evaluated op
    op_evaluated = copy.deepcopy(op)

    # assume that it is not dominated
    is_dominated = False

    # loop over the Operating Points
    for target_op in op_list.ops:

      # make sure to not consider the evaluated op
      if not op_equals(op_evaluated, target_op):

        # check if the target op is domaniated
        if dominate(target_op, op_evaluated, directions):
          is_dominated = True
          break

    if not is_dominated:
      dominant_ops.append(op_evaluated)

  # replece it in the op model
  op_list.ops = dominant_ops

  # return the list of dominant_ops
  return op_list





def plot_op_list( op_list, x_metric, y_metric, color_metric = '' ):
  """
  Generate a gnuplot script to show the Operating Points and
  print it in the standard output
  """


  # get the x-values
  x_values = [ get_field_value(op, x_metric) for op in op_list.ops ]

  # get the y-values
  y_values = [ get_field_value(op, y_metric) for op in op_list.ops ]

  # get the z-values
  if color_metric:
    z_values = [ get_field_value(op, color_metric) for op in op_list.ops ]
  else:
    z_values = [ 0 for op in op_list.ops ]


  # write the gnulot script
  print('#!/usr/bin/gnuplot')
  print('reset')
  print('set terminal pdf size 7,5 enhanced font "Verdana,20"')
  #print('set output "oplist.svg"')
  print('unset key')
  print('set style line 11 lc rgb "#808080" lt 1')
  print('set border 3 back ls 11')
  print('set tics nomirror out scale 0.75')
  print('set size square')
  print('set style circle radius screen 0.005')
  print('set palette negative defined ( \\')
  print('\t0 "#D53E4F",\\')
  print('\t1 "#F46D43",\\')
  print('\t2 "#FDAE61",\\')
  print('\t3 "#FEE08B",\\')
  print('\t4 "#E6F598",\\')
  print('\t5 "#ABDDA4",\\')
  print('\t6 "#66C2A5",\\')
  print('\t7 "#3288BD" )')
  print('set style fill transparent solid 0.8 noborder')


  # handle the x-axis
  if x_metric in op_list.translator:
    tics = ['"{0}" {1}'.format(str_value,op_list.translator[x_metric][str_value]) for str_value in op_list.translator[x_metric]]
    print('set xrange [{0}:{1}]'.format(-(len(tics)*0.1), len(tics)-1+(len(tics)*0.1)))
    print('set xtics 0,1,{0}'.format(len(tics)-1))
    for tic in tics:
      print('set xtics add ({0})'.format(tic))
  else:
    x_min = min(x_values)
    x_max = max(x_values)
    x_delta = abs(max(abs(x_min), abs(x_max))*0.1)
    x_max = x_max + x_delta
    x_min = x_min - x_delta
    print('set xrange [{0}:{1}]'.format(x_min, x_max))

  # handle the y-axis
  if y_metric in op_list.translator:
    tics = ['"{0}" {1}'.format(str_value,op_list.translator[y_metric][str_value]) for str_value in op_list.translator[y_metric]]
    print('set yrange [{0}:{1}]'.format(-(len(tics)*0.1), len(tics)-1+(len(tics)*0.1)))
    print('set ytics -1,1,{0}'.format(len(tics)-1))
    for tic in tics:
      print('set ytics add ({0})'.format(tic))
  else:
    y_min = min(y_values)
    y_max = max(y_values)
    y_delta = abs(max(abs(y_min), abs(y_max))*0.1)
    y_min = y_min - y_delta
    y_max = y_max + y_delta
    print('set yrange [{0}:{1}]'.format(y_min, y_max))

  # handle the cb-tics
  if color_metric in op_list.translator:
    tics = ['"{0}" {1}'.format(str_value,op_list.translator[color_metric][str_value]) for str_value in op_list.translator[color_metric]]
    print('set cbtics 0,1,{0}'.format(len(tics)))
    for tic in tics:
      print('set cbtics add ({0})'.format(tic))



  print('set xlabel "{0}"'.format(x_metric.capitalize()))
  print('set ylabel "{0}"'.format(y_metric.capitalize()))
  print('set cblabel "{0}"'.format(color_metric.capitalize()))
  print('plot "-" u 1:2:3 w circles lc palette')


  # print the data file
  for i in range(len(x_values)):
    print('{0} {1} {2}'.format(x_values[i], y_values[i], z_values[i]))
  print('e')




def print_op_list_xml( op_list ):

  # print the header
  print('<?xml version="1.0" encoding="UTF-8"?>')
  print('<points xmlns="http://www.multicube.eu/" version="1.3" block="{0}">'.format(op_list.name))

  # print the dictionaries
  for knob_name in op_list.translator:
    print('\t<dictionary param_name="{0}">'.format(knob_name))
    for knob_vale_string in op_list.translator[knob_name]:
      print('\t\t<value string="{0}" numeric="{1}" />'.format(knob_vale_string, op_list.translator[knob_name][knob_vale_string]))
    print('\t</dictionary>')

  # print the actul list
  for op in op_list.ops:

    print('\t<point>')

    # print the knobs
    if op.knobs:
      print('\t\t<parameters>')
      for knob_name in op.knobs:
        print('\t\t\t<parameter name="{0}" value="{1}"/>'.format(knob_name, op.knobs[knob_name]))
      print('\t\t</parameters>')

    # print the data features
    if op.features:
      print('\t\t<features>')
      for feature_name in op.features:
        print('\t\t\t<feature name="{0}" value="{1}"/>'.format(feature_name, op.features[feature_name]))
      print('\t\t</features>')

    # print the metrics
    if op.metrics:
      print('\t\t<system_metrics>')
      for metric_name in op.metrics:
        if not op.metrics_std:
          print('\t\t\t<system_metric name="{0}" value="{1}"/>'.format(metric_name, op.metrics[metric_name]))
        else:
          print('\t\t\t<system_metric name="{0}" value="{1}" standard_dev="{2}"/>'.format(metric_name, op.metrics[metric_name], op.metrics_std[metric_name]))
      print('\t\t</system_metrics>')

    print('\t</point>')

  print('</points>')



def print_op_list_csv( op_list ):

  # check if there is any op
  if not op_list.ops:
    return

  # get the list of knobs, features and metrics
  knob_names = sorted(op_list.ops[0].knobs.keys())
  metric_names = op_list.ops[0].metrics.keys()
  feature_names = sorted(op_list.ops[0].features.keys())
  only_metric_names = sorted(list(metric_names))
  if op_list.ops[0].metrics_std:
    metric_names.extend( ['{0}_standard_dev'.format(x) for x in metric_names] )
  metric_names = sorted(metric_names)


  # revert the translator dictionary
  reversed_dictionary = {}
  for k_n in op_list.translator:
    reversed_dictionary[k_n] = {}
    for string_value in op_list.translator[k_n]:
      reversed_dictionary[k_n][op_list.translator[k_n][string_value]] = string_value


  # add the prefix for the knobs
  elements = ['#{0}'.format(x) for x in knob_names]

  # add the prefix for the data features
  elements.extend(['@{0}'.format(x) for x in feature_names])

  # compose the header
  elements.extend(metric_names)

  # print the header
  print(','.join(elements))

  # print the actual list
  for op in op_list.ops:
    values = []

    # append the knobs
    for n in knob_names:
      if n in reversed_dictionary:
        values.append(str(reversed_dictionary[n][str(op.knobs[n])]))
      else:
        values.append(str(op.knobs[n]))

    # append the data features
    for n in feature_names:
      values.append(str(op.features[n]))

    # append the metrics
    for n in only_metric_names:
      values.append(str(op.metrics[n]))
      if op.metrics_std:
        values.append(str(op.metrics_std[n]))

    print(','.join(values))


def process_multiapp_outputs(multiop_list, avgmetrics):
  baselist = multiop_list.pop()
  for oplist in multiop_list:
    baselist.ops[0].add(oplist.ops[0])
  for metric in avgmetrics:
    baselist.ops[0].avg(len(multiop_list)+1, metric)
  return baselist.ops[0]


def print_distribution(plotdic):
  print("set xtic auto                          # set xtics automatically")
  print("set ytic auto                          # set ytics automatically")

  print("set terminal postscript eps enhanced color font ',20'")

  # Line style for axes and grid
  print("       set style line 80 lt 1 lw 2")
  print("       set style line 80 lc rgb \"#000000\" lw 2")
  print("       set style line 81 lt 3 lw 1  # dashed")
  print("       set style line 81 lt rgb \"#808080\" lw 2  # grey")


  # Beautify the grid and the axis
  print("       set grid back linestyle 81")
  print("       set border 3 back linestyle 80")
  print("       set xtics nomirror")
  print("       set ytics nomirror")

  # Line styles for the data traces
  print("       set style line 1 lt rgb \"#1f78b4\" lw 4 pt 7")
  print("       set datafile missing \"N/A\"")
  print("       set yrange [0:1.2]")

  print("       plot \"-\" u 1:2 title \"density\" with linespoints ls 1")
  print("0 0")
  previous = 0
  for key in sorted(plotdic.keys()):
    print ('{0} {1}'.format(float(key),plotdic[key]+previous))
    previous = plotdic[key]+previous
  print("e")



def add_metric(op_list_model, new_metric_name, formula):

  # loop over the op model
  for op_model in op_list_model.ops:


    # get a copy of the formula
    op_formula = str(formula)

    # if the the metrics are a distribution, we can't combine them
    if (op_model.metrics_std):
      print('[LOGIC ERROR] Unable to add the metric "{0}", unable to handle the standard deviations!'.format(new_metric_name))
      sys.exit(-1)


    # get the fields of the Operating Point
    fields = list(op_model.metrics.keys())
    fields.extend( list(op_model.knobs.keys()) )

    # make them unique and sorted by lenght
    fields = sorted(list(set(fields)), key = len, reverse = True)

    # replace each factor
    for field in fields:
      if field in formula:
        op_formula = op_formula.replace(field, str(get_field_value(op_model, field)))

    # evaluate the expression and add the field
    try:
      op_model.metrics[new_metric_name] = float(eval(op_formula))
    except:
      print('[RUNTIME ERROR] Unable to add the metric "{0}", NaN value found!'.format(new_metric_name))
      print('\t- original  formula: "{0}"'.format(formula))
      print('\t- evaluated formula: "{0}"'.format(op_formula))
      sys.exit(-1)
