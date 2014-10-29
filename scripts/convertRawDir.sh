#!/bin/bash

####
# This script will convert all new RAW files in RAW_DIR
# and save them into a separate mzXML folder.
####

#####
## MAKE SURE TO SET THIS TO MATCH THE PATH TO YOUR READW INSTALLATION
## e.g. PATH_TO_READW=~/.wine/drive_c/ReAdW/
#####
PATH_TO_READW=CHANGE_THIS

# Check if user specified DIR to be converted
#####
if [ $# -lt 1 ]
then
	echo "Usage: convertRawDir.sh <DIR>"
	exit
fi

RAW_DIR=$1
WORK_DIR=`readlink -f $PWD`

# If mzXML directory doesn't exist, create it
#####
if [ ! -d mzXML ]
then
	mkdir mzXML
fi

# Convert and move files to mzXML folder
#####
cd ${RAW_DIR}

for RAW_FILE in `find ./ -iname \*.raw`
do
	BASE_NAME=${RAW_FILE%.raw}
	BASE_NAME=${BASE_NAME%.RAW}
	## Only convert RAW files for which we don't already have an mzXML file
	if [ ! -e ../mzXML/${BASE_NAME}.mzXML ]
	then
		echo "Converting ${RAW_FILE}"
		wine ${PATH_TO_READW}/ReAdW.exe -c --mzXML ${RAW_FILE} ; mv ${BASE_NAME}.mzXML ../mzXML
	fi
done

cd ..
