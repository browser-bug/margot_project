import argparse
import inspect
import os
import sys
import statistics


def compute_average_model_cassandra( args, root_path ):

    # import the cassandra client
    import cassandra
    from cassandra.cluster import Cluster
    from cassandra.auth import PlainTextAuthProvider

    # connect to the cassandra database
    cassandra_node_url = [args.storage_address]
    if args.storage_username and args.storage_password:
        auth = PlainTextAuthProvider(username=args.storage_username, password=args.storage_password)
        cluster = Cluster(cassandra_node_url, auth_provider=auth)
    else:
        cluster = Cluster(cassandra_node_url)
    session = cluster.connect()

    # get all the knobs
    query_result = session.execute('SELECT name FROM {0}'.format(args.knobs))
    knobs_name = sorted([ x[0] for x in query_result ])

    # get all the features
    query_result = session.execute('SELECT name FROM {0}'.format(args.features))
    features_name = sorted([ x[0] for x in query_result ])

    # change the way that cassandra uses to prduce the results
    session.row_factory = cassandra.query.dict_factory

    # get the filter for column in the dse table
    predictor_columns = knobs_name
    predictor_columns.extend(features_name)

    # get all the required predictions
    for operating_point in session.execute('SELECT {1} FROM {0};'.format(args.model, ','.join(predictor_columns))):

        # compose the where clause for the query
        where_clause = ' AND '.join('{0} = {1}'.format(name, operating_point[name]) for name in predictor_columns)

        # get all the observation for this configuartion
        data_values = []
        for observation in session.execute('SELECT {1} FROM {0} WHERE {2} ALLOW FILTERING;'.format(args.observation, args.metric, where_clause)):
            data_values.append(float(observation[args.metric]))

        # if we don't have any observation we need to raise an exception
        if not data_values:
            raise ValueError('There is no observations for the following model entry: "{0}"'.format(where_clause))

        # if we reach this statement, we are able to compute statistics
        metric_mean = statistics.mean(data_values)
        metric_std = statistics.stdev(data_values) if len(data_values) > 1 else 0.0
        set_clause = '{0}_avg = {1}, {0}_std = {2}'.format(args.metric, metric_mean, metric_std)

        # then we update the model
        session.execute('UPDATE {0} SET {1} WHERE {2};'.format(args.model, set_clause, where_clause))



def compute_average_model_csv( args, root_path ):

    # import the cassandra client
    import csv

    # this list will contain the name of all the predictors
    predictors = []

    # open the knobs container
    with open(args.knobs, mode='r') as infile:
        reader = csv.DictReader(infile)
        for row in reader:
            predictors.append(row['name'])

    # open the features container
    with open(args.features, mode='r') as infile:
        reader = csv.DictReader(infile)
        for row in reader:
            predictors.append(row['name'])

    # this dictionary will cluster the metric value for each configuration
    trace = {}

    # open the trace container to cluster the observations
    with open(args.observation, mode='r') as infile:
        reader = csv.DictReader(infile)

        # loop over the observation
        for row in reader:

            # get the name of the configuration as key
            key_value = ','.join( row[x] for x in predictors )

            # get the metric value
            metric_value = float(row[args.metric])

            # append the value in the trace
            try:
                trace[key_value].append(metric_value)
            except ValueError as err:
                trace[key_value] = [ metric_value ]

    # this list holds the new model
    model = []

    # open the model container and compute the application model
    with open(args.model, mode='r') as infile:
        reader = csv.DictReader(infile)

        # append the header
        model.append(','.join(reader.fieldnames))

        # loop over the predictions
        for row in reader:

            # get the name of the configuration as key
            key_value = ','.join( row[x] for x in predictors )

            # compute the average and standard deviation
            metric_mean = str(statistics.mean(trace[key_value]))
            metric_std = str(statistics.stdev(trace[key_value]) if len(trace[key_value]) > 1 else 0.0)

            # update the model
            this_row = []
            for field_name in reader.fieldnames:
                if field_name == '{0}_avg'.format(args.metric):
                    this_row.append(metric_mean)
                else:
                    if field_name == '{0}_std'.format(args.metric):
                        this_row.append(metric_std)
                    else:
                        this_row.append(row[field_name])
            model.append(','.join(this_row))

    # open the model container to write the updated values
    with open(args.model, mode='w') as outfile:
        for row in model:
            outfile.write('{0}\n'.format(row))




if __name__ == '__main__':

    # get the path to this file
    root_path =  os.path.realpath(os.path.dirname(inspect.getfile( inspect.currentframe() )))

    # handle the program options
    arg_parser = argparse.ArgumentParser(description='Simple plugin that computes the average of configurations')
    arg_parser.add_argument('--version', action='version', version='AGORA average 1.0',
                            help='Print the version of the tools and exit')
    arg_parser.add_argument('--storage_type', dest='storage_type', type=str, required=True,
                            help='The type of the storage used by the application remote handler')
    arg_parser.add_argument('--storage_address', dest='storage_address', type=str, required=True,
                            help='The url of the storage')
    arg_parser.add_argument('--storage_username', dest='storage_username', type=str, required=False,
                            default="", help='The username required to authenticate in the storage')
    arg_parser.add_argument('--storage_password', dest='storage_password', type=str, required=False,
                            default="", help='The password required to authenticate in the storage')
    arg_parser.add_argument('--observation_name', dest='observation', type=str, required=True,
                            help='The name of the container with the observations')
    arg_parser.add_argument('--features_name', dest='features', type=str, required=True,
                            help='The name of the container with the name of the features')
    arg_parser.add_argument('--knobs_name', dest='knobs', type=str, required=True,
                            help='The name of the container with the name of the software knobs')
    arg_parser.add_argument('--model_name', dest='model', type=str, required=True,
                            help='The name of the container with the required predictions')
    arg_parser.add_argument('--target_metric', dest='metric', type=str, required=True,
                            help='The name of the metric to predict')
    args = arg_parser.parse_args()

    # check which is the type of the storage
    if args.storage_type == 'CASSANDRA':
        compute_average_model_cassandra(args, root_path)
        sys.exit(os.EX_OK)
    if args.storage_type == 'CSV':
        compute_average_model_csv(args, root_path)
        sys.exit(os.EX_OK)

    # if we reach this point, we don't know how to dump the information
    sys.exit(os.EX_CONFIG)
