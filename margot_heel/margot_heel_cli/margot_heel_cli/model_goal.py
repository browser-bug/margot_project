class GoalModel:
  """
  This class represents a goal in the configuration file.
  """

  # the available comparison functions
  available_cfuns = {
    'GT' : 'margot::ComparisonFunctions::GREATER',
    'GE' : 'margot::ComparisonFunctions::GREATER_OR_EQUAL', 
    'LT' : 'margot::ComparisonFunctions::LESS',
    'LE' : 'margot::ComparisonFunctions::LESS_OR_EQUAL'
  }


  def __init__(self):

    # goal parameters
    self.name = 'a_generic_goal'
    self.cfun = ''
    self.metric_name_ref = ''
    self.parameter_name_ref = ''
    self.value = '0'





  def get_cfun_enumerator_value( self ):
    """
    Set the proper comparison function for C++
    """
    return self.available_cfuns[self.cfun]



  def get_c_goal_type( self, goal_type ):
    """
    Get the complete declaration of the goal Type
    """
    return 'margot::Goal< {1}, {0} >'.format(self.get_cfun_enumerator_value(), goal_type)


  def __str__(self):
    """
    dump the goal model
    """
    dump_string = '\n'

    dump_string = '{0}\n  Static goal specification'.format(dump_string)
    dump_string = '{0}\n    Name:            {1}'.format(dump_string, self.name)
    if self.metric_name_ref:
      dump_string = '{0}\n    Metric field:    {1}'.format(dump_string, self.metric_name_ref)
    if self.parameter_name_ref:
      dump_string = '{0}\n    Parameter field: {1}'.format(dump_string, self.parameter_name_ref)
    dump_string = '{0}\n    Comparison fun:  {1}'.format(dump_string, self.cfun)
    dump_string = '{0}\n    Target value:    {1}'.format(dump_string, self.value)
    return dump_string
