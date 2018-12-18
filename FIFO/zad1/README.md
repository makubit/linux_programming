## FIFO example

Program demonstrates communicating via named pipe (fifo file).

First program (zad1_a.c) writes to fifo file letters from A to Z in 32kB blocks every 1.2 secend.
Second program (zad1_b.c) reads data from fifo file in 16kB blocks every 1.6 second.
The output tells how long programs are waiting for resource and how many bytes has been written or read.
