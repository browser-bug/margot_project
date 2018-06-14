import sys

class InputData:
  """
  This class represents a single input data type contained in the datasets
    - the input data name
    - the input data type
    - the input data values
  """
  
  available_var_types = ['int', 'float', 'double', 'string']

  def __init__(self):

    self.name = ''
    self.type = ''
    self.values = {}

  def __str__(self):
    """
    dump the input data model
    """
    dump_string = '\n'
    dump_string = '{0}\n  Input data specification'.format(dump_string)
    dump_string = '{0}\n    Name:       {1}'.format(dump_string, self.name)
    dump_string = '{0}\n    Type:       {1}'.format(dump_string, self.type)
    dump_string = '{0}\n    Values: {1}'.format(dump_string, self.values)

    return dump_string
