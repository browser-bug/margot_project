import sys    # for the upperlimit of the integers


class ExperimentModel:
  """
  This class represents a whole experiment that includes more than one trace

  Attributes:
    - experiments []               -> a list of the trace models of this experiment
    - field_mask []                -> a list of strings, which represents the interested fields of the experiments
    - common_normalized_metrics [] -> a list of strings, which represents the common fields between the experiments
                                      and that should start from a global zero
  """


  def __init__(self):
    """
    Default ctor of the class that initialize the attributes
    """
    self.experiments = []
    self.field_mask = []
    self.common_normalized_metrics = []


  def generate_data(self):
    """
    Generate the list of data. If common_timestamp is True then it normalize the timestamps

    Return value

      - data {}     -> Dictionary that contains the data per application
                       key -> id_trace, value -> data matrix
      - header {}   -> Fictaionary that translates the field name to the data index in the data matrix
                       key -> field_name, value -> index on data matrix
    """

    # declare the container of the data per experiment
    overall_data_grouped_per_trace = {}


    # declare the new translator for the fields
    new_header = {}
    for index, field in enumerate(self.field_mask):
      new_header[field] = index


    # populate the data
    for experiment in self.experiments:

      # declare the container of the data
      data_per_experiment_horizontal = []
      data_per_experiment_vertical = []

      min_length_data = float("inf")
      for field in self.field_mask:
        data = experiment.get_column(field)
        min_length_data = min(min_length_data, len(data))
        data_per_experiment_horizontal.append(data)


      # eventually slice the data if they exceed the minimum lenght
      data_per_experiment = []
      for index in range(len(data_per_experiment_horizontal)):
        data = data_per_experiment_horizontal[index]
        if len(data) > min_length_data:
          data = data[:min_length_data]
        data_per_experiment.append(data)


      # rotate the dataset to be easily printed
      for index_row in range(min_length_data):
        data_row_vertical = []
        for index_metric in range(len(self.field_mask)):
          data_row_vertical.append(data_per_experiment[index_metric][index_row])
        data_per_experiment_vertical.append(data_row_vertical)

      # append the data to the global struct
      overall_data_grouped_per_trace[experiment.name] = data_per_experiment_vertical


    # loop over the metrics that should be commonly normalized
    for metric in self.common_normalized_metrics:
      metric_minimum = sys.float_info.max
      for trace_id in overall_data_grouped_per_trace:
        data = [ x[new_header[metric]] for x in overall_data_grouped_per_trace[trace_id]]
        metric_minimum = min(metric_minimum, min(data))
      for trace_id in overall_data_grouped_per_trace:
        for row in overall_data_grouped_per_trace[trace_id]:
          row[new_header[metric]] -= metric_minimum

    return overall_data_grouped_per_trace, new_header
