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
from joblib import dump

import doe
import utils

__author__ = "Bernardo Menicagli"
__license__ = "MIT"
__version__ = "0.0.1"
__status__ = "Development"
__doc__ = "Generate a list of experiments (configurations) based on the selected design algorithm."

parser = argparse.ArgumentParser(description=f'DOE Plugin. {__doc__}')

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

    agora_properties_df = pd.DataFrame()
    knobs_df = pd.DataFrame()
    doe_params_df = pd.DataFrame()
    doe_df = pd.DataFrame()
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
        total_configs_df = doe.create_total_configuration_table(k_values)
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
    doe_df = doe.create_doe(agora_properties, doe_params, k_values)
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
    print(doe_df)

if __name__ == '__main__':
    main()
