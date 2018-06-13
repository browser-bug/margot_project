class AgoraModel:
  """
  This class collects all the information about the local application handler
  that is communicating with the agora remote application handler
  """

  available_does = ['full_factorial']

  def __init__( self ):

    self.application_name = ''
    self.broker_url = ''
    self.username = ''
    self.password = ''
    self.broker_ca = ''
    self.client_cert = ''
    self.client_key = ''
    self.qos = '2'
    self.doe = 'full_factorial'
    self.number_observation = '5'
    self.pause_polling = ''
    self.pause_timeout = ''
    self.metrics_predictors = {}
    self.knobs_values = {}
    self.features_values = {}
    self.metrics_monitors = {}
    


  def __str__(self):
    """
    Dump the content of the class
    """

    dump_string = ''

    dump_string = '{0}\n\n  Agora application local handler'.format(dump_string)
    dump_string = '{0}\n    Application Name:  {1}'.format(dump_string, self.application_name)
    dump_string = '{0}\n    Broker url:        {1}'.format(dump_string, self.broker_url)
    dump_string = '{0}\n    Broker username:   {1}'.format(dump_string, self.username)
    dump_string = '{0}\n    Broker password:   {1}'.format(dump_string, self.password)
    dump_string = '{0}\n    Message qos:       {1}'.format(dump_string, self.qos)
    dump_string = '{0}\n    DoE strategy:      {1}'.format(dump_string, self.doe)
    dump_string = '{0}\n    Number observation:{1}'.format(dump_string, self.number_observation)
    dump_string = '{0}\n    Pause polling time:{1}'.format(dump_string, self.pause_polling)
    dump_string = '{0}\n    Pause timeout:     {1}'.format(dump_string, self.pause_timeout)
    dump_string = '{0}\n    Exploring:         {1}'.format(dump_string, self.knobs_values)
    dump_string = '{0}\n    Predicting:        {1}'.format(dump_string, self.metrics_predictors)
    dump_string = '{0}\n    Given:             {1}'.format(dump_string, self.features_values)

    return dump_string
