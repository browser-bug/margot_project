import sys

class DataFeaure:
  """
  This class represents a single data feature handled by margot
    - the feature name
    - the feature type
    - the feature comparison function
  """

  def __init__(self, feature_name, feature_type, feature_cf):

    # The feature definition
    self.name = feature_name
    self.type = feature_type
    self.cf = feature_cf

  def __str__(self):
    """
    dump the data feature model model
    """
    dump_string = '\n'
    dump_string = '{0}\n  Data Feature specification'.format(dump_string)
    dump_string = '{0}\n    Name:       {1}'.format(dump_string, self.name)
    dump_string = '{0}\n    Type:       {1}'.format(dump_string, self.type)
    dump_string = '{0}\n    Comparison: {1}'.format(dump_string, self.cf)
  
    return dump_string

class DataFeatureModel:
  """
  This class represents a data feature handled by margot
    - the feature name
    - the feature type
    - the feature comparison function
  """

  # the available comparison function for the input feature
  available_comparison_functions = [ 'GE', 'LE', '-']

  # the available data features types
  available_var_types = ['int', 'float', 'double', 'long double']

  # the available distance types
  available_feature_combination = ['NORMALIZED', 'EUCLIDEAN']

  def __init__(self):

    # The list of all the data features
    self.features = []

    # how to get the closer one
    self.distance = ''


  def add_feature(self, feature_name, feature_type, feature_cf ):
    self.features.append( DataFeaure(feature_name, feature_type, feature_cf) )


  def __str__(self):
    """
    dump the data feature model model
    """

    # dump the general information
    dump_string = '\n'
    dump_string = '{0}\n  Data feature distance: {1}'.format(dump_string, self.distance)


    # dump each feature value
    for feature in self.features:
      dump_string = '{0}\n'.format(dump_string)
      dump_string = '{0}\n  Data Feature specification'.format(dump_string)
      dump_string = '{0}\n    Name:       {1}'.format(dump_string, feature.name)
      dump_string = '{0}\n    Type:       {1}'.format(dump_string, feature.type)
      dump_string = '{0}\n    Comparison: {1}'.format(dump_string, feature.cf)
    
    return dump_string