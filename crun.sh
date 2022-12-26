gcc -w -std=c99 -c UEBp1-cli.c UEBp1-aUEBc.c UEBp1-tTCP.c UEBp2-aDNSc.c
gcc -w -std=c99 -o client UEBp1-cli.o UEBp1-aUEBc.o UEBp1-tTCP.o UEBp2-aDNSc.o
./client