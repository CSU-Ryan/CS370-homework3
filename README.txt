Files in this project:

+-- Makefile   [builds and cleans code binaries.]
+-- README.txt [This file.]
+-- Checker.c     [Given a divisor and dividend,
|                  writes to shared memory if the divisor divides the dividend.]
\-- Coordinator.c [Given a divisor and four dividends,
                   creates Checker processes to determine if each divisor divides the dividend.]


To execute, first build the binaries by running
    $ make all

Then run with desired inputs
    $ coordinator <divisor> <dividend1> <dividend2> <dividend3> <dividend4>

To remove binaries, run
    $ make clean
