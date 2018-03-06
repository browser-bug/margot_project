import csv


class TraceModel:
  """
  This class represents a single trace of the application. It takes for granted that the first line
  is the header that gives a name to to the column.

  Attributes
    - data [[]]   -> The parsed data structure as in the csv file
    - header {}   -> A dictionary to correlate the name of the column with its index
                       key   -> name of the column
                       value -> index of the column
    - name        -> The name of the source file
  """

  def __init__(self, path_to_file, my_delimiter = ' '):
    """
    Parse the csv and initialize the internal data structure

    Arguments:
      - path_to_file -> The path of the target trace file
      - delimiter    -> The character used to split the columns
    """

    # initialize the internal data structer
    self.data = []
    self.header = {}
    self.name = path_to_file

    # parse the whole csv file
    csvmatrix = []
    with open(path_to_file, 'r') as csvfile:
      tracereader = csv.reader(csvfile, delimiter = my_delimiter)
      for row in tracereader:
        csvmatrix.append(row)

    # compute the header
    header_raw = csvmatrix.pop(0)
    for index_col, name_col in enumerate(header_raw):
      self.header[name_col] = index_col

    # convert the data
    for row in csvmatrix:
      data = []
      for x in row:
        try:
          data.append(float(x))
        except ValueError as e:
          data.append(x)
      self.data.append(data)


  def __nonzero__(self):
    """
    Python 2.x check for emptyness
    """
    if self.data:
      return True
    else:
      return False


  def __bool__(self):
    """
    Python 3.x check for emptyness
    """
    if self.data:
      return True
    else:
      return False


  def get_column(self, col_name):
    """
    Return the list of values given the name of the column
    """
    return [x[self.header[col_name]] for x in self.data]


  def sum_column(self, col_name):
    """
    Return the sum of the values of the column name
    """
    return sum(self.get_column(col_name))


  def get_timestamp(self, normalize = True ):
    """
    Return the list of the timestamp.

    Arguments
      - normalize  -> If true it removes the initial offset from the data
    """

    # get the values
    timestamps = self.get_column('Timestamp')

    # normalize them if needed
    if normalize:
      first_timestamp = timestamps[0]
      timestamps = [x - first_timestamp for x in timestamps]

    return timestamps


  def add_column(self, column_name, formula):
    """
    Add a column in the trace file to compute an indirect metric

    Arguments
      - column_name The name of the new column
      - formula How to compute the new column
    """

    # check if there is already a column like the target one
    if column_name in self.header:
      return

    # update the header
    self.header[column_name] = len(self.header)

    # update the data
    for row in self.data:

      # sobstitute the placeholders in the formula with the actual numeric values
      new_value_str = formula
      replaceable_fields = sorted(self.header, key = len, reverse = True )
      for field in replaceable_fields:
        if field in formula:
          new_value_str = new_value_str.replace(field, str(row[self.header[field]]))

      # evaluate the expression
      try:
        new_value_numeric = eval(new_value_str)
        row.append(float(new_value_numeric))
      except:
        row.append(new_value_str)
