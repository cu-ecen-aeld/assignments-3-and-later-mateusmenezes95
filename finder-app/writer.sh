#!/bin/sh

if [ "$#" -lt 2 ]; then
	echo "You must specify the writefile and writestr arguments, respectively"
	exit 1
fi

writefile="$1"
writestr="$2"

mkdir -p $(dirname "$writefile")
touch "$writefile"

if [ ! -f "$writefile" ]; then
	echo "$writefile not created!"
	exit 1
fi

echo "$writestr" > "$writefile"

exit 0

