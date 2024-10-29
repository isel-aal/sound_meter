#!/bin/bash

IFS=$'\n'

mkdir -p data
../build/sound_meter -i TestNoise.wav --verbose
cmp data/TestNoise.wav.csv ./TestNoise.wav.csv.ref

if [ $? -ne 0 ]; then
	exit 1;
fi

echo done
