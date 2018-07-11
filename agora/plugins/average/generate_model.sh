#!/bin/bash
ENVIRONMENTAL_FILE=$1
source $ENVIRONMENTAL_FILE

################################################################################
# THIS IS THE PLUGIN ENTRY POINT
#-------------------------------------------------------------------------------
#
# The environmental file provides to this script variables to handle the
# generation of the model model, in particular it exports:
#   - STORAGE_TYPE -> with the type of the supported storage, in the current
#                     implementation it supports only the Cassandra database
#   - STORAGE_ADDRESS -> with the address of the storage
#   - STORAGE_USERNAME -> optionally, the username required to authenticate
#                    with the storage
#   - STORAGE_PASSWORD -> optionally, the password required to authenticate
#                    with the storage
#   - OBSERVATION_CONTAINER_NAME -> the name of the container of the observation
#                    of the application behavior. The columns of this container
#                    are the following (in oder):
#                      - "day": number of days since epoch
#                      - "time": number of nanoseconds since midnight of day
#                      - "client_id": the string if of the client
#                      - "<knob>": the name of each knob of the application
#                      - "<feature>": the name of each feature of the application
#                      - "<metric>": the name of each metric of the application
#   - MODEL_CONTAINER_NAME -> the input/output container of all the required
#                    prediction of the model. The columns of this container are
#                    the following:
#                      - "<knob>": the name of each knob of the application
#                      - "<feature>": the name of each feature of the application
#                      - "<metric_avg>": the expected mean value of each metric
#                      - "<metric_std>": the expected standard deviation of each metric
#                    NOTE: the idea is that the model should updated the <metric_*>
#                    fields of each row of this table.
#   - KNOBS_CONTAINER_NAME -> the container with the name and type of each knob.
#                    The name of the columns are the following:
#                      - "name": the name of the software knob
#                      - "type": the type of the software knob
#   - FEATURES_CONTAINER_NAME -> the container with the name and type of each features.
#                    The name of the columns are the following:
#                      - "name": the name of the feature
#                      - "type": the type of the feature
#   - METRIC_NAME -> the name of the metric to predict
#   - METRIC_ROOT -> the path of this folder when called by agora
#
# Is up to the plugin writer to use this script to call the tools that perform
# the prediction. The remote application handler checks the return value of
# this script to make sure that everything is fine.
# once this script is completed, the remote handler assumes that the prediction
# is compeltely done.
################################################################################


# exit if fail
set -e

# this method is unable to interact directly with a database
# therefore we use a python script to dump and update the database


# STEP 1: execute the model
python3 $METRIC_ROOT/model.py --storage_type "$STORAGE_TYPE" --storage_address "$STORAGE_ADDRESS" --storage_username "$STORAGE_USERNAME" --storage_password "$STORAGE_PASSWORD" --observation_name "$OBSERVATION_CONTAINER_NAME" --features_name "$FEATURES_CONTAINER_NAME" --model_name "$MODEL_CONTAINER_NAME" --knobs_name "$KNOBS_CONTAINER_NAME" --target_metric "$METRIC_NAME" > $METRIC_ROOT/stdout.log 2> $METRIC_ROOT/stderr.log
