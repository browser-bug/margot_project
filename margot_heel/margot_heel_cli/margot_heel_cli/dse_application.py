import os
import inspect
import sys
import itertools

#################################
###### Import argo library
#################################
from . import parser_utility as p


#################################
###### The application real class
#################################


class Application:
  """
  This class represent the application that must be profiled

  Arguments:
    - name        (str)   -> The name of the application
    - log_file    (str)   -> The name of the logfile produced from the application
    - block       (str)   -> The name of the block involved in the DSE
    - flags       (list)  -> The list of application-wide flags
    - dependencies(list)  -> The list of file dependencies for the executable
    - knob_values (dic)   -> The dictionary of the knobs to explore
                             key   (str)   -> knob name
                             value (list)  -> the list of all the values of the knob
    - knob_flags  (dic)   -> The dictionary of the knobs to explore
                             key   (str)   -> knob name
                             value (str)   -> the flag used to drive the dse
    - metrics     (dic)   -> The dictionary of the metric to read from the execution trace
                             key   (str)   -> metric name
                             value (str)   -> the name of the field in the log file
    - compute     (dic)   -> The dictionary of the metric that derives from the observed ones
                             key   (str)   -> metric name
                             value (str)   -> the formula that should be used to compute the metric
  """


  def __init__(self, xml_path):
    """
    Parse the XML file and extracts the required information
    """

    # open the XML file
    xml_root, namespace = p.parse_xml_file(xml_path)

    # initialize application-wide configurations
    self.block = p.get_parameter(xml_root, 'block')
    self.name = p.get_parameter(xml_root, 'name', required = False)
    if not self.name:
      self.name = 'generic-app'
    self.log_file = p.get_parameter(xml_root, 'out_file', required = False)
    if not self.log_file:
      self.log_file = '{0}.log'.format(self.block)

    # get all the flags parameters
    self.flags = {}
    temp_flags = []
    flags_xml = p.get_elements(xml_root, 'flag', namespace = namespace)
    for flag_xml in flags_xml:
      flag_value = p.get_parameter(flag_xml, 'value')
      temp_flags.append(flag_value)

    # get all the knob description
    self.knob_values = {}
    self.knob_flags = {}
    knobs_xml = p.get_elements(xml_root, 'parameter', namespace = namespace, required = True)
    for knob_xml in knobs_xml:
      knob_values = []
      knob_type = p.get_parameter(knob_xml, 'type', prefixed_values = ['int', 'float', 'enum'])
      knob_name = p.get_parameter(knob_xml, 'name')
      if knob_type == 'int' or knob_type == 'float':
        if knob_type == 'float':
          actual_type = float
        else:
          actual_type = int
        start_value = p.get_parameter(knob_xml, 'start_value', my_target_type = actual_type)
        stop_value = p.get_parameter(knob_xml, 'stop_value', my_target_type = actual_type)
        step_value = p.get_parameter(knob_xml, 'step', my_target_type = actual_type)
        index = start_value
        while( index <= stop_value):
          knob_values.append(str(index))
          index += step_value
      else:
        if knob_type != 'enum':
          print('[DEFENSIVE ERROR] This should not happens! humm... 3324231412')
          sys.exit(-1)
        values_str = p.get_parameter(knob_xml, 'values')
        knob_values = [ x for x in values_str.split(' ') if x]
      self.knob_values[knob_name] = knob_values
      self.knob_flags[knob_name] = p.get_parameter(knob_xml, 'flag')

    #get all the input_groups values
    self.input_groups = {}
    groups_xml = p.get_elements(xml_root, 'input_group', namespace = namespace, required = False)
    input_groups_list = []
    for group_xml in groups_xml:
      input_block_list = []
      for input_block in group_xml:
        data_list = []
        data_list_out = []
        for data in input_block:
          item_values = []
          item_type = p.get_parameter(data, 'type', prefixed_values = ['int', 'float', 'enum'])
          if item_type == 'int' or item_type == 'float':
            if item_type == 'float':
              actual_type = float
            else:
              actual_type = int
            start_value = p.get_parameter(data, 'start_value', my_target_type = actual_type)
            stop_value = p.get_parameter(data, 'stop_value', my_target_type = actual_type)
            step_value = p.get_parameter(data, 'step', my_target_type = actual_type)
            index = start_value
            while( index <= stop_value):
              item_values.append(str(index))
              index += step_value
          else:
            if item_type != 'enum':
              print('[DEFENSIVE ERROR] This should not happen! humm... 3324231412')
              sys.exit(-1)
            values_str = p.get_parameter(data, 'values')
            item_values = [ x for x in values_str.split(' ') if x]
          temp = []
          for value in item_values:
            temp.append('{0} {1}'.format(p.get_parameter(data, 'flag'),value))
          data_list.append(temp)
        #postprocess here the data read
        #cartesian product of block items
        for value in itertools.product(*data_list):
          input_block_list.append(value)
      #and append all the blocks in the "group list"
      input_groups_list.append(input_block_list)

    #generate cartesian product of the list of input groups.
    for index, element in enumerate (itertools.product(*input_groups_list)):
      self.input_groups[index]=[]
      for subelement in element:
        self.input_groups[index].extend(subelement)

    if (len(self.input_groups) == 0):
      self.input_groups[0] = []
    for input_key,input_value in self.input_groups.items():
      self.flags[input_key] = input_value
      (self.flags[input_key])[0:0]=(temp_flags)




    # get all the observed metric description
    self.metrics = {}
    metrics_xml = p.get_elements(xml_root, 'metric', namespace = namespace)
    for metric_xml in metrics_xml:
      metric_name = p.get_parameter(metric_xml, 'name')
      metric_field = p.get_parameter(metric_xml, 'field')
      self.metrics[metric_name] = metric_field

    # get all the metric description that should be computed
    self.compute = {}
    computed_metrics_xml = p.get_elements(xml_root, 'compute', namespace = namespace)
    for computed_metric_xml in computed_metrics_xml:
      metric_name = p.get_parameter(computed_metric_xml, 'name')
      metric_field = p.get_parameter(computed_metric_xml, 'formula')
      self.compute[metric_name] = metric_field

    # get all the metric description that should be computed
    self.dependencies = []
    dependencies_xml = p.get_elements(xml_root, 'dep', namespace = namespace)
    for dependency in dependencies_xml:
      dep_path = p.get_parameter(dependency, 'path')
      self.dependencies.append(os.path.join(os.getcwd(), dep_path))



  def __str__(self):
    string = '***** {0} *****\n'.format(self.name.upper())
    string = '{0}\tBlock:    "{1}"\n'.format(string, self.block)
    string = '{0}\tLog file: "{1}"\n'.format(string, self.log_file)
    string = '{0}\tFlags:    "{1}"\n'.format(string, '", "'.join(self.flags))
    string = '{0}\tKnobs:\n'.format(string)
    for knob_name in self.knob_values:
      values = '"{0}"'.format('", "'.join(self.knob_values[knob_name]))
      string = '{0}\t\t{1} ("{3}") : {2}\n'.format(string, knob_name, values, self.knob_flags[knob_name])
    string = '{0}\tObserved Metrics:\n'.format(string)
    for metric_name in self.metrics:
      string = '{0}\t\t{1} <- "{2}"\n'.format(string, metric_name, self.metrics[metric_name])
    string = '{0}\tComputed Metrics:\n'.format(string)
    for metric_name in self.compute:
      string = '{0}\t\t{1} <- "{2}"\n'.format(string, metric_name, self.compute[metric_name])
    return string



  def get_translator_dictionaries(self):
    knob_names = sorted(self.knob_values.keys())
    translator_eval_string = 'translator = {'
    reverse_translator_eval_string = 'reverse_translator = {'
    terms = []
    terms_reverse = []
    for knob_name in knob_names:
      numeric = True
      for knob_value in self.knob_values[knob_name]:
        try:
          float(knob_value)
        except:
          numeric = False
          break
      if not numeric:
        sorted_values = sorted(self.knob_values[knob_name])
        for counter, value in enumerate(sorted_values):
          terms.append('"{0}":{1}'.format(value, counter))
          terms_reverse.append('{0}:"{1}"'.format(counter, value))
    translator_eval_string = '{0}{1}}}'.format(translator_eval_string, ','.join(terms))
    reverse_translator_eval_string = '{0}{1}}}'.format(reverse_translator_eval_string, ','.join(terms_reverse))
    return translator_eval_string, reverse_translator_eval_string
