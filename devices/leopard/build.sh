#!/bin/sh

FIRSTBINARY=usbsupervisor/exe/usbdcomposite.bin
LAUNCHSIZE=$(( 64 * 1024 ))
SECONDBINARY=usbforth/LEOPARD.img

cd usbsupervisor; make; cd .. 

set -- $( ls -l $FIRSTBINARY ); LEN=$5
PAD=$(( $LAUNCHSIZE - $LEN )) 
set -- $( ls -l $SECONDBINARY ); LEN2=$5
TOT=$(( $LEN + $PAD + $LEN2))

echo "$FIRSTBINARY($LEN) + $PAD + $SECONDBINARY($LEN2) = $TOT"
{ 
	cat $FIRSTBINARY;
	dd if=/dev/zero bs=1 count=$PAD;
	cat $SECONDBINARY;
} > packaged.bin

# Generate a .o file for use with gdb & the Black Magic probe.
arm-none-eabi-objcopy -O elf32-littlearm \
	-B arm\
	--rename-section .data=.text\
	-I binary packaged.bin\
	packaged.elf


