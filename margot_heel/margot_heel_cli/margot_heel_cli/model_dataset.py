class Dataset:
  """
  This class collects all the information about a dataset,
  whether it is a training or a production one.
  """

  available_dataset_types = ['training', 'production']

  def __init__( self ):

    self.type = ''
    self.input_data_models = []
    


  def __str__(self):
    """
    Dump the content of the class
    """

    dump_string = ''

    dump_string = '{0}\n*************************************'.format(dump_string)
    dump_string = '{0}\n*    Dataset Type:      {1}'.format(dump_string, self.type)    
    dump_string2 = '\n\n-----------  INPUT DATA -----------'
    for input_data_model in self.input_data_models:
      dump_string2 = '{0}\n{1}'.format(dump_string2, input_data_model)

    dump_string2 = dump_string2.replace('\n', '\n* ')
    dump_string = '{0}{1}\n*************************************'.format(dump_string,dump_string2)
    dump_string = '{0}\n'.format(dump_string)

    return dump_string
