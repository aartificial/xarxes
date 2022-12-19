#include "UEBp1-aUEBs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_PATH "./ser.cfg"
#define LOG_PATH "./ser.log"
#define NOMBRECONNSMAX		4

int AfegeixSck(int Sck, int *LlistaSck, int LongLlistaSck);
int TreuSck(int Sck, int *LlistaSck, int LongLlistaSck);
void log_message(char *message);
void read_config(int* port, char* source);
void close_sck(int SckEsc);

void print_sockets(const int *LlistaSck) {
    for (int i =0; i < NOMBRECONNSMAX; i++)
        printf("%d ", LlistaSck[i]);
}

int main() {
    int SckEsc, SckCon, portTCPser, portTCPcli;
    char MisRes[200], IPser[16], IPcli[16], source[200];

    int LlistaSck[NOMBRECONNSMAX] = {-1};

    read_config(&portTCPser, source);
    FILE *fitxer = fopen(LOG_PATH, "w");
    fprintf(fitxer, "");
    fclose(fitxer);

    int ini_s = UEBs_IniciaServ(&SckEsc, portTCPser, MisRes, source);
    printf("%s\n", MisRes);
    log_message(MisRes);
    if (ini_s != 0)
        return -1;

    int breakpoint = 0;
    while (!breakpoint) {
        char TipusPeticio[4], NomFitx[9999] = {0};

        SckCon = UEBs_AcceptaConnexio(SckEsc, IPser, &portTCPser, IPcli, &portTCPcli, MisRes);
        printf("%s\n", MisRes);
        log_message(MisRes);
        if (SckCon < 0)
            continue;

        printf("[DEBUG] Adding socket %d.\n", SckCon);
        AfegeixSck(SckCon, LlistaSck, NOMBRECONNSMAX);
        printf("[DEBUG] Socket %d added.\n", SckCon);
        print_sockets(LlistaSck);
        printf("[DEBUG] Waiting for socket.\n");
        SckCon = UEBs_HaArribatAlgunaCosa(LlistaSck, NOMBRECONNSMAX, MisRes);
        printf("[DEBUG] Socket recieved %d.\n", SckCon);
        printf("[DEBUG] Removing socket %d.\n", SckCon);
        TreuSck(SckCon, LlistaSck, NOMBRECONNSMAX);
        printf("[DEBUG] Socket %d removed.\n", SckCon);
        print_sockets(LlistaSck);

        int pet_s = UEBs_ServeixPeticio(SckCon, TipusPeticio, NomFitx, MisRes);
        printf("%s\n", MisRes);
        log_message(MisRes);
        if (pet_s != 0) {
            close_sck(SckCon);
            if (pet_s == -3)
                breakpoint = 1;
            continue;
        }
        close_sck(SckCon);
    }
    close_sck(SckEsc);
}

void close_sck(int SckEsc) {
    char MisRes[200];
    UEBs_TancaConnexio(SckEsc, MisRes);
    printf("%s", MisRes);
    log_message(MisRes);
}

void log_message(char *message) {
    FILE *fitxer = fopen(LOG_PATH, "a");
    fprintf(fitxer, "%s\n", message);
    fclose(fitxer);
}

void read_config(int* port, char* source) {
    char debug_path[200] = "/mnt/c/Users/jiesa/CLionProjects/xarxes/p2/server/ser.cfg";
    FILE* fitxer = fopen(debug_path, "r");
    char data[400];
    if (fitxer != NULL) {
        fread(data, 200, 2, fitxer);
        int i = 0;
        if (data[i] == '#') {
            i+=9;
            *port = atoi(&data[9]);
            while (data[i] != '#')
                i++;
        }
        if (data[i] == '#') {
            i+=7;
            strcpy(source, &data[i]);
        }
        fclose(fitxer);
    }
}

/* Donada la llista d'identificadors de sockets “LlistaSck” (de longitud  */
/* “LongLlistaSck” sockets), hi busca una posició "lliure" (una amb un    */
/* contingut igual a -1) i hi escriu l'identificador de socket "Sck".     */
/*                                                                        */
/* "LlistaSck" és un vector d'int d'una longitud d'almenys LongLlistaSck. */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si tot va bé;                                                       */
/* -1 si hi ha error.                                                     */
int AfegeixSck(int Sck, int *LlistaSck, int LongLlistaSck) {
    for (int i = 0; i<LongLlistaSck; i++) {
        if (LlistaSck[i] == -1) {
            LlistaSck[i] = Sck;
            return 0;
        }
    }
    return -1;
}

/* Donada la llista d'identificadors de sockets “LlistaSck” (de longitud  */
/* “LongLlistaSck” sockets), hi busca la posició on hi ha l'identificador */
/* de socket "Sck" i la marca com "lliure" (hi escriu un contingut igual  */
/* a -1).                                                                 */
/*                                                                        */
/* "LlistaSck" és un vector d'int d'una longitud d'almenys LongLlistaSck. */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si tot va bé;                                                       */
/* -1 si hi ha error.                                                     */
int TreuSck(int Sck, int *LlistaSck, int LongLlistaSck) {
    for (int i = 0; i < LongLlistaSck; i++) {
        if (LlistaSck[i] == Sck) {
            LlistaSck[i] = -1;
            return 0;
        }
    }
    return -1;
}