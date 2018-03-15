from .parser_utility import parse_xml_file
from .parser_utility import get_elements
from .parser_utility import get_parameter
from .model_op import OperatingPointModel
from .model_op_list import OperatingPointListModel
import sys
import csv
import os



def parse_op( op_xml_element, namespace = ''):

  # create the model
  my_op_model = OperatingPointModel()

  # get the reference to the knobs
  param_xml_element = get_elements(op_xml_element, 'parameters', namespace = namespace, unique = True, required = True)[0]

  # get all the params
  params_xml_element = get_elements( param_xml_element, 'parameter', namespace = namespace, required = True)
  for param_xml_element in params_xml_element:

    # parse the name and value
    knob_name = get_parameter(param_xml_element, 'name')
    knob_value = get_parameter(param_xml_element, 'value')

    # add it to the parameters
    my_op_model.knobs[knob_name] = knob_value

  # get the reference to the metrics
  metric_xml_element = get_elements(op_xml_element, 'system_metrics', namespace = namespace, unique = True, required = True)[0]

  # get all the metrics
  metrics_xml_element = get_elements( metric_xml_element, 'system_metric', namespace = namespace, )
  for metric_xml_element in metrics_xml_element:

    # parse the name and value
    metric_name = get_parameter(metric_xml_element, 'name')
    try:

      # parse the metric value
      metric_value = float(get_parameter(metric_xml_element, 'value'))

      # add it to the metrics
      my_op_model.metrics[metric_name] = metric_value
    except ValueError as err:
      print('[ERROR] Unable to convert a value of the metric "{0}" to a float value: value = {1}'.format(metric_name, get_parameter(metric_xml_element, 'value')))
      sys.exit(-1)

    # try to parse also the standard deviation
    try:
      metric_std = get_parameter(metric_xml_element, 'standard_dev', required = False)
      if metric_std:
        my_op_model.metrics_std[metric_name] = float(metric_std)
    except ValueError as err:
      pass # it's ok, the standard deviation is only optional

  # make sure that either all the metrics of the ops have a standard deviation
  # or that neither of them have it
  if my_op_model.metrics_std:
    if len(my_op_model.metrics_std) != len(my_op_model.metrics):
      for metric_name in my_op_model.metrics:
        try:
          value = my_op_model.metrics_std[metric_name]
        except KeyError as err:
          my_op_model.metrics_std[metric_name] = float(0)

  # get the reference to the data features (if any)
  feature_xml_element = get_elements(op_xml_element, 'features', namespace = namespace, unique = True, required = False)

  # get all the data features
  if (feature_xml_element):
    features_xml_element = get_elements( feature_xml_element[0], 'feature', namespace = namespace, required = True )
    for feature_xml_element in features_xml_element:

      # parse the name and value (make sure to lower the feature name)
      feature_name = get_parameter(feature_xml_element, 'name').lower()
      feature_value = get_parameter(feature_xml_element, 'value')

      # make sure that the data feature is numeric
      try:
        feature_value_num = int(feature_value)
      except ValueError:
        try:
          feature_value_num = float(feature_value)
        except ValueError:
          print('[ERROR] Unable to convert a value of the feature "{0}" to a numeric value: value = {1}'.format(metric_name, get_parameter(metric_xml_element, 'value')))
          sys.exit(-1)

      # add it to the parameters
      my_op_model.features[feature_name] = feature_value


  # return the model
  return my_op_model





def filter_op_list(op_list_model):
  """
  Create the translator for string knobs
  """

  # get the list of knobs and metrics sorted by name
  knob_names = sorted(op_list_model.ops[0].knobs.keys())
  metric_names = sorted(op_list_model.ops[0].metrics.keys())

  # here we want to check if the knobs are numeric or not
  string_knobs = []
  for knob_name in knob_names:

    # assume that it is a numeric parameter
    for op in op_list_model.ops:
      try:
        op.knobs[knob_name] = int(op.knobs[knob_name])
      except ValueError:
        try:
          op.knobs[knob_name] = float(op.knobs[knob_name])
        except ValueError:
          string_knobs.append(knob_name)
          break

  # now we must enumerate all the possible values for string parameters
  op_list_model.translator = {}
  op_list_model.reverse_translator = {}
  for knob_name in string_knobs:
    values = sorted(list(set([ op.knobs[knob_name] for op in op_list_model.ops ])))
    op_list_model.translator[knob_name] = {}
    op_list_model.reverse_translator[knob_name] = {}
    for index, value in enumerate(values):
      op_list_model.translator[knob_name][value] = index
      op_list_model.reverse_translator[knob_name][index] = value


  # eventually we convert the param values to numbers
  for knob_name in string_knobs:
    for op in op_list_model.ops:
      op.knobs[knob_name] = int(op_list_model.translator[knob_name][op.knobs[knob_name]])

  return op_list_model



def parse_ops_csv( csv_path, my_delimiter ):
  """
  Parse the Operating Point from the csv format
  """

  # parse the content of the file
  csvmatrix = []
  with open(csv_path, 'r') as csvfile:
    tracereader = csv.reader(csvfile, delimiter = my_delimiter)
    for row in tracereader:
      csvmatrix.append([x.strip() for x in row])

  if len(csvmatrix) < 2:
    return None


  # parse the header
  header_row = csvmatrix.pop(0)


  # compose the parameter, feature and metric name -> column index translator
  knob_dic = {}
  metric_dic = {}
  metric_std_dic = {}
  feature_dic = {}
  for col_index, header_token in enumerate(header_row):
    if header_token:
      if header_token[0] == '@':
        feature_dic[header_token[1:]] = col_index
      else:
        if header_token[0] == '#':
          knob_dic[header_token[1:]] = col_index
        else:
          if header_token.endswith('_standard_dev'):
            metric_std_dic[header_token.replace('_standard_dev', '')] = col_index
          else:
            metric_dic[header_token] = col_index

  knob_names = sorted(knob_dic.keys())
  metric_names = sorted(metric_dic.keys())
  feature_names = sorted(feature_dic.keys())

  # create the Operating Point list
  op_list_model = OperatingPointListModel()
  for csv_op_description in csvmatrix:

    # create the OP model
    op_model = OperatingPointModel()
    for feature_name in feature_names:
      try:
        op_model.features[feature_name] = int(csv_op_description[feature_dic[feature_name]])
      except ValueError:
        try:
          op_model.features[feature_name] = float(csv_op_description[feature_dic[feature_name]])
        except ValueError:
          print('[ERROR] Unable to convert "{0}" to a float value'.format(csv_op_description[feature_dic[feature_name]]))
          sys.exit(-1)

    for knob_name in knob_names:
      op_model.knobs[knob_name] = csv_op_description[knob_dic[knob_name]]
    try:
      for metric_name in metric_names:
        op_model.metrics[metric_name] = float(csv_op_description[metric_dic[metric_name]])
        if metric_std_dic:
          try:
            op_model.metrics_std[metric_name] = float(csv_op_description[metric_std_dic[metric_name]])
          except KeyError as e:
            op_model.metrics_std[metric_name] = float(0)

    except ValueError as e:
      print('[ERROR] Unable to convert "{0}" to a float value'.format(csv_op_description[metric_dic[metric_name]]))
      sys.exit(-1)

    # append it on the list
    op_list_model.ops.append(op_model)

  return filter_op_list(op_list_model)





def parse_ops_xml( op_file_path ):
  """
  Parse the list of Operating Points from an xml representation
  """

  # declare the model
  op_list_model = OperatingPointListModel()

  # parse the file
  xml_root, namespace = parse_xml_file(op_file_path)

  # parse the block name
  op_list_model.name = get_parameter(xml_root, 'block')

  # get all the Operating Points
  op_xml_elements = get_elements(xml_root, 'point', required = True, namespace = namespace)

  # loop over the Operating Points
  for op_xml_element in op_xml_elements:

    # parse the Operating Point
    op_model = parse_op(op_xml_element, namespace = namespace)

    # add it to the list
    op_list_model.ops.append(op_model)

  # handles any string parameters left
  op_list_model = filter_op_list(op_list_model)

  # parse all the knobs dictionaries
  dic_xml_elements = get_elements(xml_root, 'dictionary', namespace = namespace)

  # loop over them
  for dic_xml_element in dic_xml_elements:

    # get the name of the parameters
    param_name = get_parameter(dic_xml_element, 'param_name')

    # get the possible values
    values_xml_elements = get_elements(dic_xml_element, 'value', namespace = namespace, required = True )

    # loop over them
    op_list_model.translator[param_name] = {}
    op_list_model.reverse_translator[param_name] = {}
    for values_xml_element in values_xml_elements:
      string_value = get_parameter(values_xml_element, 'string')
      numeric_value = get_parameter(values_xml_element, 'numeric')
      op_list_model.translator[param_name][string_value] = numeric_value
      op_list_model.reverse_translator[param_name][numeric_value] = string_value

  return op_list_model



def parse_multi_xml_list( op_file_path ):
  """
  Parse the list of oplists fields (aka number of test run) from an xml representation
  """
  #dict with run as key to maintain reference with id
  multilist={}
  # parse the file
  xml_root, namespace = parse_xml_file(op_file_path)

  # get all the Operating Points
  oplists_xml_elements = get_elements(xml_root, 'oplists', required = True, namespace = namespace)
  # loop over the Operating Points
  for oplists_xml_element in oplists_xml_elements:
    # parse the Operating Point
    op_model = parse_single_oplist_collection(oplists_xml_element, namespace = namespace)

    # add it to the list
    multilist[get_parameter(oplists_xml_element, 'id')]=op_model

  return multilist


def parse_folder_oplist_collection( pattern_name):
  """
  parse the oplists of a folder to obtain a list of oplists
  """
  list_of_oplist = []
  files = [f for f in os.listdir('.') if os.path.isfile(f)]
  for f in files:
    if pattern_name in f:
      op = parse_ops_xml(f)
      list_of_oplist.append(op)
  return list_of_oplist



def parse_single_oplist_collection( oplists_xml_element, namespace ):
  """
  Parse the list of oplists (aka the single run, all the application run parallelly) from an xml representation
  """
  single_op_list=[]
  # get all the Operating Points
  oplist_xml_elements = get_elements(oplists_xml_element, 'oplist', required = True, namespace = namespace)
  # loop over the Operating Points
  for oplist_xml_element in oplist_xml_elements:

    # parse the Operating Point
    op_model = parse_ops_xml(oplist_xml_element.text)#actually need this for namespace and crap parsing, but should return a single op oplist
    # add it to the list
    single_op_list.append(op_model)

  return single_op_list


def parse_params_multiapp_run( xml_source):
  dict_params={}
  xml_root, namespace = parse_xml_file(xml_source)
  params_xml_elements = get_elements(xml_root, 'parameters', required = True, namespace = namespace)

  # loop over the Operating Points
  for params_list in params_xml_elements:

    # parse the Operating Point
    temp={}
    parlist = get_elements(params_list, 'parameter',required = True)
    for par in parlist:
      temp[get_parameter(par,'name')]=get_parameter(par,'value')
    dict_params[get_parameter(params_list, 'id')]=temp

  return dict_params
