## FIFO example

Program demonstrates communitating via named pipe (fifo file).

Fist program (observer.c) opens fifo file and observes it until size of data inside pipe is < 600.

Second program (sender.c) generates random data (1-4 bytes) and writes it to pipe. Program ignores SIGPIPE signal, but reacts on EPIPE error.

### To compile in Linux:

gcc -Wall observer.c -o observer.out

gcc -Wall sender.c -o sender.out

./observer.out fifo

./sender.out fifo
