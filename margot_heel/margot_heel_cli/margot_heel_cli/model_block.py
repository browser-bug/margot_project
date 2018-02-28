import sys

class BlockModel:
  """
  This class represents the model of a tunable block of code
  """

  def __init__(self, block_name = 'no_name'):

    # the name of the block itself
    self.block_name = block_name

    # the list of monitors associated to the block
    self.monitor_models = []

    # the list of goals associated to the block
    self.goal_models = []

    # the list of software knob associated to the block
    self.software_knobs = []

    # the list of data features associated to the block
    self.features = []

    # how to compute the distance between data feature
    self.feature_distance = ''

    # the list of metrics associated to the block
    self.metrics = []

    # the list of the optimization problems associated to the block
    self.state_models = []

    # the list of field adaptors models
    self.field_adaptor_models = []

    # the application name in agora context
    self.application_name = ''

    # the agora model, to enable online learning
    self.agora_model = None



  def __str__(self):
    """
    Dump this model as collection of others models
    """

    dump_string = ''

    dump_string = '{0}\n\n######### Block specification #########'.format(dump_string)
    dump_string = '{0}\n##  Name: {1}'.format(dump_string, self.block_name)
    dump_string = '{0}\n#######################################'.format(dump_string)

    dump_string2 = '\n\n-----------  MONITORS -----------'
    for monitor_model in self.monitor_models:
      dump_string2 = '{0}\n{1}'.format(dump_string2, monitor_model)

    dump_string2 = '{0}\n\n--------- FIELD ADAPTORS --------'.format(dump_string2)
    for field_adaptor_model in self.field_adaptor_models:
      dump_string2 = '{0}\n{1}'.format(dump_string2, field_adaptor_model)
    dump_string2 = '{0}\n'.format(dump_string2)

    dump_string2 = '{0}\n\n-------------  GOALS ------------'.format(dump_string2)
    for goal_model in self.goal_models:
      dump_string2 = '{0}\n{1}'.format(dump_string2, goal_model)

    dump_string2 = '{0}\n\n--------  SOFTWARE KNOBS --------'.format(dump_string2)
    for knob_model in self.software_knobs:
      dump_string2 = '{0}\n{1}'.format(dump_string2, knob_model)

    dump_string2 = '{0}\n\n--------  DATA FEATURES  --------'.format(dump_string2)
    for feature_model in self.features:
      dump_string2 = '{0}\nData feature distance = {1}'.format(dump_string2, self.feature_distance.upper())
      dump_string2 = '{0}\n{1}'.format(dump_string2, feature_model)

    dump_string2 = '{0}\n\n------------ METRICS ------------'.format(dump_string2)
    for metric_model in self.metrics:
      dump_string2 = '{0}\n{1}'.format(dump_string2, metric_model)

    dump_string2 = '{0}\n\n------------  STATES ------------'.format(dump_string2)
    for state_model in self.state_models:
      dump_string2 = '{0}\n{1}'.format(dump_string2, state_model)
    dump_string2 = '{0}\n'.format(dump_string2)


    dump_string2 = '{0}\n\n------------- AGORA -------------'.format(dump_string2)
    if self.agora_model is not None:
        dump_string2 = '{0}\n{1}'.format(dump_string2, self.agora_model)
    dump_string2 = '{0}\n'.format(dump_string2)

    dump_string2 = dump_string2.replace('\n', '\n# ')
    dump_string = '{0}{1}\n#######################################'.format(dump_string,dump_string2)
    dump_string = '{0}\n#######################################\n\n'.format(dump_string)

    return dump_string



  def postprocess(self):
    """
    This method performs the following operations:
      - propagate the name of the field in a static goal toward the state
      - check if all the field adaptors refer to a known monitor and a known metric
      - check if all the goals refer to a known knob or metric
      - check if the agora model referes to all the metrics, knobs and features of the block
      - check if all the metrics in agora are observed by at least one monitor
    """

    # ----- propagate the target field for a static constraint

    # loop over the states
    for state_model in self.state_models:

      # loop over the constraints
      for constraint_model in state_model.constraint_list:

        # get the goal referene
        goal_ref = constraint_model.goal_ref

        # loop over the goals
        for goal_model in self.goal_models:

          # check if it is the target goal
          if goal_model.name == goal_ref:

            # propagate the target field
            if goal_model.metric_name_ref:
              constraint_model.target_metric = goal_model.metric_name_ref
            if goal_model.parameter_name_ref:
              constraint_model.target_knob = goal_model.parameter_name_ref
            break

    # ----- check the goals

    # loop over the goals
    for goal_model in self.goal_models:

      # check if the goal is on a knob, which is defined
      knob_name = goal_model.parameter_name_ref
      if knob_name:

        # look for the corresponding knob
        found = False

        # loop through the knobs
        for knob_model in self.software_knobs:
          if knob_model.name == knob_name:
            found = True
            break

        # check if we found a monitor
        if not found:
          print('[CONSISTENCY ERROR] The goal "{0}" has a reference to an unknown knob "{1}"'.format(goal_model.name, knob_name))
          print('                    Available knobs: "{0}"'.format('", "'.join([x.name for x in self.software_knobs])))
          sys.exit(-1)

      # check if the goal is on a metric, which is defined
      metric_name = goal_model.metric_name_ref
      if metric_name:

        # look for the corresponding knob
        found = False

        # loop through the knobs
        for metric_model in self.metrics:
          if metric_model.name == metric_name:
            found = True
            break

        # check if we found a monitor
        if not found:
          print('[CONSISTENCY ERROR] The goal "{0}" has a reference to a metric "{1}"'.format(goal_model.name, metric_name))
          print('                    Available knobs: "{0}"'.format('", "'.join([x.name for x in self.metrics])))
          sys.exit(-1)


    # ----- check the fields adaptor

    # loop over the field adators
    for field_adaptor in self.field_adaptor_models:

      # get the reference name of the monitor
      monitor_name = field_adaptor.monitor_name

      # get the name of the parameter
      if field_adaptor.knob_name:
        param_name = field_adaptor.knob_name
      if field_adaptor.metric_name:
        param_name = field_adaptor.metric_name

      # loop over the monitors
      found = False
      for monitor in self.monitor_models:
        if (monitor.monitor_name == monitor_name):
          found = True
          break

      # check if we found a monitor
      if not found:
        print('[CONSISTENCY ERROR] The adaptor of field "{0}" adapt an unknown monitor "{1}"'.format(param_name, monitor_name))
        print('                    Available monitors: "{0}"'.format('", "'.join([x.monitor_name for x in self.monitor_models])))
        sys.exit(-1)

      # loop over the metrics
      found = False
      for metric_model in self.metrics:
        if metric_model.name == param_name:
          found = True
          break


      # check if we found the metric
      if not found:
        print('[CONSISTENCY ERROR] The adaptor of field "{0}" refer to an unknown metric'.format(param_name))
        print('                    Available metrics: "{0}"'.format('", "'.join([x.name for x in self.metrics])))
        sys.exit(-1)


    # ----- enforce the consistency of agora

    if self.agora_model is not None:

        # propagate the application name
        self.agora_model.application_name = self.application_name

        # get all the knobs, features and metrics from the model
        block_knobs = sorted([ x.name for x in self.software_knobs ])
        block_features = sorted([ x.name for x in self.features ])
        block_metrics = sorted([ x.name for x in self.metrics ])

        # get all the knobs, features and metrics from the model
        agora_knobs = sorted(self.agora_model.knobs_values.keys())
        agora_features = sorted(self.agora_model.features_values.keys())
        agora_metrics = sorted(self.agora_model.metrics_predictors.keys())

        # check the consistency
        if not agora_knobs == block_knobs:
            print('[CONSISTENCY ERROR] The software knobs of the block and agora differs!')
            print('                    Block software-knobs: "{0}"'.format('", "'.join(block_knobs)))
            print('                    Agora software-knobs: "{0}"'.format('", "'.join(agora_knobs)))
            sys.exit(-1)
        if not block_features == agora_features:
            print('[CONSISTENCY ERROR] The features of the block and agora differs!')
            print('                    Block features: "{0}"'.format('", "'.join(block_features)))
            print('                    Agora features: "{0}"'.format('", "'.join(agora_features)))
            sys.exit(-1)
        if not block_metrics == agora_metrics:
            print('[CONSISTENCY ERROR] The metrics of the block and agora differs!')
            print('                    Block metrics: "{0}"'.format('", "'.join(block_metrics)))
            print('                    Agora metrics: "{0}"'.format('", "'.join(agora_metrics)))
            sys.exit(-1)



    # ----- enforce that all the metrics are observed by a monitor

    if self.agora_model is not None:

        # get all the monitors related to metrics
        agora_monitors = set([ self.agora_model.metrics_monitors[x] for x in self.agora_model.metrics_monitors ])

        # get all the monitors of the block
        block_monitors = sorted([x.monitor_name for x in self.monitor_models])

        # check the consistency
        for monitor in agora_monitors:
            if monitor not in block_monitors:
                print('[CONSISTENCY ERROR] A metric is observed by the unknown monitor "{0}"'.format(monitor))
                print('                    Available monitors: "{0}"'.format('", "'.join(block_monitors)))
                sys.exit(-1)
