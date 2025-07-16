#!/bin/sh

if [ "$#" -lt 2 ]; then
	echo "You must specify the filedir and searchsrt arguments, respectively"
	exit 1
fi

filesdir="$1"
searchstr="$2"

echo "[finder.sh]: Searching in the file '$filesdir' the string '$searchstr'\n"

if [ ! -d "$filesdir" ]; then
	echo "$1 is not a directory!"
	exit 1
fi

number_of_files=$(find "$filesdir" -type f | wc -l)
number_of_matches=$(grep -rwi "$searchstr" "$filesdir" | wc -l)

echo "The number of files are $number_of_files and the number of matching lines are $number_of_matches"

exit 0

