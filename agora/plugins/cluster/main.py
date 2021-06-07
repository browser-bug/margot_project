#!/usr/bin/env python3
#
# Copyright (C) 2021 Bernardo Menicagli
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
# USA
#

import os
import sys
import argparse
import pandas as pd
import uuid
from pathlib import Path
from dotenv import load_dotenv

from cluster import create_cluster
import utils

__author__ = "Bernardo Menicagli"
__license__ = "MIT"
__version__ = "0.0.1"
__status__ = "Development"
__doc__ = "Generate a list of centroids based on an input features clustering."

parser = argparse.ArgumentParser(description=f'Clustering Plugin. {__doc__}')

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
    description_fs_type = os.getenv('DESCRIPTION_FS_TYPE')
    cluster_fs_type = os.getenv('CLUSTER_FS_TYPE')
    observation_fs_type = os.getenv('OBSERVATION_FS_TYPE')

    # Just ignore this for now, we'll add them to the equation once we have a db implementation
    # db_address = os.getenv('DATABASE_ADDRESS')
    # db_username = os.getenv('DATABASE_USERNAME')
    # db_password = os.getenv('DATABASE_PASSWORD')

    features_container = os.getenv('FEATURES_CONTAINER_NAME')
    observation_container = os.getenv('OBSERVATION_CONTAINER_NAME')
    cluster_container = os.getenv('CLUSTER_CONTAINER_NAME')
    cluster_parameters_container = os.getenv('CLUSTER_PARAMETERS_CONTAINER_NAME')

    features_df = pd.DataFrame()
    observation_df = pd.DataFrame()
    cluster_df = pd.DataFrame()
    cluster_params_df = pd.DataFrame()

    # Read tables
    if description_fs_type == 'csv':
        features_df = pd.read_csv(features_container)
        cluster_params_df = pd.read_csv(cluster_parameters_container)
    if observation_fs_type == 'csv':
        observation_df = pd.read_csv(observation_container)

    # generate a dictionary for all the parameters
    cluster_params_dict = cluster_params_df.set_index('parameter_name').T.to_dict('records')
    cluster_params = cluster_params_dict[0] if cluster_params_dict else {}

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
