import sys
from .model_op import OperatingPointModel

class OperatingPointListModel:
  """
  This class represents a list of Operating Points:
    - The list of OperatingPointModel
    - The dictionary used to translate the Operating Points string -> int
    - The reverse dictionary int -> string
    - The name of the block
  """

  def __init__(self):
    self.ops = []
    self.translator = {}
    self.reverse_translator = {}
    self.name = 'elaboration'

  def get_op (self, knob_map):
    """
    knob map must have the same format as in operatin point model:
    key == knob name
    value = knob value

    """
    if len(self.ops) == 0:
      print ("attempt to get an OP from an empty oplist")
      sys.exit(-1)

    if len(self.ops[0].knobs) != len(knob_map):
      print ("attempt to get an OP with a wrong knob map")
      sys.exit(-1)
    op_to_return = OperatingPointModel()
    for op in self.ops:
      flag=True
      for knob in op.knobs:
        if float(op.knobs[knob]) != float(knob_map[knob]):
          flag=False
      if flag:
        if not op_to_return.knobs:
          op_to_return=op
        else:
          print ("oplist contains two points with the same knobs")
          sys.exit(-1)
    return op_to_return

  def get_op_knobs_as_string (self, op):
    knob_map = {}
    for knob in op.knobs:
      if (knob in self.reverse_translator.keys()):
        knob_map[knob] = self.reverse_translator[knob][str(op.knobs[knob])]
      else:
        knob_map[knob] = str(op.knobs[knob])

    if (len (knob_map) > 0):
      return knob_map
    else:
      print ("ERROR: the op has no knobs to convert in string")
      sys.exit(-1)


  def get_c_type( self, type_name_suffix ):

    # get a reference to the first op
    first_op = ops[0]

    # check if the metrics are distributions
    metric_type_prefix = 'margot::Data'
    if (first_op.metrics_std):
      metric_type_prefix = 'margot::Distribution'

    # get the number of fields
    number_knobs = len(first_op.knobs.keys())
    number_metrics = len(firs_op.metrics.keys())

    # check if knobs might be integer
    knob_type = 'int'
    for op in self.ops:
      for knob_name in op.knobs:
        try:
          value = int(knob_name)
        except ValueError as err:
          knob_type = 'float'
          break

    # check if metrics might be integer
    metric_type = 'int'
    for op in self.ops:
      for metric_name in op.metrics:
        try:
          value = int(metric_name)
        except ValueError as err:
          metric_type = 'float'
          break

    # now we can compose the Operating Points type
    geometry_knobs = 'margot::OperatingPointSegment< {0}, margot::Data<{1}> >'.format(number_knobs, knob_type)
    geometry_metrics = 'margot::OperatingPointSegment< {0}, {1}<{2}> >'.format(number_metrics, metric_type_prefix, metric_type)
    return 'using {0}OperatingPoints = margot::OperatingPoint< {1}, {2} >'.format(type_name_suffix, geometry_knobs, geometry_metrics)


