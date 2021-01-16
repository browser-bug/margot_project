import os
import sys
import argparse
import pandas as pd
from joblib import dump, load
from dotenv import load_dotenv

from create_model import create_model

__author__ = "Bernardo Menicagli"
__copyright__ = ""
__credits__ = [""]
__license__ = "MIT"
__version__ = "0.0.1"
__maintainer__ = ""
__email__ = ""
__status__ = "Development"

parser = argparse.ArgumentParser(description='Model Plugin')

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
    model_params = model_params_df.set_index('parameter_name').T.to_dict('records')[0]
    agora_properties = agora_properties_df.set_index('property_name').T.to_dict('records')[0]

    # Extract the training data and the target value (and shuffle them)
    data = observation_df[list(k_types.keys()) + list(f_types.keys())].sample(frac=1)
    target = observation_df[metric_name].sample(frac=1)
    if data.empty or target.empty:
        print("No training data or target values available, stopping the model plugin.")
        return

    #TODO: discard incomplete observations? (e.g. with NA values)

    # Label encoding for every knobs/features that are of the "string" type
    encoders = load("../encoders.joblib")
    for k_name, k_type in k_types.items():
        if k_type == 'string':
            data[k_name] = encoders[k_name].transform(data[k_name])

    # Create the Model
    print("Finding a new model for metric:", metric_name)
    is_good,model = create_model(model_params, data, target)

    if is_good or iteration_number >= int(agora_properties['max_number_iteration']):
        print("I'm accepting the model. Dumping estimator data on disk.")
        model.fit(data, target)
        dump(model, model_container)
    else:
        print("Couldn't find a suitable model, waiting for more observations.")

if __name__ == '__main__':
    main()
