class KnobModel:
  """
  This class represents an exposed knob of the application, and it inclues:
    - the type of the variable in the application
    - the name of the variable in the application
    - the name of the parameter
  """

  available_var_types = ['int', 'float', 'double', 'long double']

  def __init__(self):
    self.var_name = ""
    self.var_type = ""
    self.name = ""


  def __str__(self):
    """
    Dump the content of the class
    """

    dump_string = ''

    dump_string = '{0}\n\n  Knob specification'.format(dump_string)
    dump_string = '{0}\n    Name:          {1}'.format(dump_string, self.name)
    dump_string = '{0}\n    Variable name: {1}'.format(dump_string, self.var_name)
    dump_string = '{0}\n    Variable type: {1}'.format(dump_string, self.var_type)

    return dump_string
