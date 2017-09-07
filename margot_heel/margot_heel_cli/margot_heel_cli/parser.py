from __future__ import print_function

from .parser_utility import parse_xml_file
from .parser_utility import get_elements
from .parser_utility import get_parameter
from .parse_block import parse_block_xml
from .parse_op import parse_ops_xml,parse_ops_csv,parse_multi_xml_list,parse_params_multiapp_run,parse_folder_oplist_collection
from .model_op_list import OperatingPointListModel
from .op_utils import process_multiapp_outputs
import sys

class Parser:
  """
  This class drives the parsing phase.
  It takes as input the path to the xml file and provides as output all the
  models of the application
  """


  def __init__(self):
    """
    Initialize a parser
    """

    self.block_models = {}  # key -> name of the block, value -> its model
    self.ops = {} # The Operating Point models


  def parse_margot(self, path_margot):
    """
    Parse the margot configuration file
    """

    # parse the main XML file
    xml_root, namespace = parse_xml_file(path_margot)


    # get all the elements that defines a block
    xml_blocks = get_elements(xml_root, "block", namespace = namespace, exclusive = True)

    # loop over them
    for xml_block in xml_blocks:

      # parse it
      block_model = parse_block_xml(xml_block, namespace = namespace )

      # postprocess it
      block_model.postprocess()
      print(block_model)

      # append the model in the list
      self.block_models[block_model.block_name] = block_model


  def parse_ops(self, path_ops_files):
    """
    Parse the lists of Operating Points
    """

    # loop through the list of Operating Points
    for path_op_file in path_ops_files:

      # parse the file
      op_list_model = parse_ops_xml(path_op_file)

      # add them to the list of Operating Points list
      self.ops[op_list_model.name] = op_list_model



  def parse_multi_app(self, xml_source, avgmetrics, param_xml):
    """
    gets the list of oplists from the xml
    """
    self.multi_list = parse_multi_xml_list(xml_source)
    self.ops = OperatingPointListModel ()
    self.ops.name= "prova"
    paramdict= parse_params_multiapp_run(param_xml)
    for key, list_of_oplist in self.multi_list.items():
      temp = process_multiapp_outputs(list_of_oplist,avgmetrics.split(","))
      #add parameters from the xml
      temp.knobs=paramdict[key]
      #add tmp to oplist
      self.ops.ops.append(temp)



  def process_folder_lists(self,varname,pattern):
    """
    gets the list of points to be plotted. already does the normalization
    """
    plotlist =[]
    self.list_of_oplist = parse_folder_oplist_collection(pattern)
    for oplist in self.list_of_oplist:
      for metric in oplist.ops[0].metrics:
        if metric == varname:
          plotlist.append(float(oplist.ops[0].metrics[metric]))

    #elaborate data
    max_value = max(plotlist)
    sum_value = sum(float(max_value)/float(i) for i in plotlist)

    plotdic = {}
    for i in plotlist:
      if i in plotdic.keys():
        plotdic[i]=plotdic[i]+((float(max_value)/float(i))/sum_value)
      else:
        plotdic[i]=((float(max_value)/float(i))/sum_value)
    return plotdic



  def postprocess(self):
    """
    This methods performs two actions for each block:
      - deletes all the list of Operating Points that are not managed by margot
      - if an op list is provided, then checks if it is consistent with the margot description
    """

    # find all the elements that sould be eliminated from the op list
    op_list_to_be_deleted = []
    for block_name in self.ops:

      # avoid to process an useless op list
      try:
        tmp = self.block_models[block_name]
      except KeyError:
        op_list_to_be_deleted.append(block_name)
        print('[CONSISTENCY WARNING] The block "{0}" is not managed!'.format(block_name))
        print('                      ITS OP LIST WILL BE IGNORED!')
        continue

      # otherwise, check that all the ops have the same structure
      first_op = self.ops[block_name].ops[0]

      # loop for the other ops
      for op_model in self.ops[block_name].ops:
        similar = first_op.similar(op_model)
        if not similar:
          print('[CONSISTENCY WARNING] The Operaint Point list for the block "{0}" is not self-consistent!'.format(block_name))
          print('                      THE OP LIST WILL BE IGNORED!')
          op_list_to_be_deleted.append(block_name)
          break


    # remove the elements
    for name in op_list_to_be_deleted:
      del self.ops[name]


    # loop over all the managed blocks
    for block_name in self.block_models:

      # check if they have a list of Operating Points
      try:

        # get the name of the knobs from the state
        knob_names = sorted([x.name for x in self.block_models[block_name].software_knobs])

        # get the name of the metrics from the state
        metric_names = sorted([x.name for x in self.block_models[block_name].metrics])

        # extract the name of knobs from the op
        op_knob_names = sorted(self.ops[block_name].ops[0].knobs.keys())

        # extract the name of metrics from op
        op_metric_names = sorted(self.ops[block_name].ops[0].metrics.keys())

        if not (knob_names == op_knob_names and metric_names == op_metric_names):
          print('[CONSISTENCY ERROR] Mismatch between the Operating List and the requirements for the block "{0}"'.format(block_name))
          sys.exit(-1)

      except KeyError:

        # there is no Operating Points... but now it is ok
        pass
