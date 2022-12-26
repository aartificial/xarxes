#include "UEBp1-aUEBs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CONFIG_PATH "./ser.cfg"
#define LOG_PATH "./ser.log"
#define NOMBRECONNSMAX 2

struct UEB_DataTO initialize_server();
int AfegeixSck(int Sck, int *LlistaSck, int LongLlistaSck);
int TreuSck(int Sck, int *LlistaSck, int LongLlistaSck);
void plog(char *message);

int main() {
    struct UEB_DataTO dto = initialize_server();
    int status = UEBs_IniciaServ(&dto);
    printf("%s", dto.response);
    if (status != 0)
        return -1;
    printf("at @%d(@%s:#%d).\n", dto.ser.socket, dto.ser.ip, dto.ser.port);
    
    AfegeixSck(dto.ser.socket, dto.scon_i, NOMBRECONNSMAX + 1);
	
    while (1) {
        plog("\nServer listening...\n");
        int sck = UEBs_HaArribatAlgunaCosa(&dto);
        plog(dto.response);
        if (sck < 0)
            continue;

        if (sck == dto.ser.socket) {
            dto.cli.socket = UEBs_AcceptaConnexio(&dto);
            plog(dto.response);
            if (dto.cli.socket < 0)
                continue;
            if (-1 == AfegeixSck(dto.cli.socket, dto.scon_i, dto.scon_n)) {
                plog("\nMaximum number of connections reached, closing socket");
                UEBs_TancaConnexio(&dto);
                plog(dto.response);
                continue;
            }
        } else {
            dto.cli.socket = sck;
        }
        int res = UEBs_ServeixPeticio(&dto);
        printf("%s\n", dto.response);
        if (res == -3)
            TreuSck(sck, dto.scon_i, NOMBRECONNSMAX + 1);
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

void plog(char *message) {
    FILE *file = fopen(LOG_PATH, "a");
    fprintf(file, "%s", message);
    fclose(file);
    printf("%s", message);
}

struct UEB_DataTO initialize_server() {
    struct UEB_DataTO dto;
    for (int i = 0; i < NOMBRECONNSMAX + 1; ++i)
        dto.scon_i[i] = -1;
    dto.scon_n = NOMBRECONNSMAX + 1;

    FILE* fitxer = fopen(CONFIG_PATH, "r");
    char data_config[400];
    if (fitxer != NULL) {
        fread(data_config, 200, 2, fitxer);
        int i = 0;
        if (data_config[i] == '#') {
            i += 9;
            dto.ser.port = atoi(&data_config[9]);
            while (data_config[i] != '#')
                i++;
        }
        if (data_config[i] == '#') {
            i += 7;
            strcpy(dto.path, &data_config[i]);
        }
        fclose(fitxer);
    } else {
        strcpy(dto.path, "/mnt/c/Users/jiesa/CLionProjects/xarxes/p2/server");
        dto.ser.port = 3000;
    }
    strcpy(dto.ser.ip, "127.0.0.1");
    dto.ser.port = 3000;
    return dto;
}
