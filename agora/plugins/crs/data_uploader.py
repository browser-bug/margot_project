from __future__ import print_function
import argparse
import inspect
import os
import sys

import csv

def update_cassandra_database( args, root_path ):

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
    knobs_keys = []
    knobs = session.execute('SELECT name FROM {0}'.format(args.knobs))
    knobs_keys = sorted( [ str(x[0]).lower() for x in knobs ] )

    # get all the features
    feature_keys = []
    features = session.execute('SELECT name FROM {0}'.format(args.features))
    feature_keys = sorted( [ str(x[0]).lower() for x in features ] )

    # compose the predictors
    predictor_columns = knobs_keys
    predictor_columns.extend(feature_keys)

    # open the predicted values
    with open(os.path.join(root_path, 'prediction.txt'), 'r') as model_output:

        # initialize the csv reader
        model_reader = csv.reader(model_output, delimiter=',')

        # read the file
        for index,row in enumerate(model_reader):

            # get the header
            if index == 0:
                header = {}
                for column_index, column_name in enumerate(row):
                    header[column_name.lower()] = column_index
                continue

            # declare the set clause that update the table
            mean_value = row[header['mean']]
            std_value = row[header['std']]
            set_clause = '{0}_avg = {1}, {0}_std = {2}'.format(args.metric, mean_value, std_value)

            # figure out the configuration
            terms = ['{0} = {1}'.format(predictor, row[header[predictor]]) for predictor in predictor_columns]

            # declare the where statement
            where_cluase = '{0}'.format(' and '.join(terms))

            # execute the query
            session.execute('UPDATE {0} SET {1} WHERE {2};'.format(args.model, set_clause, where_cluase))



def update_csv_model( args, root_path ):

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

    # this variable holds the list of predictions made with this
    # stupid plugins that requires data pre- and post-processing
    precition = {}

    # open the file with the plugin predictions
    with open('prediction_request.txt', mode='r') as infile:
        reader = csv.DictReader(infile)

        # loop over the model
        for row in reader:

            # append the prediction
            prediction[','.join(row[x] for x in predictors)] = [ row['mean'], row['std'] ]

    model_rows = []

    # open the actual model to store data
    with open(args.model, mode='r') as infile:
        reader = csv.DictReader(infile)
        model_rows.append(','.join(reader.fieldnames))
        for row in reader:
            configuration = ','.join(row[x] for x in predictors)

            this_row = []
            for field_name in reader.fieldnames:
                if field_name == '{0}_avg'.format(args.metric):
                    this_row.append(prediction[configuration][0])
                else:
                    if field_name == '{0}_std'.format(args.metric):
                        this_row.append(prediction[configuration][1])
                    else:
                        this_row.append(row[field_name])

    # open the model container to write the updated values
    with open(args.model, mode='w') as outfile:
        for row in model_rows:
            outfile.write('{0}\n'.format(row))


if __name__ == '__main__':

    # get the path to this file
    root_path =  os.path.realpath(os.path.dirname(inspect.getfile( inspect.currentframe() )))

    # handle the program options
    arg_parser = argparse.ArgumentParser(description='Utility script that prepare the data for the R script')
    arg_parser.add_argument('--version', action='version', version='AGORA dumper 1.0',
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
        update_cassandra_database(args, root_path)
        sys.exit(os.EX_OK)
    if args.storage_type == 'CSV':
        update_csv_model(args, root_path)
        sys.exit(os.EX_OK)

    # if we reach this point, we don't know how to dump the information
    sys.exit(os.EX_CONFIG)
