#include "UEBp1-aUEBs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define CONFIG_PATH "/Users/rocarmengoumartra/CLionProjects/xarxes/ser.cfg"
//#define LOG_PATH "/Users/rocarmengoumartra/CLionProjects/xarxes/ser.log"
#define CONFIG_PATH "/mnt/c/Users/jiesa/CLionProjects/xarxes/p2/server/ser.cfg"
#define LOG_PATH "/mnt/c/Users/jiesa/CLionProjects/xarxes/p2/server/ser.log"
#define NOMBRECONNSMAX 4

void structSetup(struct Data *data);
int AfegeixSck(int Sck, int *LlistaSck, int LongLlistaSck);
int TreuSck(int Sck, int *LlistaSck, int LongLlistaSck);
void log_message(char *message);
void read_config(struct Data *data);
void close_sck(int SckEsc);

void print_sockets(const int *LlistaSck) {
    for (int i =0; i < NOMBRECONNSMAX; i++)
        printf("%d ", LlistaSck[i]);
}

int main() {
    struct Data data;
    structSetup(&data);
    FILE *fitxer = fopen(LOG_PATH, "w");
    fprintf(fitxer, "");
    fclose(fitxer);

    int ini_s = UEBs_IniciaServ(&data);
    printf("%s\n", data.MisRes);
    log_message(data.MisRes);
    if (ini_s != 0)
        return -1;

    int breakpoint = 0;
    while (!breakpoint) {
        memset( data.NomFitx, '\0', sizeof(char)*9999 );
        data.SckCon = UEBs_AcceptaConnexio(&data);
        printf("%s\n", data.MisRes);
        log_message(data.MisRes);
        if (data.SckCon < 0)
            continue;

        printf("[DEBUG] Adding socket %d.\n", data.SckCon);
        AfegeixSck(data.SckCon, data.LlistaSck, NOMBRECONNSMAX);
        printf("[DEBUG] Socket %d added.\n", data.SckCon);
        print_sockets(data.LlistaSck);
        printf("[DEBUG] Waiting for socket.\n");
        data.SckCon = UEBs_HaArribatAlgunaCosa(&data);
        printf("[DEBUG] Socket recieved %d.\n", data.SckCon);
        printf("[DEBUG] Removing socket %d.\n", data.SckCon);
        TreuSck(data.SckCon, data.LlistaSck, NOMBRECONNSMAX);
        printf("[DEBUG] Socket %d removed.\n", data.SckCon);
        print_sockets(data.LlistaSck);

        int pet_s = UEBs_ServeixPeticio(&data);
        printf("%s\n", data.MisRes);
        log_message(data.MisRes);
        if (pet_s != 0) {
            close_sck(data.SckCon);
            if (pet_s == -3)
                breakpoint = 1;
            continue;
        }
        close_sck(data.SckCon);
    }
    close_sck(data.SckEsc);
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

void read_config(struct Data *data) {
    FILE* fitxer = fopen(CONFIG_PATH, "r");
    char data_config[400];
    if (fitxer != NULL) {
        fread(data_config, 200, 2, fitxer);
        int i = 0;
        if (data_config[i] == '#') {
            i+=9;
            data->portTCPser = atoi(&data_config[9]);
            while (data_config[i] != '#')
                i++;
        }
        if (data_config[i] == '#') {
            i+=7;
            strcpy(data->source, &data_config[i]);
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

void structSetup(struct Data *data) {
    data->LlistaSck = malloc((sizeof *data->LlistaSck) * NOMBRECONNSMAX);
    data->LongLlistaSck = NOMBRECONNSMAX;
    for(int i=0; i <data->LongLlistaSck; i++){
        data->LlistaSck[i] = -1;
    }
    read_config(data);
}