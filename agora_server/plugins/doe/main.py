import os
import sys
import argparse
import pandas as pd
import uuid
from pathlib import Path
from dotenv import load_dotenv
from joblib import dump

from total_config import create_total_configuration_table
from create_doe import create_doe
import utils

__author__ = "Bernardo Menicagli"
__copyright__ = ""
__credits__ = [""]
__license__ = "MIT"
__version__ = "0.0.1"
__maintainer__ = ""
__email__ = ""
__status__ = "Development"

parser = argparse.ArgumentParser(description='DOE Plugin')

# Positional Arguments
parser.add_argument('plugin_config_path',
                    help="the environmental configuration file path"
                    )

args = parser.parse_args()

def main():
    config_file_path = sys.argv[1]
    load_dotenv(config_file_path, verbose=True)

    # set the current working directory
    plugin_working_dir = Path(__file__).parent
    os.chdir(plugin_working_dir)

    # Accessing env variables
    app_name = os.getenv('APPLICATION_NAME')
    block_name = os.getenv('BLOCK_NAME')
    # version = os.getenv('VERSION')

    description_fs_type = os.getenv('DESCRIPTION_FS_TYPE')
    doe_fs_type = os.getenv('DOE_FS_TYPE')

    # Just ignore this for now, we'll add them to the equation once we have a db implementation
    # db_address = os.getenv('DATABASE_ADDRESS')
    # db_username = os.getenv('DATABASE_USERNAME')
    # db_password = os.getenv('DATABASE_PASSWORD')

    agora_properties_container = os.getenv('AGORA_PROPERTIES_CONTAINER_NAME')
    knobs_container = os.getenv('KNOBS_CONTAINER_NAME')
    doe_container = os.getenv('DOE_CONTAINER_NAME')
    doe_parameters_container = os.getenv('DOE_PARAMETERS_CONTAINER_NAME')
    total_configurations_container = os.getenv('TOTAL_CONFIGURATIONS_CONTAINER_NAME')

    # Printing env variables (debug)
    # print(app_name)
    # print(block_name)
    # print(description_fs_type)
    # print(doe_fs_type)
    # print(knobs_container)
    # print(doe_container)
    # print(doe_parameters_container)
    # print(total_configurations_container)

    agora_properties_df = pd.DataFrame()
    knobs_df = pd.DataFrame()
    doe_df = pd.DataFrame()
    doe_params_df = pd.DataFrame()
    total_configs_df = pd.DataFrame()

    # Read tables
    if description_fs_type == 'csv':
        agora_properties_df = pd.read_csv(agora_properties_container)
        knobs_df = pd.read_csv(knobs_container)
        doe_params_df = pd.read_csv(doe_parameters_container)

    # Create a dictionary: {knob_name, [knob_values]}
    # Create a dictionary: {knob_name, knob_type}
    k_values = {}
    k_types = {}
    for row in knobs_df.itertuples(index=False):
        k_values[row[0]] = row[2].split(';')
        k_types[row[0]] = row[1]

    # Defining levels, array of integers that defines the ranges for each factor
    levels = []
    factors = []
    for key in k_values:
        levels.append(len(k_values[key]))
        factors.append(k_values[key])
    print(levels)
    print(factors)

    # generate a dictionary for all the parameters
    doe_params_dict = doe_params_df.set_index('parameter_name').T.to_dict('records')
    doe_params = doe_params_dict[0] if doe_params_dict else {}
    agora_properties_dict = agora_properties_df.set_index('property_name').T.to_dict('records')
    agora_properties = agora_properties_dict[0] if agora_properties_dict else {}

    # if not already, create the total configurations table for later use
    if Path(total_configurations_container).is_file():
        print("Total configurations table already exists, loading it up.")
        if description_fs_type == 'csv':
            total_configs_df = pd.read_csv(total_configurations_container)
    else:
        total_configs_df = create_total_configuration_table(levels, factors)
        total_configs_df.columns = k_values.keys()
        utils.convert_types(total_configs_df, k_types)

        # filtering
        constraints = doe_params['constraint'].split(';') if 'constraint' in doe_params.keys() else []
        limit_query = ' and '.join(constraints)
        if limit_query:
            total_configs_df = total_configs_df.query(limit_query)

        # create a LabelEncoder for each knob string type and save it globally
        encoders = utils.encode_data(total_configs_df, k_types)
        dump(encoders, "../encoders.joblib")

        # write the df down
        if description_fs_type == 'csv':
            output_dir = Path(total_configurations_container).parent
            output_dir.mkdir(parents=True, exist_ok=True)
            total_configs_df.to_csv(total_configurations_container, index=False)

    # Create the DOE
    doe_df = create_doe(agora_properties, doe_params, levels, factors)
    doe_df.columns = k_values.keys()
    utils.convert_types(doe_df, k_types)

    # Remove configurations which don't verify the user limits
    doe_df = pd.merge(doe_df, total_configs_df, how='inner')

    # Add an unique_id and a counter (num observation to make) for each configuration
    num_obs_per_config = int(agora_properties['number_observations_per_configuration'])
    doe_df.insert(0, 'counter', num_obs_per_config)
    doe_df.insert(0, 'config_id', [ uuid.uuid4() for _ in doe_df.index])

    if doe_fs_type == 'csv':
        output_dir = Path(doe_container).parent
        if not output_dir.exists():
            output_dir.mkdir(parents=True, exist_ok=True)
        doe_df.to_csv(doe_container, index=False)

    print("A new DOE has been created.")

if __name__ == '__main__':
    main()
