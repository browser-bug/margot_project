#!/bin/bash

# get the root path of the project
if git rev-parse --git-dir > /dev/null 2>&1; then :
	PROJECT_ROOT=$(git rev-parse --show-toplevel)
else :
	>&2 echo "Error: you must execute this script in the benchmark repository"
	exit -1
fi

# load all the paths for margot
source $PROJECT_ROOT/etc/path.env

# print the install folder of the external dependencies
echo -n $PROJECT_ROOT/$DEPS_INSTALL_PATH
