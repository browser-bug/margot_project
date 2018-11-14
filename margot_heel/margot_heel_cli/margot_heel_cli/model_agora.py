class AgoraModel:
  """
  This class collects all the information about the local application handler
  that is communicating with the agora remote application handler
  """

  available_does = ['strauss', 'dmax', 'lhs', 'wsp', 'factorial3', 'factorial5', 'full_factorial']

  def __init__( self ):

    self.application_name = ''
    self.broker_url = '127.0.0.1:1883'
    self.username = ''
    self.password = ''
    self.broker_ca = ''
    self.client_cert = ''
    self.client_key = ''
    self.qos = '2'
    self.metrics_predictors = {}
    self.knobs_values = {}
    self.features_values = {}
    self.metrics_monitors = {}
    self.doe_name = 'dmax'
    self.number_configurations_per_iteration = 10
    self.number_observations_per_configuration = 5
    self.max_number_iteration = 10
    self.max_mae = 0.2
    self.min_r2 = 0.7
    self.validation_split = 0.2
    self.k_value = 5
    self.min_distance = 0.2
    self.constraints = []


  def __str__(self):
    """
    Dump the content of the class
    """

    dump_string = ''

    dump_string = '{0}\n\n  Agora application local handler'.format(dump_string)
    dump_string = '{0}\n    Application Name:      {1}'.format(dump_string, self.application_name)
    dump_string = '{0}\n    Broker url:            {1}'.format(dump_string, self.broker_url)
    dump_string = '{0}\n    Broker username:       {1}'.format(dump_string, self.username)
    dump_string = '{0}\n    Broker password:       {1}'.format(dump_string, self.password)
    dump_string = '{0}\n    Message qos:           {1}'.format(dump_string, self.qos)
    dump_string = '{0}\n    DoE:                   {1}'.format(dump_string, self.doe_name)
    dump_string = '{0}\n    Confs per iteration:   {1}'.format(dump_string, self.number_configurations_per_iteration)
    dump_string = '{0}\n    Obs per configuration: {1}'.format(dump_string, self.number_observations_per_configuration)
    dump_string = '{0}\n    N iterations:          {1}'.format(dump_string, self.max_number_iteration)
    dump_string = '{0}\n    Max MaE:               {1}'.format(dump_string, self.max_mae)
    dump_string = '{0}\n    Min R^2:               {1}'.format(dump_string, self.min_r2)
    dump_string = '{0}\n    Validation split:      {1}'.format(dump_string, self.validation_split)
    dump_string = '{0}\n    K-fold value:          {1}'.format(dump_string, self.k_value)
    dump_string = '{0}\n    Minimum distance:      {1}'.format(dump_string, self.min_distance)
    dump_string = '{0}\n    Exploring:             {1}'.format(dump_string, self.knobs_values)
    dump_string = '{0}\n    Constraints:           {1}'.format(dump_string, self.constraints)
    dump_string = '{0}\n    Predicting:            {1}'.format(dump_string, self.metrics_predictors)
    dump_string = '{0}\n    Given:                 {1}'.format(dump_string, self.features_values)

    return dump_string
