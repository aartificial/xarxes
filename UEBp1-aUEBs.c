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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

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

#define PATH "/Users/rocarmengoumartra/CLionProjects/xarxes/llocUEB"

/* Declaració de funcions INTERNES que es fan servir en aquest fitxer     */
/* (les  definicions d'aquestes funcions es troben més avall) per així    */
/* fer-les conegudes des d'aquí fins al final d'aquest fitxer, p.e.,      */

/* int FuncioInterna(arg1, arg2...);                                      */

int ConstiEnvMis(int SckCon, const char *tipus, const char *info1, int long1);
int RepiDesconstMis(int SckCon, char *tipus, char *info1, int *long1);

void construct_msg(char* msg, const char* op, const char* info1, int long1);
int deconstruct_msg(char* buffer, char* tipus, char* info1, int* long1);
int read_file(char* NomFitx, char* info1, int* long1, char* MisRes);
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
int UEBs_IniciaServ(int *SckEsc, int portTCPser, char *MisRes, const char* source) {
    char buffer[200], IPloc[16];
    sprintf(buffer, "[OK] Source set @%s\n", source);

    strcpy(IPloc,"0.0.0.0");
    if(-1 == (*SckEsc=TCP_CreaSockServidor(IPloc, portTCPser))) {
        sprintf(buffer + 21, MisRes, "[ER] Unable to create socket server.");
        return -1;
    }
    sprintf(buffer + strlen(buffer), "[OK] Server running at @%d(@%s:#%d).", *SckEsc, IPloc, portTCPser);
    strcpy(MisRes, buffer);
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
int UEBs_AcceptaConnexio(int SckEsc, char *IPser, int *portTCPser, char *IPcli, int *portTCPcli, char *MisRes) {
    int socket_fd;
    if(-1 == (socket_fd = TCP_AcceptaConnexio(SckEsc, IPcli, portTCPcli))) {
        sprintf(MisRes, "[ER] Connection rejected from @%s:#%d.", IPcli, *portTCPcli);
        return -1;
    }
    TCP_TrobaAdrSockLoc(socket_fd, IPser, portTCPser);
    TCP_TrobaAdrSockRem(socket_fd, IPcli, portTCPcli);
    sprintf(MisRes, "[OK] Connection accepted at @%d from @%s:#%d.", socket_fd, IPcli, *portTCPcli);
    return socket_fd;
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
int UEBs_ServeixPeticio(int SckCon, char *TipusPeticio, char *NomFitx, char *MisRes) {
    int long1, error, ret;
    char info1[PUEB_MAXINFO1SIZE];

    if (0 > (ret = RepiDesconstMis(SckCon, TipusPeticio, NomFitx, &long1))) {
        switch (ret) {
            case -1: strcpy(MisRes, "[ER] UEBs_ServeixPetició >>> Hi ha un error status l'interfície de sockets.");break;
            case -2: strcpy(MisRes, "[ER] UEBs_ServeixPetició >>> Protocol incorrecte o connexio tancada.");break;
            case -3: strcpy(MisRes, "[ER] UEBs_ServeixPetició >>> Connexio tancada.");break;
            default: ;break;
        }
        return ret;
    }

    //cannot send all info through MisRes **** stack smashing detected ***
    printf("[OK] Petition %s%04d%s recieved from @%d.\n", TipusPeticio,long1,NomFitx, SckCon);

    int status = read_file(NomFitx, info1, &long1, MisRes);
    error = set_response_type(TipusPeticio, status);

    if (0 > (ret = ConstiEnvMis(SckCon, TipusPeticio, info1, long1))) {
        switch (ret) {
            case -1:strcpy(MisRes, "[ER] Cannot send to socket interface.");break;
            case -2:strcpy(MisRes, "[ER] Invalid protocol.");break;
            default: ;break;
        }
        return ret;
    }

    //cannot send all info through MisRes **** stack smashing detected ***
    if (long1 < 100)
        printf("[OK] Petition %s%04d%s served at @%d.\n", TipusPeticio, long1, info1, SckCon);
    else {
        printf("[OK] Petition %s%04d served at @%d.\n", TipusPeticio, long1, SckCon);
        printf("[DATA]\n");
        write(1, info1, long1);
        printf("\n");
    }

    if (error != 0)
        return -4;
    strcpy(MisRes, "[OK] Petition served successfully.");
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
int UEBs_TancaConnexio(int SckCon, char *MisRes) {
    if (-1 == (TCP_TancaSock(SckCon))) {
        strcpy(MisRes, "[ER] Unable to close socket.");
        return -1;
    }
    strcpy(MisRes, "[OK] Socket successfully closed.");
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
int ConstiEnvMis(int SckCon, const char *tipus, const char *info1, int long1) {
    if (long1 < PUEB_MINDATASIZE || long1 > PUEB_MAXINFO1SIZE)
        return -2;
    char msg[long1];
    construct_msg(msg, tipus, info1, long1);
    return TCP_Envia(SckCon, msg, long1 + PUEB_TYPESIZE + PUEB_INFO1SIZE);
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
int RepiDesconstMis(int SckCon, char *tipus, char *info1, int *long1) {
    char buffer[PUEB_MAXBUFFERSIZE]= {0};
    int bytes = TCP_Rep(SckCon, buffer, sizeof(buffer));
    switch (bytes) {
        case -1: return -1;
        case  0: return -3;
        default: if (bytes < PUEB_TYPESIZE + PUEB_INFO1SIZE) return -2;
    }
    return deconstruct_msg(buffer, tipus, info1, long1);
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
int UEBs_HaArribatAlgunaCosa(const int *LlistaSck, int LongLlistaSck, char *TextRes) {

    int Sck = TCP_HaArribatAlgunaCosaEnTemps(LlistaSck, LongLlistaSck, 0);
    printf("[DEBUG] UEBs Socket returned %d\n", Sck);
    if (Sck == -1) {
        strcpy(TextRes, "[ERROR] Unable to select socket.");
        return -1;
    }
    strcpy(TextRes, "[OK] Socket selected.");
    return Sck;
}

void construct_msg(char* msg, const char* op, const char* info1, int long1) {
    char tmp[PUEB_INFO1SIZE]={0};
    sprintf(tmp, "%04d", long1);
    memcpy(msg, op, PUEB_TYPESIZE);
    memcpy(msg + PUEB_TYPESIZE, tmp, PUEB_INFO1SIZE);
    memcpy(msg + PUEB_TYPESIZE + PUEB_INFO1SIZE, info1, long1);
}

int deconstruct_msg(char* buffer, char* tipus, char* info1, int* long1) {
    char tmp[PUEB_INFO1SIZE]; tmp[PUEB_TYPESIZE] = '\0';
    memcpy(tipus, buffer, PUEB_TYPESIZE);
    memcpy(tmp, buffer + PUEB_TYPESIZE, PUEB_INFO1SIZE);
    *long1 = atoi(tmp);
    if (*long1 <= 0 || *long1 > PUEB_MAXINFO1SIZE)
        return -2;
    memcpy(info1, buffer + PUEB_TYPESIZE + PUEB_INFO1SIZE, *long1);
    return 0;
}

int read_file(char* NomFitx, char* info1, int* long1, char* MisRes) {
    char source[200];
    strcpy(source, PATH);
    // Check leading dash
    if (NomFitx[0] != '/') {
        strcpy(MisRes, "[ER] File name invalid format.");
        strcpy(info1, "error2");
        *long1 = (int)  strlen(info1);
        return -4;
    }
    strcat(source, NomFitx);
    FILE* file = fopen(source, "r");

    // Check file can be opened
    if (file == NULL) {
        strcpy(MisRes, "[ER] Unable to open file.");
        strcpy(info1, "error4");
        *long1 = (int) strlen(info1);
        return -2;
    }

    *long1 = (int) fread(info1, 1, PUEB_MAXINFO1SIZE, file);
    fclose(file);

    // Check file is empty
    if (*long1 == 0) {
        strcpy(MisRes, "[ER] Unable to read file..");
        strcpy(info1, "error1");
        *long1 = (int) strlen(info1);
        return -3;
    }

    // Check file size too big
    if (*long1 > PUEB_MAXINFO1SIZE) {
        strcpy(MisRes, "[ER] File size is too big.");
        strcpy(info1, "error3");
        *long1 = (int) strlen(info1);
        return -4;
    }
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