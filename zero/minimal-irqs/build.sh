#!/bin/sh

LAUNCHSIZE=$(( 3 * 4096 ))

cd launcher; make;

cd ../forth
xArmCortexDevOSX include zero.ctl

cd .. 

set -- $( ls -l launcher/exe/launcher.bin ) 
LEN=$5

PAD=$(( $LAUNCHSIZE - $LEN )) 

{ 
	cat launcher/exe/launcher.bin;
	dd if=/dev/zero bs=1 count=$PAD;
	cat forth/ZERO.img;
} > packaged.bin


