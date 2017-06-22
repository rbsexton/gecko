#!/bin/sh

LAUNCHSIZE=$(( 2 * 4096 ))
FIRSTBINARY=supervisor/exe/supervisor.bin
SECONDBINARY=forth/TINY.img

cd supervisor; make;
cd .. 


set -- $( ls -l $FIRSTBINARY ) 
LEN=$5

PAD=$(( $LAUNCHSIZE - $LEN )) 

set -- $( ls -l $SECONDBINARY ) 
LEN2=$5

TOT=$(( $LEN + $PAD + $LEN2))

echo "$FIRSTBINARY($LEN) + $PAD + $SECONDBINARY($LEN2) = $TOT"
{ 
	cat $FIRSTBINARY;
	dd if=/dev/zero bs=1 count=$PAD;
	cat $SECONDBINARY;
} > packaged.bin

