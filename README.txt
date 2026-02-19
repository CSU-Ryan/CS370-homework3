Files in this project:

+-- Makefile   [builds and cleans code binaries.]
+-- README.txt [This file.]
+-- bin \ [
\-- src
    +-- Checker.c     [Given a divisor and dividend, returns if the divisor divides the dividend.]
    \-- Coordinator.c [Given a dividend and four divisors,
                       creates Checker processes to determine if each divisor divides the dividend.]


To execute, first build the binaries by running
    $ make all

Then run with desired inputs
    $ bin/coordinator.o <dividend> <divisor1> <divisor2> <divisor3> <divisor4>

To remove binaries, run
    $ make clean
