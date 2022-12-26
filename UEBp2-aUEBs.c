/**************************************************************************/
/*                                                                        */
/* P1 - UEB amb sockets TCP/IP - Part I                                   */
/* Fitxer aUEB.c que implementa la capa d'aplicació de UEB, sobre la      */
/* cap de transport TCP (fent crides a la "nova" interfície de la         */
/* capa TCP o "nova" interfície de sockets TCP), en la part servidora.    */
/*                                                                        */
/* Autors:                                                                */
/* Data:                                                                  */
/*                                                                        */
/**************************************************************************/

/* Inclusió de llibreries, p.e. #include <sys/types.h> o #include "meu.h" */
/*  (si les funcions externes es cridessin entre elles, faria falta fer   */
/*   un #include del propi fitxer capçalera)                              */

#include "UEBp1-tTCP.h"
#include "UEBp1-aUEBs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>


/* Definició de constants, p.e.,                                          */
/* #define XYZ       1500                                                 */

#define PUEB_MAXINFO1SIZE 9999
#define PUEB_MINDATASIZE 1
#define PUEB_MAXBUFFERSIZE 10006
#define PUEB_TYPESIZE 3
#define PUEB_INFO1SIZE 4
#define PUEB_OBT "OBT\0"
#define PUEB_ERR "ERR\0"
#define PUEB_COR "COR\0"

/* Declaració de funcions INTERNES que es fan servir en aquest fitxer     */
/* (les  definicions d'aquestes funcions es troben més avall) per així    */
/* fer-les conegudes des d'aquí fins al final d'aquest fitxer, p.e.,      */

/* int FuncioInterna(arg1, arg2...);                                      */

int ConstiEnvMis(struct UEB_DataTO *dto);
int RepiDesconstMis(struct UEB_DataTO *dto);

void construct_msg(char* msg, struct UEB_DataTO *dto);
int deconstruct_msg(char* buffer, struct UEB_DataTO *dto);
int read_source(struct UEB_DataTO *dto);
int read_file(struct UEB_DataTO *dto, int log);
int read_directory(struct UEB_DataTO *dto);
int set_response_type(char *TipusPeticio, int a);

/* Definició de funcions EXTERNES, és a dir, d'aquelles que es cridaran   */
/* des d'altres fitxers, p.e., int UEBs_FuncioExterna(arg1, arg2...) { }  */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* formen la interfície de la capa UEB, en la part servidora.             */

/* Inicia el S UEB: crea un nou socket TCP "servidor" a una @IP local     */
/* qualsevol i al #port TCP “portTCPser”. Escriu l'identificador del      */
/* socket creat a "SckEsc".                                               */
/* Escriu un missatge que descriu el resultat de la funció a "MisRes".    */
/*                                                                        */
/* "MisRes" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud màxima de 200 chars (incloent '\0').              */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si tot va bé;                                                       */

/* -1 si hi ha un error en la interfície de sockets.                      */
int UEBs_IniciaServ(struct UEB_DataTO *dto) {
    //printf("[UEB][OK] Source set @%s\n", dto->path);
    printf("Source set @/xarxes/llocUEB\n");

    if (-1 == (dto->ser.socket = TCP_CreaSockServidor(dto->ser.ip, dto->ser.port))) {
        strcat(dto->response, "[ER] Unable to create socket server.");
        return -1;
    }

    strcpy(dto->response, "Server is running ");
    return 0;
}

/* Accepta una connexió d'un C UEB que arriba a través del socket TCP     */
/* "servidor" d'identificador "SckEsc". Escriu l'@IP i el #port TCP del   */
/* socket TCP "client" a "IPcli" i "portTCPcli", respectivament, i l'@IP  */
/* i el #port TCP del socket TCP "servidor" a "IPser" i "portTCPser",     */
/* respectivament.                                                        */
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
int UEBs_AcceptaConnexio(struct UEB_DataTO *dto) {
    int socket;
    char buffer[200];
    if (-1 == (socket = TCP_AcceptaConnexio(dto->ser.socket, dto->cli.ip, &dto->cli.port))) {
        strcpy(dto->response, "\nConnection rejected");
        return -1;
    }
    TCP_TrobaAdrSockLoc(socket, dto->ser.ip, &dto->ser.port);
    TCP_TrobaAdrSockRem(socket, dto->cli.ip, &dto->cli.port);

    sprintf(buffer, "\nConnection accepted as @%d(%s:#%d)", socket, dto->cli.ip, dto->cli.port);
    strcpy(dto->response,  buffer);
    return socket;
}

/* Serveix una petició UEB d'un C a través de la connexió TCP             */
/* d'identificador "SckCon". A "TipusPeticio" hi escriu el tipus de       */
/* petició (p.e., PUEB_OBT) i a "NomFitx" el nom del fitxer de la petició.     */
/* Escriu un missatge que descriu el resultat de la funció a "MisRes".    */
/*                                                                        */
/* "TipusPeticio" és un "string" de C (vector de chars imprimibles acabat */
/* en '\0') d'una longitud de 4 chars (incloent '\0').                    */
/* "NomFitx" és un "string" de C (vector de chars imprimibles acabat en   */
/* '\0') d'una longitud <= 10000 chars (incloent '\0').                   */
/* "MisRes" és un "string" de C (vector de chars imprimibles acabat en    */
/* '\0') d'una longitud màxima de 200 chars (incloent '\0').              */
/*                                                                        */
/* Retorna:                                                               */
/*  0 si el fitxer existeix al servidor;                                  */
/*  1 la petició és ERRònia (p.e., el fitxer no existeix);                */
/* -1 si hi ha un error a la interfície de sockets;                       */
/* -2 si protocol és incorrecte (longitud camps, tipus de peticio, etc.); */
/* -3 si l'altra part tanca la connexió;                                  */
/* -4 si hi ha problemes amb el fitxer de la petició (p.e., nomfitxer no  */
/*  comença per /, fitxer no es pot llegir, fitxer massa gran, etc.).     */
int UEBs_ServeixPeticio(struct UEB_DataTO *dto) {
    int error, ret;
    char buffer[1006];

    if (0 > (ret = RepiDesconstMis(dto))) {
        switch (ret) {
            case -1: strcpy(dto->response, "\nError in socket interface >>> ");break;
            case -2: strcpy(dto->response, "\nInvalid protocol >>> ");break;
            case -3: strcpy(dto->response, "\nClient connection is closed >>> ");break;
            default: ;break;
        }
        strcat(dto->response, TCP_ObteMissError());
        return ret;
    }

    sprintf(buffer,"\nPetition %s%04d%s recieved from @%d.", dto->pet.type, dto->pet.size, dto->pet.data, dto->cli.socket);
    strcpy(dto->response, buffer);

    int status = read_source(dto);
    error = set_response_type(dto->pet.type, status);

    if (0 > (ret = ConstiEnvMis(dto))) {
        switch (ret) {
            case -1:strcat(dto->response, "Cannot send to socket interface >>> ");break;
            case -2:strcat(dto->response, "Invalid protocol >>> ");break;
            default: ;break;
        }
        strcat(dto->response, TCP_ObteMissError());
        return ret;
    }

    sprintf(buffer,"\nPetition served at @%d as %s%04d%s",  dto->cli.socket, dto->pet.type, dto->pet.size, dto->pet.data);
    strcat(dto->response, buffer);

    if (error != 0)
        return -4;
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
int UEBs_TancaConnexio(struct UEB_DataTO *dto) {
    if (-1 == (TCP_TancaSock(dto->cli.socket))) {
        strcpy(dto->response, "\nUnable to close socket >>> ");
        strcat(dto->response, TCP_ObteMissError());
        return -1;
    }
    strcpy(dto->response, "\nSocket successfully closed.");
    return 0;
}

/* Si ho creieu convenient, feu altres funcions EXTERNES                  */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int UEBs_FuncioExterna(arg1, arg2...)
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
int ConstiEnvMis(struct UEB_DataTO *dto) {
    if (dto->pet.size < PUEB_MINDATASIZE || dto->pet.size > PUEB_MAXINFO1SIZE)
        return -2;
    char msg[dto->pet.size];
    construct_msg(msg, dto);
    return TCP_Envia(dto->cli.socket, msg, dto->pet.size + PUEB_TYPESIZE + PUEB_INFO1SIZE);
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
/*  0 si tot va bé,                                                       */
/* -1 si hi ha un error a la interfície de sockets;                       */
/* -2 si protocol és incorrecte (longitud camps, tipus de peticio);       */
/* -3 si l'altra part tanca la connexió.	                              */
int RepiDesconstMis(struct UEB_DataTO *dto) {
    char buffer[PUEB_MAXBUFFERSIZE];
    int bytes = TCP_Rep(dto->cli.socket, buffer, sizeof(buffer));
    switch (bytes) {
        case -1: return -1;
        case  0: return -3;
        default: if (bytes < PUEB_TYPESIZE + PUEB_INFO1SIZE) return -2;
    }
    return deconstruct_msg(buffer, dto);
}

/* Examina simultàniament i sense límit de temps (una espera indefinida)  */
/* els sockets (poden ser TCP, UDP i  teclat -stdin-) amb identificadors  */
/* en la llista “LlistaSck” (de longitud “LongLlistaSck” sockets) per     */
/* saber si hi ha arribat alguna cosa per ser llegida, excepte aquells    */
/* que tinguin identificadors igual a -1.                                 */
/* Escriu un text que descriu el resultat de la funció a "TextRes".       */
/*                                                                        */
/* "LlistaSck" és un vector d'int d'una longitud d'almenys LongLlistaSck. */
/* "TextRes" és un "string" de C (vector de chars imprimibles acabat en   */
/* '\0') d'una longitud màxima de 200 chars (incloent '\0').              */
/*                                                                        */
/* Retorna:                                                               */
/*  l'identificador del socket a través del qual ha arribat alguna cosa;  */
/*  -1 si hi ha error.                                                    */
int UEBs_HaArribatAlgunaCosa(struct UEB_DataTO *dto) {
    int socket = TCP_HaArribatAlgunaCosaEnTemps(dto->scon_i, dto->scon_n, 0);
    if (socket == -1) {
        strcpy(dto->response, "\nUnable to select socket >>> ");
        strcat(dto->response, TCP_ObteMissError());
        return -1;
    }
    char buffer[200];
    sprintf(buffer, "\nSocket @%d selected", socket);
    strcpy(dto->response, buffer);
    return socket;
}

void construct_msg(char* msg, struct UEB_DataTO *dto) {
    char tmp[PUEB_INFO1SIZE]={0};
    sprintf(tmp, "%04d", dto->pet.size);
    memcpy(msg, dto->pet.type, PUEB_TYPESIZE);
    memcpy(msg + PUEB_TYPESIZE, tmp, PUEB_INFO1SIZE);
    memcpy(msg + PUEB_TYPESIZE + PUEB_INFO1SIZE, dto->pet.data, dto->pet.size);
}

int deconstruct_msg(char* buffer, struct UEB_DataTO *dto) {
    char tmp[PUEB_INFO1SIZE];
    tmp[PUEB_TYPESIZE] = '\0';
    memset(dto->pet.type, '\0', 4);
    memcpy(dto->pet.type, buffer, PUEB_TYPESIZE);
    memcpy(tmp, buffer + PUEB_TYPESIZE, PUEB_INFO1SIZE);
    dto->pet.size = atoi(tmp);
    if (dto->pet.size <= 0 || dto->pet.size > PUEB_MAXINFO1SIZE)
        return -2;
    memset(dto->pet.data, '\0', 9999);
    memcpy(dto->pet.data, buffer + PUEB_TYPESIZE + PUEB_INFO1SIZE, dto->pet.size);
    return 0;
}

int read_source(struct UEB_DataTO *dto) {
    // Check leading dash
    if (dto->pet.data[0] != '/') {
        strcat(dto->response, "\nFile name invalid format.");
        strcpy(dto->pet.data, "error2");
        dto->pet.size = (int) strlen(dto->pet.data);
        return -4;
    }

    if(dto->pet.data[dto->pet.size - 1] == '/')
        return read_directory(dto);
    else
        return read_file(dto, 1);
}

int read_file(struct UEB_DataTO *dto, int log) {
    char source[200];
    strcpy(source, dto->path);
    strcat(source, dto->pet.data);
    FILE* file = fopen(source, "r");

    // Check file can be opened
    if (file == NULL) {
        if (log) {
            printf("PATH: %s\n", source);
            strcat(dto->response, "\nUnable to open file.");
            strcpy(dto->pet.data, "error4");
            dto->pet.size = (int) strlen(dto->pet.data);
        }
        return -2;
    }

    dto->pet.size = (int) fread(dto->pet.data, 1, PUEB_MAXINFO1SIZE, file);
    fclose(file);

    // Check file is empty
    if (dto->pet.size == 0) {
        strcat(dto->response, "\nUnable to read file..");
        strcpy(dto->pet.data, "error1");
        dto->pet.size = (int) strlen(dto->pet.data);
        return -3;
    }

    // Check file size too big
    if (dto->pet.size > PUEB_MAXINFO1SIZE) {
        strcat(dto->response, "\nFile size is too big.");
        strcpy(dto->pet.data, "error3");
        dto->pet.size = (int) strlen(dto->pet.data);
        return -4;
    }

    return 0;
}

int read_directory(struct UEB_DataTO *dto){
    char path[200];
    strcpy(path, dto->path);
    strcat(path, dto->pet.data);
    DIR* directory = opendir(path);
    if(directory == NULL) {
        strcat(dto->response, "\nFile does not exist.");
        strcpy(dto->pet.data, "error5");
        dto->pet.size = (int) strlen(dto->pet.data);
        return -1;
    }

    char Nom2[200];
    strcpy(Nom2, path);
    strcat(Nom2, "index.html");
    int res = read_file( dto, 0);
    if (res == 0)
        return res;

    char aux[9999] = "<!DOCTYPE html>\n<HTML>\n<HEAD>\n<META charset=UTF-8>\n<TITLE>ERROR</TITLE>\n</HEAD>\n<BODY>";
    char command[200] = "ls -l ";
    strcat(command, path);
    FILE *ls_cmd = popen(command, "r");
    static char buff[9999];
    while ((fread(buff, 1, sizeof(buff)-1, ls_cmd)) > 0)
        strcat(aux, buff);
    pclose(ls_cmd);
    strcat(aux, "</BODY>\n</HTML>");



    strcpy(dto->pet.data, aux);
    dto->pet.size = strlen(aux);

    return 0;
}


int set_response_type(char *TipusPeticio, int a) {
    int bad_petition;
    if (a < 0) {
        strcpy(TipusPeticio, PUEB_ERR);
        if (a == -4)
            bad_petition = 1;
    } else {
        strcpy(TipusPeticio, PUEB_COR);
    }
    return bad_petition;
}