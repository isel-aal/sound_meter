#!/bin/bash

make -C .. -B

if [ $? -ne 0 ]; then
	exit 1;
fi

./sound_meter_ref -i ../samples/TestNoise.wav -o TestNoise.wav.ref
../build/sound_meter -i ../samples/TestNoise.wav  -o TestNoise.wav.out
cmp TestNoise.wav.ref TestNoise.wav.out

if [ $? -ne 0 ]; then
	exit 1;
fi
