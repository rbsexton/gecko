A more complex app for the Zero Gecko.

Umbilical Forth Edition

* Launcher

This consists of a 'launcher' that initializes the part
and starts the forth environment.  

This particular launcher has interrupt service routines.  
There is a small table of shared data at the very beginning
of memory.   Interconnect.fth contains routines for convenient
retrieval of symbols from the interconnect function table.

The top level contains a build.sh script that compiles the
launcher and the forth, and combines them together into a
single binary.
