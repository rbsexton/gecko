Minimal MPE Forth for the Silabs/Energy Micro Gecko

This consists of a 'launcher' that initializes the part
and starts the forth environment.  

This particular launcher has interrupt service routines.  
There is a small table of shared data at the very beginning
of memory.

The top level contains a build.sh script that compiles the
launcher and the forth, and combines them together into a
single binary.

