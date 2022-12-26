gcc -w -std=c99 -c UEBp1-ser.c UEBp1-aUEBs.c UEBp1-tTCP.c
gcc -w -std=c99 -o server UEBp1-ser.o UEBp1-aUEBs.o UEBp1-tTCP.o
./server