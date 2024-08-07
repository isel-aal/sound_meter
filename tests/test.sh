#!/bin/bash

IFS=$'\n'

if [ -z $1 ]; then
	echo "Deve indicar um ficheiro wave"
	exit 1;
fi

../build/sound_meter -i $1
cmp data/$1.csv ./$1.ref

if [ $? -ne 0 ]; then
	exit 1;
fi

echo done
