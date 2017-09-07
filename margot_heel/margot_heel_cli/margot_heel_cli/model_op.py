class OperatingPointModel:
  """
  This class represents an Operating Point.
  It includes the following information:
    - metrics -> the map that describes the average value of the software knobs
    - knobs ->the map that describes the average value of the metrics
    - metrics_std -> the standard deviation of the metrics
  """

  def __init__(self):
    self.metrics = {}     # key -> metric_name, value -> metric_average
    self.knobs = {}       # key -> knob_name, value -> metric_average
    self.metrics_std = {} # key -> metric_name, value -> metric_standard_deviation

  def __str__(self):
    string = '{0}\tSoftware-knobs:\n'.format(string)
    for knob_name in self.knobs:
      string = '{0}\t\t{1} = {2}\n'.format(string, knob_name, self.knobs[knob_name])
    string = '{0}\tMetrics:\n'.format(string)
    if (self.mertics_std):
      for metric_name in self.metrics:
        string = '{0}\t\t{1} = {2} +- {3}\n'.format(string, metric_name, self.metrics[metric_name], self.metrics_std[metric_name])
      else:
        string = '{0}\t\t{1} = {2}\n'.format(string, metric_name, self.metrics[metric_name])
    return string


  def similar(self, other_op):
    """
    Return true if other_op has the same fields wrt self
    """

    my_metrics = sorted(self.metrics.keys())
    my_knobs = sorted(self.knobs.keys())

    other_metrics = sorted(other_op.metrics.keys())
    other_knobs = sorted(other_op.knobs.keys())

    standard_dev_num = len(self.metrics_std.keys())
    other_standard_dev_num = len(other_op.metrics_std.keys())

    return (my_metrics == other_metrics) and (my_knobs == other_knobs) and (standard_dev_num == other_standard_dev_num)


  def add(self, other_op):
    """
    add the metrics of the two ops. have to be similar
    """
    if self.similar(other_op):
      for key in self.metrics.keys():
        self.metrics[key]=float(other_op.metrics[key])+float(self.metrics[key])
        if self.mertics_std:
          self.metrics_std[key] = float(other_op.metrics_std[key]) + float(self.metrics_std[key])

    else:
      print ("attempt to add metrics of non similar ops: aborting")
      print (self)
      print (other_op)
      sys.exit (-1)


  def avg(self, num_values, key):
    """
    divides the metric identified by key for the num_values.
    """
    if key in self.metrics.keys():
      self.metrics[key]=float(self.metrics[key])/int(num_values)
      self.metrics_std[key]=float(self.metrics_std[key])/int(num_values)
