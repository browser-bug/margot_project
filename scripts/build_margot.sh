#!/bin/bash

#-------------- CONFIGURING GLOBAL VARIABLES

# get the root path of the project
if git rev-parse --git-dir > /dev/null 2>&1; then :
	GIT_REPO_ROOT=$(git rev-parse --show-toplevel)
else :
	>&2 echo "Error: you must execute this script in the mARGOt repository"
	exit -1
fi

# load all the paths for margot
source $GIT_REPO_ROOT/etc/path.env

# define the path for the stdout and stderr files for logging
STDOUT_FILE=$PWD/stdout.txt
STDERR_FILE=$PWD/stderr.txt


# compile each component of the mARGOt framework
for COMPONENT_NAME in margot heel
do
	# define the component specific paths
	COMPONENT_BUILD_PATH=$GIT_REPO_ROOT/$COMPONENT_NAME/build

	# create the build folders
	echo "Creating the build folders for ${COMPONENT_NAME}..."
	mkdir -p $COMPONENT_BUILD_PATH > $STDOUT_FILE 2> $STDERR_FILE
	if [ ! $? -eq 0 ]; then
		>&2 echo "Error: unable to create the folder \"$DEPENDENCY_SRC\""
		>&2 echo "       standard output file: $STDOUT_FILE"
		>&2 echo "       standard error file:  $STDERR_FILE"
		exit -1
	fi

	# configure step of the dependency
	if [ ! -f $DEPENDENCY_BUILD_PATH/Makefile ]; then
		echo "Configuring ${COMPONENT_NAME}..."
		cd $COMPONENT_BUILD_PATH && cmake -DCMAKE_INSTALL_PREFIX:PATH=$MARGOT_INSTALL_PATH -DCMAKE_BUILD_TYPE:STRING=Release .. > $STDOUT_FILE 2> $STDERR_FILE
		if [ ! -f $COMPONENT_BUILD_PATH/Makefile ]; then
			>&2 echo "Error: unable to generate the $COMPONENT_NAME building files"
			>&2 echo "       standard output file: $STDOUT_FILE"
			>&2 echo "       standard error file:  $STDERR_FILE"
			exit -1
		fi
	fi


	# building && installing the dependency
	echo "Building ${COMPONENT_NAME}..."
	make -C $COMPONENT_BUILD_PATH install > $STDOUT_FILE 2> $STDERR_FILE
	if [ $? -ne 0 ]; then
		>&2 echo "Error: the compilation of $COMPONENT_NAME returned a non-zero value"
		>&2 echo "       standard output file: $STDOUT_FILE"
		>&2 echo "       standard error file:  $STDERR_FILE"
		exit -1
	fi


	# remove the log files
	rm -f $STDOUT_FILE
	rm -f $STDERR_FILE

done
