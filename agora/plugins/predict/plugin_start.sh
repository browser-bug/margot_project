#!/bin/bash
ENVIRONMENTAL_FILE=$1
source $ENVIRONMENTAL_FILE

################################################################################
# THIS IS THE PLUGIN ENTRY POINT
#-------------------------------------------------------------------------------
#
# The environmental file provides to this script variables to handle the
# generation of the final predictions for each EFP.
#   - APPLICATION_NAME -> the name of the application
#   - BLOCK_NAME -> the name of the block of code managed
#   - VERSION -> the version number of the application
#
#   - DESCRIPTION_FS_TYPE -> the storage type for the app description
#   - DOE_FS_TYPE -> the storage type for the app doe
#   - PREDICTION_FS_TYPE -> the storage type for the app prediction
#   - CLUSTER_FS_TYPE -> the storage type for the app cluster
#   - OBSERVATION_FS_TYPE -> the storage type for the app observation
#
#   - DATABASE_ADDRESS -> the address of the database in use (if any)
#   - DATABASE_USERNAME -> [opt] the username required to authenticate with the database (if any)
#   - DATABASE_PASSWORD -> [opt] the password required to authenticate with the database (if any)
#
#   - KNOBS_CONTAINER_NAME -> the container with the name and type of each knob.
#                    The name of the columns are the following:
#                      - "name": the name of the software knob
#                      - "type": the type of the software knob
#   - FEATURES_CONTAINER_NAME -> the container with the name and type of each feature.
#                    The name of the columns are the following:
#                      - "name": the name of the feature
#                      - "type": the type of the feature
#   - METRIC_CONTAINER_NAME -> the container with the name and type of each metric.
#                    The name of the columns are the following:
#                      - "name": the name of the metric
#                      - "type": the type of the metric
#   - TOTAL_CONFIGURATIONS_CONTAINER_NAME -> the container which will a grid of all the possible configurations that the application can have
#                    The name of the columns are the following:
#                      - "[knob_name]+": for each software knob there will be a coresponding value
#   - CLUSTER_CONTAINER_NAME -> the container with the list of cluster centroids found.
#                    The name of the columns are the following:
#                      - "centroid_id": the identification number of the centroid.
#                      - "[feature_name]+": for each feature there will be a coresponding value
#   - PREDICTIONS_CONTAINER_NAME -> the container with all the predictions that will be used to generate the final application knowledge
#                    The name of the columns are the following:
#                      - "prediction_id": the identification number of the prediction.
#                      - "[knob_name]+": for each knob there will be a coresponding value
#                      - "[feature_name]+": for each feature (if any) there will be a coresponding value
#                      - "[metric_name]+": for each metric (the prediction output) there will be a coresponding value
#
#   - WORKING_DIRECTORY -> the plugin working directory.
#   - CONFIG_FILE_PATH -> the plugin environmental configuration file path.
#
# Is up to the plugin writer to use this script to call the tools that perform
# the prediction. The remote application handler checks the return value of
# this script to make sure that everything is fine.
# Once this script is completed, the remote handler assumes that the plugin has finished.
################################################################################


# exit if fail
set -e

# load data, generate the final predictions and write the results based on the storage implementation being used.
$WORKING_DIRECTORY/main.py $CONFIG_FILE_PATH
# if we want to log into files
#$WORKING_DIRECTORY/main.py $CONFIG_FILE_PATH >> $WORKING_DIRECTORY/doe.log 2>> $WORKING_DIRECTORY/doe_err.log
