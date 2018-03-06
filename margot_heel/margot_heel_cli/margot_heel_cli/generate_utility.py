

def generate_start_monitor_signature( block_model, use = False ):
  """
  This function generates the signature of the start_monitor
  function
  """

  # get the start parameter from all the monitors
  start_parameters = []
  for monitor_model in block_model.monitor_models:
    start_parameters.extend( monitor_model.start_parameters )

  # compose the parameter signature
  if not use:
    param_string = ', '.join('{0} {1}'.format(x.var_type, x.var_name) for x in start_parameters if x.var_name )
  else:
    param_string = ', '.join(x.var_name for x in start_parameters if x.var_name )
  if not param_string:
    if not use:
      param_string = 'void'
    else:
      param_string = ''

  # compose the whole signature
  signature = 'start_monitor( {0} )'.format(param_string)

  return signature




def generate_stop_monitor_signature( block_model, use = False ):
  """
  This function generates the signature of the stop_monitor
  function
  """

  # get the stop parameter from all the monitors
  stop_parameters = []
  for monitor_model in block_model.monitor_models:
    stop_parameters.extend( monitor_model.stop_parameters )

  # compose the parameter signature
  if not use:
    param_string = ', '.join('{0} {1}'.format(x.var_type, x.var_name) for x in stop_parameters if x.var_name )
  else:
    param_string = ', '.join(x.var_name for x in stop_parameters if x.var_name )
  if not param_string:
    if not use:
      param_string = 'void'
    else:
      param_string = ''

  # compose the whole signature
  signature = 'stop_monitor( {0} )'.format(param_string)

  return signature


def generate_update_signature( block_model, use = False, c_language = False ):
  """
  This function generates the signature of the update
  function
  """

  # compose the parameter signature
  if not use:
    if not c_language:
      param_list = ['{0}& {1}'.format(x.var_type, x.var_name) for x in block_model.software_knobs]
      param_list.extend(['const {0} {1}'.format(x.type, x.name) for x in block_model.features])
    else:
      param_list = ['{0}* {1}'.format(x.var_type, x.var_name) for x in block_model.software_knobs]
      param_list.extend(['const {0} {1}'.format(x.type, x.name) for x in block_model.features])
    param_string = ', '.join( param_list )
  else:
    if not c_language:
      param_list = ['{0}'.format(x.var_name) for x in block_model.software_knobs]
      param_list.extend(['{0}'.format(x.name) for x in block_model.features])
    else:
      param_list = ['*{0}'.format(x.var_name) for x in block_model.software_knobs]
      param_list.extend(['{0}'.format(x.name) for x in block_model.features])
    param_string = ', '.join(param_list)
  if not param_string:
    if not use:
      param_string = 'void'
    else:
      param_string = ''

  # compose the whole signature
  signature = 'update( {0} )'.format(param_string)

  return signature
