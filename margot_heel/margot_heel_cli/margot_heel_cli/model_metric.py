class MetricModel:
  """
  This class represents a metric of the Operating Point
  """

  available_types = ['int', 'float', 'double']

  def __init__( self ):

    self.name = ''
    self.type = ''
    self.distribution = True


  def __str__(self):
    """
    Dump the content of the class
    """

    dump_string = ''

    dump_string = '{0}\n\n  Metric specification'.format(dump_string)
    dump_string = '{0}\n    Name:              {1}'.format(dump_string, self.name)
    dump_string = '{0}\n    Type:              {1}'.format(dump_string, self.type)
    dump_string = '{0}\n    Is distribution:   {1}'.format(dump_string, self.distribution)

    return dump_string
