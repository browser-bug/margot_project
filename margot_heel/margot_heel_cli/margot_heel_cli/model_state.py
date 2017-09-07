from . import model_op_field

import sys

class StateModel:
  """
  This class represents a state, which is an optimization problem.
  In particular, it is composed by the following information:
    - the name of the state
    - the rank definition
    - the list of constraints
    - if this is the starting state
  """

  rank_available_combination_types = {
    'GEOMETRIC' : 'FieldComposer::GEOMETRIC',
    'LINEAR'    : 'FieldComposer::LINEAR',
    'SIMPLE'    : 'FieldComposer::SIMPLE'
  }


  rank_available_directions = {
    'MAXIMIZE' : 'RankObjective::MAXIMIZE',
    'MINIMIZE' : 'RankObjective::MINIMIZE'
  }


  def __init__(self):
    self.name = ""
    self.rank_fields = []
    self.rank_type = 'GEOMETRIC'
    self.rank_direction = 'MINIMIZE'
    self.starting = True
    self.constraint_list = []


  def set_rank_direction(self, direction):
    """
    check to assign the correct direction to the rank
    """
    if direction.upper() == 'MAXIMIZE':
      self.rank_direction = 'MAXIMIZE'
      return

    if direction.upper() == 'MINIMIZE':
      self.rank_direction = 'MINIMIZE'
      return

    self.rank_direction = 'unknown (unreachable statement)'


  def __str__(self):
    """
    Dump the content of the class
    """

    dump_string = ''

    dump_string = '{0}\n\n  State specification'.format(dump_string)
    dump_string = '{0}\n    Name: {1}'.format(dump_string, self.name)

    # print the rank
    rank_string = '{0}'.format(self.rank_direction)
    for rank_field in self.rank_fields:

      if self.rank_type == 'GEOMETRIC':
        if rank_field.metric_name:
          param_name = rank_field.metric_name
        if rank_field.knob_name:
          param_name = rank_field.knob_name
        rank_string = '{0} ({1}^{2}) *'.format(rank_string, param_name, rank_field.coefficient)

      if self.rank_type == 'LINEAR':
        if rank_field.metric_name:
          param_name = rank_field.metric_name
        if rank_field.knob_name:
          param_name = rank_field.knob_name
        rank_string = '{0} {1}*{2} +'.format(rank_string, param_name, rank_field.coefficient)

      if self.rank_type == 'SIMPLE':
        if rank_field.metric_name:
          param_name = rank_field.metric_name
        if rank_field.knob_name:
          param_name = rank_field.knob_name
        rank_string = '{0} {1} '.format(rank_string, param_name)

    rank_string = rank_string[:-1]
    dump_string = '{0}\n    {1}'.format(dump_string, rank_string)
    dump_string = '{0}\n    subject to:'.format(dump_string)

    for index, constraint in enumerate(sorted(self.constraint_list, key=lambda x: int(x.priority))) :
      dump_string = '{0}\n      c{1}: "{2}"'.format(dump_string, index + 1, constraint.goal_ref)
      if constraint.target_metric:
        dump_string = '{0} on metric "{1}" with confidence {2}'.format(dump_string, constraint.target_metric, constraint.confidence)
      if constraint.target_knob:
        dump_string = '{0} on knob "{1}"'.format(dump_string, constraint.target_knob)

    if self.starting:
      dump_string = '{0}\n    This is the starting state!'.format(dump_string)


    return dump_string
