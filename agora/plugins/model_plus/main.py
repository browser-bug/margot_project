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
from joblib import dump, load
from dotenv import load_dotenv
from pathlib import Path

from model import create_model

__author__ = "Bernardo Menicagli"
__license__ = "MIT"
__version__ = "0.0.1"
__status__ = "Development"
__doc__ = "Generate a predicting model for the target EFP using the best model selected among a list of them."

parser = argparse.ArgumentParser(description=f'Modelling Plugin v2. {__doc__}')

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
    observation_fs_type = os.getenv('OBSERVATION_FS_TYPE')

    # Just ignore this for now, we'll add them to the equation once we have a db implementation
    # db_address = os.getenv('DATABASE_ADDRESS')
    # db_username = os.getenv('DATABASE_USERNAME')
    # db_password = os.getenv('DATABASE_PASSWORD')

    agora_properties_container = os.getenv('AGORA_PROPERTIES_CONTAINER_NAME')
    iteration_number = int(os.getenv('ITERATION_NUMBER'))
    metric_name = os.getenv('METRIC_NAME')
    knobs_container = os.getenv('KNOBS_CONTAINER_NAME')
    features_container = os.getenv('FEATURES_CONTAINER_NAME')
    observation_container = os.getenv('OBSERVATION_CONTAINER_NAME')
    model_container = os.getenv('MODEL_CONTAINER_NAME')
    model_parameters_container = os.getenv('MODEL_PARAMETERS_CONTAINER_NAME')

    agora_properties_df = pd.DataFrame()
    knobs_df = pd.DataFrame()
    features_df = pd.DataFrame()
    model_params_df = pd.DataFrame()
    observation_df = pd.DataFrame()

    # Read tables
    if description_fs_type == 'csv':
        agora_properties_df = pd.read_csv(agora_properties_container)
        knobs_df = pd.read_csv(knobs_container)
        features_df = pd.read_csv(features_container)
        model_params_df = pd.read_csv(model_parameters_container, quotechar="\"")
    if observation_fs_type == 'csv':
        observation_df = pd.read_csv(observation_container)

    # Create a dictionary: {knob_name, knob_type}
    k_types = {}
    for row in knobs_df.itertuples(index=False):
        k_types[row[0]] = row[1]
    # Create a dictionary: {feature_name, feature_type}
    f_types = {}
    for row in features_df.itertuples(index=False):
        f_types[row[0]] = row[1]

    # generate a dictionary for all the parameters
    model_params_dict = model_params_df.set_index('parameter_name').T.to_dict('records')
    model_params = model_params_dict[0] if model_params_dict else {}
    agora_properties_dict = agora_properties_df.set_index('property_name').T.to_dict('records')
    agora_properties = agora_properties_dict[0] if agora_properties_dict else {}

    # shuffle and extract the training data and the target value
    observation_df = observation_df.sample(frac=1)
    data = observation_df[list(k_types.keys()) + list(f_types.keys())]
    target = observation_df[metric_name]
    if data.empty or target.empty:
        print("No training data or target values available, stopping the model plugin.")
        return

    # Label encoding for every knobs that has a "string" type. Then save a local copy.
    encoders_path = Path("encoders.joblib")
    if not encoders_path.exists():
        encoders = load("../encoders.joblib")
        dump(encoders, encoders_path)
    else:
        encoders = load(encoders_path)
    for k_name, k_type in k_types.items():
        if k_type == 'string':
            data[k_name] = encoders[k_name].transform(data[k_name])

    # Create the Model
    print("Finding a new model for metric:", metric_name)
    is_last_iteration = iteration_number >= int(agora_properties['max_number_iteration'])
    is_good,model = create_model(metric_name, iteration_number, model_params, data, target, is_last_iteration)

    if is_good or is_last_iteration:
        if is_good:
            print("Model is good! Dumping estimator data on disk.")
        else:
            print("Max. number of iterations reached. Using the best model I've found. Dumping estimator data on disk.")
        output_dir = Path(model_container).parent
        if not output_dir.exists():
            output_dir.mkdir(parents=True, exist_ok=True)
        dump(model, model_container)
    else:
        print("Couldn't find a suitable model, waiting for more observations.")

if __name__ == '__main__':
    main()

