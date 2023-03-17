#!/bin/bash

IFS=$'\n'

if [ -z $1 ]; then
	echo "Deve indicar um ficheiro wave"
	exit 1;
fi

./sound_meter_ref -i $1 -o $1.ref
../sound_meter -i $1 -o $1.out
cmp $1.ref $1.out

if [ $? -ne 0 ]; then
	exit 1;
fi
