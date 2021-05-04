#!/bin/bash
ENVIRONMENTAL_FILE=$1
source $ENVIRONMENTAL_FILE

################################################################################
# THIS IS THE PLUGIN ENTRY POINT
#-------------------------------------------------------------------------------
#
# The environmental file provides to this script variables to handle the
# generation of the Design of Experiments.
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
#                      - "values": every legal values that the software knob can assume
#   - DOE_CONTAINER_NAME -> the container which will store the output DOE.
#                    The name of the columns are the following:
#                      - "config_id": the identification number of the configuration (row)
#                      - "counter": the number of observation that will be taken from this configuration
#                      - "[knob_name]+": for each software knob there will be a coresponding value
#   - DOE_PARAMETERS_CONTAINER_NAME -> the container which will store a list of parameters for the plugin.
#                    The name of the columns are the following:
#                      - "parameter_name": the name of the parameter
#                      - "value": the value of the parameter
#   - TOTAL_CONFIGURATIONS_CONTAINER_NAME -> the container which will a grid of all the possible configurations that the application can have
#                    The name of the columns are the following:
#                      - "[knob_name]+": for each software knob there will be a coresponding value
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

# load data, generate the DOE and write the results based on the storage implementation being used.
$WORKING_DIRECTORY/main.py $CONFIG_FILE_PATH
# if we want to log into files
#$WORKING_DIRECTORY/main.py $CONFIG_FILE_PATH >> $WORKING_DIRECTORY/doe.log 2>> $WORKING_DIRECTORY/doe_err.log
