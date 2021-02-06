import os
import sys
import argparse
import pandas as pd
from pathlib import Path
from joblib import dump, load
from dotenv import load_dotenv
import uuid

from predict import create_predictions

__author__ = "Bernardo Menicagli"
__copyright__ = ""
__credits__ = [""]
__license__ = "MIT"
__version__ = "0.0.1"
__maintainer__ = ""
__email__ = ""
__status__ = "Development"

parser = argparse.ArgumentParser(description='Predict Plugin')

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
    doe_fs_type = os.getenv('DOE_FS_TYPE')
    cluster_fs_type = os.getenv('CLUSTER_FS_TYPE')
    prediction_fs_type = os.getenv('PREDICTION_FS_TYPE')

    # Just ignore this for now, we'll add them to the equation once we have a db implementation
    # db_address = os.getenv('DATABASE_ADDRESS')
    # db_username = os.getenv('DATABASE_USERNAME')
    # db_password = os.getenv('DATABASE_PASSWORD')

    agora_properties_container = os.getenv('AGORA_PROPERTIES_CONTAINER_NAME')
    knobs_container = os.getenv('KNOBS_CONTAINER_NAME')
    features_container = os.getenv('FEATURES_CONTAINER_NAME')
    metrics_container = os.getenv('METRICS_CONTAINER_NAME')
    total_configurations_container = os.getenv('TOTAL_CONFIGURATIONS_CONTAINER_NAME')
    cluster_container = os.getenv('CLUSTER_CONTAINER_NAME')
    predictions_container = os.getenv('PREDICTIONS_CONTAINER_NAME')
    models_container = os.getenv('MODELS_CONTAINER')

    agora_properties_df = pd.DataFrame()
    knobs_df = pd.DataFrame()
    features_df = pd.DataFrame()
    metrics_df = pd.DataFrame()
    total_configs_df = pd.DataFrame()
    cluster_df = pd.DataFrame()
    prediction_df = pd.DataFrame()

    # Read tables
    if description_fs_type == 'csv':
        agora_properties_df = pd.read_csv(agora_properties_container)
        knobs_df = pd.read_csv(knobs_container)
        features_df = pd.read_csv(features_container)
        metrics_df = pd.read_csv(metrics_container)
    if doe_fs_type == 'csv':
        total_configs_df = pd.read_csv(total_configurations_container)
    if cluster_fs_type == 'csv':
        if not features_df.empty:
            cluster_df = pd.read_csv(cluster_container)

    # Create a dictionary: {knob_name, knob_type}
    k_types = {}
    for row in knobs_df.itertuples(index=False):
        k_types[row[0]] = row[1]
    # Create a dictionary: {metric_name, metric_type}
    m_types = {}
    for row in metrics_df.itertuples(index=False):
        m_types[row[0]] = row[1]

    # generate a dictionary for all the parameters
    agora_properties_dict = agora_properties_df.set_index('property_name').T.to_dict('records')
    agora_properties = agora_properties_dict[0] if agora_properties_dict else {}

    # Generate the sample dataset for the final prediction
    samples_df = total_configs_df
    if not features_df.empty:
        samples_df = pd.merge(samples_df, cluster_df, 'cross')
        del samples_df['centroid_id']

    # Label encoding for every knobs that are of the "string" type
    # Save a local copy and destroy the global one
    encoders_path = Path("encoders.joblib")
    if not encoders_path.exists():
        global_encoders_path = Path("../encoders.joblib")
        encoders = load(global_encoders_path)
        global_encoders_path.unlink(missing_ok=True)
        dump(encoders, "encoders.joblib")
    else:
        encoders = load(encoders_path)

    for k_name, k_type in k_types.items():
        if k_type == 'string':
            samples_df[k_name] = encoders[k_name].transform(samples_df[k_name])

    # Load up the models for each metric
    print("Loading up the models.")
    models = {}
    for m_name in m_types.keys():
        model_path = Path(models_container) / (m_name + "_model.data")
        models[m_name] = load(model_path)

    # Produce the predictions
    prediction_df = create_predictions(samples_df, models)

    # Transform labels back to original encoding
    for k_name, k_type in k_types.items():
        if k_type == 'string':
            samples_df[k_name] = encoders[k_name].inverse_transform(samples_df[k_name])

    # Join with coresponding configuration row and add a prediction_id
    prediction_df = samples_df.join(prediction_df)
    prediction_df.insert(0, 'pred_id', [ uuid.uuid4() for _ in prediction_df.index])
    print(prediction_df)

    # Store the predictions table
    print("Storing the predictions.")
    if prediction_fs_type == 'csv':
        output_dir = Path(predictions_container).parent
        if not output_dir.exists():
            output_dir.mkdir(parents=True, exist_ok=True)
        prediction_df.to_csv(predictions_container, index=False)

if __name__ == '__main__':
    main()
