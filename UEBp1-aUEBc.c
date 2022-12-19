/**************************************************************************/
/*                                                                        */
/* P1 - UEB amb sockets TCP/IP - Part I                                   */
/* Fitxer aUEBc.c que implementa la capa d'aplicació de UEB, sobre la     */
/* cap de transport TCP (fent crides a la "nova" interfície de la         */
/* capa TCP o "nova" interfície de sockets TCP), en la part client.       */
/*                                                                        */
/* Autors:                                                                */
/* Data:                                                                  */
/*                                                                        */
/**************************************************************************/

/* Inclusió de llibreries, p.e. #include <sys/types.h> o #include "meu.h" */
/*  (si les funcions externes es cridessin entre elles, faria falta fer   */
/*   un #include del propi fitxer capçalera)                              */

#include "UEBp1-tTCP.h"
#include "UEBp1-aUEBc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Definició de constants, p.e.,                                          */

/* #define XYZ       1500                                                 */
#define PUEB_MAXINFO1SIZE 9999
#define PUEB_TYPESIZE 3
#define PUEB_INFO1SIZE 4
#define PUEB_OBT "OBT\0"
#define PUEB_ERR "ERR\0"
#define PUEB_COR "COR\0"

/* Declaració de funcions INTERNES que es fan servir en aquest fitxer     */
/* (les  definicions d'aquestes funcions es troben més avall) per així    */
/* fer-les conegudes des d'aquí fins al final d'aquest fitxer, p.e.,      */

/* int FuncioInterna(arg1, arg2...);                                      */

int ConstiEnvMis(int SckCon, const char *tipus, const char *info1, int long1);
int RepiDesconstMis(int SckCon, char *tipus, char *info1, int *long1);

void construct_msg(char* msg, const char* op, const char* info1, int long1);
int deconstruct_msg(char* buffer, char* tipus, char* info1, int* long1);

/* Definició de funcions EXTERNES, és a dir, d'aquelles que es cridaran   */
/* des d'altres fitxers, p.e., int UEBc_FuncioExterna(arg1, arg2...) { }  */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* formen la interfície de la capa UEB, en la part client.                */

/* Crea un socket TCP "client" en una @IP i #port TCP local qualsevol, a  */
/* través del qual demana una connexió a un S UEB que escolta peticions   */
/* al socket TCP "servidor" d'@IP "IPser" i #portTCP "portTCPser".        */
/* Escriu l'@IP i el #port TCP del socket "client" creat a "IPcli" i      */
/* "portTCPcli", respectivament.                                          */
/* Escriu un missatge que descriu el resultat de la funció a "MisRes".    */
/*                                                                        */
/* "IPser" i "IPcli" són "strings" de C (vector de chars imprimibles      */
/* acabats en '\0') d'una longitud màxima de 16 chars (incloent '\0').    */
/* "MisRes" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud màxima de 200 chars (incloent '\0').              */
/*                                                                        */
/* Retorna:                                                               */
/*  l'identificador del socket TCP connectat si tot va bé;                */
/* -1 si hi ha un error a la interfície de sockets.                       */
int UEBc_DemanaConnexio(struct Data *data) {
    int socket_fd;
    char aux[200];
    if(-1 == (socket_fd = TCP_CreaSockClient(data->IPcli, data->portTCPcli))) {
        strcpy(data->MisRes, "[ER] Unable to create socket.");
        return -1;
    }
    printf("[DEBUG] socket=%d IPser=%s portTCPser=%d\n", socket_fd, data->IPser, data->portTCPser);
    if (-1 == (TCP_DemanaConnexio(socket_fd, data->IPser, data->portTCPser))) {
        printf("%s:%d\n", data->IPser, data->portTCPser);
        strcpy(data->MisRes, "[ER] Connection refused.");
        return -1;
    }
    int transfer = 0;
    if (-1 == TCP_TrobaAdrSockLoc(socket_fd, data->IPcli, &transfer)) {
        strcpy(data->MisRes, "[ER] Cannot find local address.");
        return -1;
    }
    data->portTCPcli = transfer;
    sprintf(aux, "[OK] Connection established with @%d(@%s:#%d) as (@%s:#%d).", socket_fd, data->IPcli, data->portTCPcli, data->IPser, data->portTCPser);
    strcpy(data->MisRes, aux);
    return socket_fd;
}

/* Obté el fitxer de nom "file_name" del S UEB a través de la connexió TCP  */
/* d'identificador "sck_s". Escriu els bytes del fitxer a "file" i la    */
/* longitud del fitxer en bytes a "file_size".                             */
/* Escriu un missatge que descriu el resultat de la funció a "MisRes".    */
/*                                                                        */
/* "file_name" és un "string" de C (vector de chars imprimibles acabat en   */
/* '\0') d'una longitud <= 10000 chars (incloent '\0').                   */
/* "file" és un vector de chars (bytes) qualsevol (recordeu que en C,     */
/* un char és un enter de 8 bits) d'una longitud <= 9999 bytes.           */
/* "MisRes" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud màxima de 200 chars (incloent '\0').              */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si el fitxer existeix al servidor;                                  */
/*  1 la petició és ERRònia (p.e., el fitxer no existeix);                */
/* -1 si hi ha un error a la interfície de sockets;                       */
/* -2 si protocol és incorrecte (longitud camps, tipus de peticio, etc.); */
/* -3 si l'altra part tanca la connexió.                                  */
int UEBc_ObteFitxer(struct Data *data) {
    int resposta;
    char tipus[PUEB_TYPESIZE];
    if ( 0 > (resposta = ConstiEnvMis(data->sck_s, PUEB_OBT, data->file_name, (int)strlen(data->file_name)))) {
        switch (resposta) {
            case -1: strcpy(data->MisRes, "[ER] UEBc_ObteFitxer >>> Enviar a interface de sockets."); break;
            case -2: strcpy(data->MisRes, "[ER] UEBc_ObteFitxer >>> Protocol incorrecte.");break;
            default: strcpy(data->MisRes, "[ER] UEBc_ObteFitxer >>> Desconegut ENV.");
        }
        return resposta;
    }

    printf("[OK] Petition sent at @%d as OBT%04d%s\n", data->sck_s, (int) strlen(data->file_name), data->file_name);

    int transfer = 0;
    if (0 > (resposta = RepiDesconstMis(data->sck_s, tipus, data->file, &transfer))) {
        switch (resposta) {
            case -1: strcpy(data->MisRes, "\n[ER] UEBc_ObteFitxer >>> Rebre a interface de sockets."); break;
            case -2: strcpy(data->MisRes, "\n[ER] UEBc_ObteFitxer >>> Protocol incorrecte o connexio tancada.");break;
            case -3: strcpy(data->MisRes, "\n[ER] UEBc_ObteFitxer >>> Connexio tancada.");break;
            default: strcpy(data->MisRes, "\n[ER] UEBc_ObteFitxer >>> Desconegut REP.");
        }
        return resposta;
    }
    data->file_size = transfer;

    printf("[OK] Response received from @%d as: %s%04d", data->sck_s, tipus, data->file_size);
    fflush(stdout);
    write(1, data->file, data->file_size);
    printf("\n");

    if (strcmp(tipus, PUEB_ERR) == 0) {
        switch ((int)atoi((const char *) data->file[5])) {
            case  2: strcpy(data->MisRes, "[ER] Invalid petition, file name must start with '/'."); break;
            case  3: strcpy(data->MisRes, "[ER] Invalid petition, file size is too large to fit in PUEB protocol."); break;
            case  4: strcpy(data->MisRes, "[ER] Invalid petition, file is unreachable.");break;
            default: strcpy(data->MisRes, "[ER] Invalid petition.");
        }
        return 1;
    }

    strcpy(data->MisRes, "[OK] Petition resolved as valid");
    return 0;
}

/* Tanca la connexió TCP d'identificador "SckCon".                        */
/* Escriu un missatge que descriu el resultat de la funció a "MisRes".    */
/*                                                                        */
/* "MisRes" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud màxima de 200 chars (incloent '\0').              */
/*                                                                        */
/* Retorna:                                                               */
/*   0 si tot va bé;                                                      */
/*  -1 si hi ha un error a la interfície de sockets.                      */
int UEBc_TancaConnexio(int SckCon, char *MisRes) {
    if ((TCP_TancaSock(SckCon)) == -1) {
        strcpy(MisRes, "[ER] Unable to close connection.");
        return -1;
    }
    strcpy(MisRes, "[OK] Connection closed successfully.");
    return 0;
}

/* Si ho creieu convenient, feu altres funcions EXTERNES                  */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int UEBc_FuncioExterna(arg1, arg2...)
{
	
} */

/* Definició de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer. Les seves declaracions es        */
/* troben a l'inici d'aquest fitxer.                                      */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int FuncioInterna(arg1, arg2...)
{
	
} */

/* "Construeix" un missatge de PUEB a partir dels seus camps tipus,       */
/* long1 i info1, escrits, respectivament a "tipus", "long1" i "info1"    */
/* (que té una longitud de "long1" bytes), i l'envia a través del         */
/* socket TCP “connectat” d’identificador “SckCon”.                       */
/*                                                                        */
/* "tipus" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud de 4 chars (incloent '\0').                       */
/* "info1" és un vector de chars (bytes) qualsevol (recordeu que en C,    */
/* un char és un enter de 8 bits) d'una longitud <= 9999 bytes.           */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si tot va bé;                                                       */
/* -1 si hi ha un error a la interfície de sockets;                       */
/* -2 si protocol és incorrecte (longitud camps, tipus de peticio).       */
int ConstiEnvMis(int SckCon, const char *tipus, const char *info1, int long1) {
    // Check mida missatge
    if (long1 < 1 || long1 > PUEB_MAXINFO1SIZE)
        return -2;

    char msg[PUEB_TYPESIZE + PUEB_INFO1SIZE + PUEB_MAXINFO1SIZE];
    construct_msg(msg, tipus, info1, long1);
    return TCP_Envia(SckCon, msg, long1 + PUEB_TYPESIZE + PUEB_INFO1SIZE + 1);
}

/* Rep a través del socket TCP “connectat” d’identificador “SckCon” un    */
/* missatge de PUEB i el "desconstrueix", és a dir, obté els seus camps   */
/* tipus, long1 i info1, que escriu, respectivament a "tipus", "long1"    */
/* i "info1" (que té una longitud de "long1" bytes).                      */
/*                                                                        */
/* "tipus" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud de 4 chars (incloent '\0').                       */
/* "info1" és un vector de chars (bytes) qualsevol (recordeu que en C,    */
/* un char és un enter de 8 bits) d'una longitud <= 9999 bytes.           */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si tot va bé;                                                       */
/* -1 si hi ha un error a la interfície de sockets;                       */
/* -2 si protocol és incorrecte (longitud camps, tipus de peticio);       */
/* -3 si l'altra part tanca la connexió.                                  */
int RepiDesconstMis(int SckCon, char *tipus, char *info1, int *long1) {
    char buffer[PUEB_TYPESIZE + PUEB_INFO1SIZE + PUEB_MAXINFO1SIZE];
    int bytes = TCP_Rep(SckCon, buffer, sizeof(buffer));

    switch (bytes) {
        case -1: return -1;
        case  0: return -3;
        default: if (bytes < 7) return -2;
    }
    return deconstruct_msg(buffer, tipus, info1, long1);
}

void construct_msg(char* msg, const char* op, const char* info1, int long1) {
    char tmp[PUEB_INFO1SIZE];
    sprintf(tmp, "%04d", long1);
    memcpy(msg, op, PUEB_TYPESIZE);
    memcpy(msg + PUEB_TYPESIZE, tmp, PUEB_INFO1SIZE);
    memcpy(msg + PUEB_TYPESIZE + PUEB_INFO1SIZE, info1, long1);
}

int deconstruct_msg(char* buffer, char* tipus, char* info1, int* long1) {
    char tmp[4]; tmp[3] = '\0';
    memcpy(tipus, buffer, PUEB_TYPESIZE);
    memcpy(tmp, buffer + 3, 4);
    *long1 = atoi(tmp);
    if (*long1 <= 0 || *long1 > PUEB_MAXINFO1SIZE)
        return -2;
    memcpy(info1, buffer + PUEB_TYPESIZE + PUEB_INFO1SIZE, *long1);
    return 0;
}