#!/bin/bash

DEPENDENCY_NAME="PahoMQTT"

#############################################
### PLEASE, DO NOT MODIFY UNDER THIS MARK ###
#############################################


#-------------- CONFIGURING GLOBAL VARIABLES

# get the root path of the project
if git rev-parse --git-dir > /dev/null 2>&1; then :
	PROJECT_ROOT=$(git rev-parse --show-toplevel)
else :
	>&2 echo "Error: you must execute this script in the mARGOt repository"
	exit -1
fi

# load all the paths for margot
source $PROJECT_ROOT/etc/path.env

# compose the path to the source code of the dependency
DEPENDENCY_SRC=$PROJECT_ROOT/$DEPS_SRC_PATH/$DEPENDENCY_NAME
DEPENDENCY_BUILD_PATH=$DEPENDENCY_SRC/build
DEPENDENCY_INSTALL_PATH=$PROJECT_ROOT/$DEPS_INSTALL_PATH

# define the path for the stdout and stderr files for logging
STDOUT_FILE=$PWD/stdout.txt
STDERR_FILE=$PWD/stderr.txt

# retrieveing the source code of the dependency
if [ ! -d $DEPENDENCY_SRC ]; then
	echo "Cloning the $DEPENDENCY_NAME framework..."
	git clone https://github.com/eclipse/paho.mqtt.c.git $DEPENDENCY_SRC --branch v1.3.1 > $STDOUT_FILE 2> $STDERR_FILE
	if [ ! $? -eq 0 ]; then
		>&2 echo "Error: unable to download the $DEPENDENCY_NAME sources"
		>&2 echo "       standard output file: $STDOUT_FILE"
		>&2 echo "       standard error file:  $STDERR_FILE"
		exit -1
	fi
fi

# creating the build folder
echo "Creating the folders for the $DEPENDENCY_NAME dependency..."
mkdir -p $DEPENDENCY_BUILD_PATH > $STDOUT_FILE 2> $STDERR_FILE
if [ ! $? -eq 0 ]; then
	>&2 echo "Error: unable to create the folder \"$DEPENDENCY_SRC\""
	>&2 echo "       standard output file: $STDOUT_FILE"
	>&2 echo "       standard error file:  $STDERR_FILE"
	exit -1
fi


# configure step of the dependency
if [ ! -f $DEPENDENCY_BUILD_PATH/Makefile ]; then
	echo "Configuring the $DEPENDENCY_NAME dependency..."
	cd $DEPENDENCY_BUILD_PATH && cmake -DPAHO_BUILD_STATIC:STRING=TRUE -DCMAKE_INSTALL_PREFIX:PATH=$DEPENDENCY_INSTALL_PATH -DCMAKE_BUILD_TYPE:STRING=RELEASE -DPAHO_WITH_SSL=TRUE .. > $STDOUT_FILE 2> $STDERR_FILE
	if [ ! -f $DEPENDENCY_BUILD_PATH/Makefile ]; then
		>&2 echo "Error: unable to generate the $DEPENDENCY_NAME building files"
		>&2 echo "       standard output file: $STDOUT_FILE"
		>&2 echo "       standard error file:  $STDERR_FILE"
		exit -1
	fi
fi


# building && installing the dependency
echo "Building the $DEPENDENCY_NAME dependency..."
make -C $DEPENDENCY_BUILD_PATH install > $STDOUT_FILE 2> $STDERR_FILE
if [ ! -d $OPENCV_INSTALL_PATH ]; then
	>&2 echo "Error: unable to compile the $DEPENDENCY_NAME framework"
	>&2 echo "       standard output file: $STDOUT_FILE"
	>&2 echo "       standard error file:  $STDERR_FILE"
	exit -1
fi


# remove the log files
rm -f $STDOUT_FILE
rm -f $STDERR_FILE
