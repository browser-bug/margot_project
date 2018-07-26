from __future__ import print_function
import argparse
import inspect
import os
import sys


import cassandra
from cassandra.cluster import Cluster
from cassandra.auth import PlainTextAuthProvider


def dump_cassandra_database( args, root_path ):

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
    with open(os.path.join(root_path, 'knobs.txt'), 'w') as outfile:
        knobs_keys = sorted( [ x[0] for x in knobs ] )
        outfile.write('{0}\n'.format('\n'.join(knobs_keys)))



    # get all the features
    feature_keys = []
    features = session.execute('SELECT name FROM {0}'.format(args.features))
    with open(os.path.join(root_path, 'features.txt'), 'w') as outfile:
        feature_keys = sorted( [ x[0] for x in features ] )
        outfile.write('{0}\n'.format('\n'.join(feature_keys)))

    # change the way that cassandra uses to prduce the results
    session.row_factory = cassandra.query.dict_factory

    # get the filter for column in the dse table
    predictor_columns = knobs_keys
    predictor_columns.extend(feature_keys)

    dse_columns = list(predictor_columns)
    dse_columns.append(args.metric)

    # get all the observations
    observations = session.execute('SELECT {1} FROM {0}'.format(args.observation, ','.join(dse_columns)))
    with open(os.path.join(root_path, 'dse.txt'), 'w') as outfile:
        for index, values in enumerate(observations) :

            # first we need to sort the key of the result, to enforce consistency
            row_header = sorted(values.keys())
            row_values = [ str(values[x]) for x in row_header ]

            # if it is the first step, we need to print the table header
            if index == 0:
                outfile.write('{0}\n'.format(','.join(row_header)))
                
            print("PRINTING ROW VALUES X:\n")
            # NB: the following "for-else-break" structure is to "continue" the outer loop when we meet the "null" (="None" exported) metric.
            for x in row_values:
                print(x + "\n")
                if (x == "null" or x == "None" or x == None or x == "NULL" or x == ''):
                    print("SKIPPING LAST NUMBER, x: " + x)
                    break; 
            else:
                # then we might want to print the actual values
                outfile.write('{0}\n'.format(','.join(row_values)))

    # get all the required predictions
    model = session.execute('SELECT {1} FROM {0}'.format(args.model, ','.join(predictor_columns)))
    with open(os.path.join(root_path, 'prediction_request.txt'), 'w') as outfile:
        for index, values in enumerate(model) :

            # first we need to sorte the key of the result, to enforce consistency
            row_header = sorted(values.keys())
            row_values = [ str(values[x]) for x in row_header ]

            # if it is the first step, we need to print the table header
            if index == 0:
                outfile.write('{0}\n'.format(','.join(row_header)))

            # then we might want to print the actual values
            outfile.write('{0}\n'.format(','.join(row_values)))



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
        dump_cassandra_database(args, root_path)
        sys.exit(os.EX_OK)

    # if we reach this point, we don't know how to dump the information
    sys.exit(os.EX_CONFIG)
