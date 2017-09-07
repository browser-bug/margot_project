from . import model_parameter
import sys


class FieldAdaptorModel:
  """
  This class represents a field adaptor
  """


  def __init__( self ):
    self.monitor_name = ''
    self.knob_name = ''
    self.metric_name = ''
    self.inertia = 1
  	


  def __str__( self ):

    # initial string
    dump_string = '\n'
    dump_string = '{0}\n  Field adaptor specification:'.format(dump_string)
    if self.metric_name:
      dump_string = '{0}\n    Metric name:  {1}'.format(dump_string, self.metric_name)
    if self.knob_name:
      dump_string = '{0}\n    Knob name:    {1}'.format(dump_string, self.knob_name)
    dump_string = '{0}\n    Monitor name: {1}'.format(dump_string, self.monitor_name)
    dump_string = '{0}\n    Inertia:      {1}'.format(dump_string, self.inertia)
    return dump_string