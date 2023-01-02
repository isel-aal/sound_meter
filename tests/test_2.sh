#!/bin/bash

make -C .. -B

if [ $? -ne 0 ]; then
	exit 1;
fi

./sound_meter_ref -i ../sound_samples/TestNoise.wav -o TestNoise.wav.ref
../build/sound_meter -g sound_meter_test.conf -i ../sound_samples/TestNoise.wav -o TestNoise.wav.out
cmp TestNoise.wav.ref TestNoise.wav.out

if [ $? -ne 0 ]; then
	exit 1;
fi
