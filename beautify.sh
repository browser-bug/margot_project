#!/bin/bash

# check if astyle is available
astyle --version > /dev/null
rc=$?; if [[ $rc != 0 ]]; then echo "Please, install astyle" && exit $rc; fi

# creating the astyle configuration file
ASTYLE_FILE_NAME=$(mktemp astyle.XXXXXXXXX.conf)
cat <<EOT > $ASTYLE_FILE_NAME
style=bsd
indent=spaces=2
indent-classes
indent-switches
indent-namespaces
indent-preproc-block
indent-col1-comments
min-conditional-indent=0
break-blocks
pad-oper
pad-header
align-pointer=type
align-reference=type
add-brackets
max-code-length=200
break-after-logical
EOT

# beautify the files
astyle --suffix=none --recursive --options=$ASTYLE_FILE_NAME *.hpp
astyle --suffix=none --recursive --options=$ASTYLE_FILE_NAME *.cc

# remove the configuration file
rm $ASTYLE_FILE_NAME
