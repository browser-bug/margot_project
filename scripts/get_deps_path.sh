#!/bin/bash

# get the root path of the project
if git rev-parse --git-dir > /dev/null 2>&1; then :
	GIT_REPO_ROOT=$(git rev-parse --show-toplevel)
else :
	>&2 echo "Error: you must execute this script in the benchmark repository"
	exit -1
fi

# load all the paths for margot
source $GIT_REPO_ROOT/etc/path.env

# print the install folder of the external dependencies
echo -n $DEPS_INSTALL_PATH
