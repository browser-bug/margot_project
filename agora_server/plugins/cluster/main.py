import os
import sys
import argparse
import pandas as pd
import uuid
from pathlib import Path
from dotenv import load_dotenv

from create_cluster import create_cluster
import utils

__author__ = "Bernardo Menicagli"
__copyright__ = ""
__credits__ = [""]
__license__ = "MIT"
__version__ = "0.0.1"
__maintainer__ = ""
__email__ = ""
__status__ = "Development"

parser = argparse.ArgumentParser(description='Clustering Plugin')

# Positional Arguments
parser.add_argument('plugin_config_path',
                    help="the environmental configuration file path"
                    )

args = parser.parse_args()

def main():
    config_file_path = sys.argv[1]
    load_dotenv(config_file_path, verbose=True)

    # set the current working directory
    plugin_working_dir = os.path.dirname(os.path.realpath(__file__))
    os.chdir(plugin_working_dir)

    # Accessing env variables
    app_name = os.getenv('APPLICATION_NAME')
    block_name = os.getenv('BLOCK_NAME')
    # version = os.getenv('VERSION')

    description_fs_type = os.getenv('DESCRIPTION_FS_TYPE')
    cluster_fs_type = os.getenv('CLUSTER_FS_TYPE')
    observation_fs_type = os.getenv('OBSERVATION_FS_TYPE')

    # Just ignore this for now, we'll add them to the equation once we have a db implementation
    # db_address = os.getenv('DATABASE_ADDRESS')
    # db_username = os.getenv('DATABASE_USERNAME')
    # db_password = os.getenv('DATABASE_PASSWORD')

    agora_properties_container = os.getenv('AGORA_PROPERTIES_CONTAINER_NAME')
    features_container = os.getenv('FEATURES_CONTAINER_NAME')
    observation_container = os.getenv('OBSERVATION_CONTAINER_NAME')
    cluster_container = os.getenv('CLUSTER_CONTAINER_NAME')
    cluster_parameters_container = os.getenv('CLUSTER_PARAMETERS_CONTAINER_NAME')

    # Printing env variables (debug)
    # print(app_name)
    # print(block_name)
    # print(description_fs_type)
    # print(cluster_fs_type)
    # print(cluster_container)
    # print(cluster_parameters_container)

    agora_properties_df = pd.DataFrame()
    features_df = pd.DataFrame()
    observation_df = pd.DataFrame()
    cluster_df = pd.DataFrame()
    cluster_params_df = pd.DataFrame()

    # Read tables
    if description_fs_type == 'csv':
        agora_properties_df = pd.read_csv(agora_properties_container)
        features_df = pd.read_csv(features_container)
        cluster_params_df = pd.read_csv(cluster_parameters_container)
    if observation_fs_type == 'csv':
        observation_df = pd.read_csv(observation_container)

    # generate a dictionary for all the parameters
    cluster_params_dict = cluster_params_df.set_index('parameter_name').T.to_dict('records')
    cluster_params = cluster_params_dict[0] if cluster_params_dict else {}
    agora_properties_dict = agora_properties_df.set_index('property_name').T.to_dict('records')
    agora_properties = agora_properties_dict[0] if agora_properties_dict else {}
    print(cluster_params)

    # Create a dictionary: {feature_name, feature_type}
    f_types = {}
    for row in features_df.itertuples(index=False):
        f_types[row[0]] = row[1]

    # Extract the features matrix
    features_matrix = observation_df[f_types.keys()]

    # Create the Cluster
    cluster_df = create_cluster(cluster_params, features_matrix)
    cluster_df = cluster_df.round(decimals=4).drop_duplicates()
    cluster_df.columns = f_types.keys()
    utils.convert_types(cluster_df, f_types)
    print(cluster_df)

    # Add an unique_id for each centroid
    cluster_df.insert(0, 'centroid_id', [ uuid.uuid4() for _ in cluster_df.index])

    if cluster_fs_type == 'csv':
        output_dir = Path(cluster_container).parent
        if not output_dir.exists():
            output_dir.mkdir(parents=True, exist_ok=True)
        cluster_df.to_csv(cluster_container, index=False)

    print("New clusters have been created.")

if __name__ == '__main__':
    main()
