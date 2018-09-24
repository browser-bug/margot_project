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
    self.number_point_per_dimension = 10
    self.number_observations_per_point = 5
    self.min_distance = 0.2


  def __str__(self):
    """
    Dump the content of the class
    """

    dump_string = ''

    dump_string = '{0}\n\n  Agora application local handler'.format(dump_string)
    dump_string = '{0}\n    Application Name:     {1}'.format(dump_string, self.application_name)
    dump_string = '{0}\n    Broker url:           {1}'.format(dump_string, self.broker_url)
    dump_string = '{0}\n    Broker username:      {1}'.format(dump_string, self.username)
    dump_string = '{0}\n    Broker password:      {1}'.format(dump_string, self.password)
    dump_string = '{0}\n    Message qos:          {1}'.format(dump_string, self.qos)
    dump_string = '{0}\n    DoE:                  {1}'.format(dump_string, self.doe_name)
    dump_string = '{0}\n    N point x dimension:  {1}'.format(dump_string, self.number_point_per_dimension)
    dump_string = '{0}\n    N obs. x point:       {1}'.format(dump_string, self.number_observations_per_point)
    dump_string = '{0}\n    Minimum distance:     {1}'.format(dump_string, self.min_distance)
    dump_string = '{0}\n    Exploring:            {1}'.format(dump_string, self.knobs_values)
    dump_string = '{0}\n    Predicting:           {1}'.format(dump_string, self.metrics_predictors)
    dump_string = '{0}\n    Given:                {1}'.format(dump_string, self.features_values)

    return dump_string
