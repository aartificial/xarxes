/**************************************************************************/
/*                                                                        */
/* P1 - UEB amb sockets TCP/IP - Part I                                   */
/* Fitxer tTCP.c que "implementa" la capa de transport TCP, o més         */
/* ben dit, que encapsula les funcions de la interfície de sockets        */
/* TCP, en unes altres funcions més simples i entenedores: la "nova"      */
/* interfície de sockets TCP.                                             */
/*                                                                        */
/* Autors:                                                                */
/* Data:                                                                  */
/*                                                                        */
/**************************************************************************/

/* Inclusió de llibreries, p.e. #include <sys/types.h> o #include "meu.h" */
/*  (si les funcions externes es cridessin entre elles, faria falta fer   */
/*   un #include del propi fitxer capçalera)                              */

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

/* Definició de constants, p.e.,                                          */

/* #define XYZ       1500                                                 */

/* Declaració de funcions INTERNES que es fan servir en aquest fitxer     */
/* (les  definicions d'aquestes funcions es troben més avall) per així    */
/* fer-les conegudes des d'aquí fins al final d'aquest fitxer, p.e.,      */

/* int FuncioInterna(arg1, arg2...);                                      */

int get_max_scon(const int *pInt, int sck);

/* Definició de funcions EXTERNES, és a dir, d'aquelles que es cridaran   */
/* des d'altres fitxers, p.e., int TCP_FuncioExterna(arg1, arg2...) { }   */
/* En termes de capes de l'aplicació, aquest conjunt de funcions externes */
/* són la "nova" interfície de la capa de transport TCP (la "nova"        */
/* interfície de sockets TCP).                                            */

/* Crea un socket TCP “client” a l’@IP “IPloc” i #port TCP “portTCPloc”   */
/* (si “IPloc” és “0.0.0.0” i/o “portTCPloc” és 0 es fa/farà una          */
/* assignació implícita de l’@IP i/o del #port TCP, respectivament).      */
/*                                                                        */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna -1 si hi ha error; l’identificador del socket creat si tot     */
/* va bé.                                                                 */
int TCP_CreaSockClient(const char *IPloc, int portTCPloc) {
	int scon;
	if((scon=socket(AF_INET,SOCK_STREAM,0))==-1)
		return -1;

    struct sockaddr_in addr;
    int i;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(portTCPloc);
    addr.sin_addr.s_addr=inet_addr(IPloc); /* o bé: ...s_addr = INADDR_ANY */
    for(i=0;i<8;i++){addr.sin_zero[i]='\0';}

	return scon;
}

/* Crea un socket TCP “servidor” (o en estat d’escolta – listen –) a      */
/* l’@IP “IPloc” i #port TCP “portTCPloc” (si “IPloc” és “0.0.0.0” i/o    */
/* “portTCPloc” és 0 es fa una assignació implícita de l’@IP i/o del      */
/* #port TCP, respectivament).                                            */
/*                                                                        */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna -1 si hi ha error; l’identificador del socket creat si tot     */
/* va bé.                                                                 */
int TCP_CreaSockServidor(const char *IPloc, int portTCPloc) {//Passarli strcpy(IPloc,"0.0.0.0"); i portloc = 3000;{
	int sesc;
	if((sesc=socket(AF_INET,SOCK_STREAM,0))==-1)
		return -1;
	struct sockaddr_in addr;
	int i;
	addr.sin_family=AF_INET;
	addr.sin_port=htons(portTCPloc);
	addr.sin_addr.s_addr=inet_addr(IPloc); /* o bé: ...s_addr = INADDR_ANY */
	for(i=0;i<8;i++){addr.sin_zero[i]='\0';}

    if((bind(sesc,(struct sockaddr*)&addr,sizeof(addr)))==-1)
		return -1;

	if((listen(sesc,3))==-1)
		return -1;

    return sesc;
}

/* El socket TCP “client” d’identificador “Sck” es connecta al socket     */
/* TCP “servidor” d’@IP “IPrem” i #port TCP “portTCPrem” (si tot va bé    */
/* es diu que el socket “Sck” passa a l’estat “connectat” o establert     */
/* – established –). Recordeu que això vol dir que s’estableix una        */
/* connexió TCP (les dues entitats TCP s’intercanvien missatges           */
/* d’establiment de la connexió).                                         */
/*                                                                        */
/* "IPrem" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna -1 si hi ha error; 0 si tot va bé.                             */
int TCP_DemanaConnexio(int Sck, const char *IPrem, int portTCPrem) {
    struct sockaddr_in addr;
	int i;
	//strcpy(iprem,"10.0.0.23");
	//portrem = 3000;
    addr.sin_family=AF_INET;
	addr.sin_port=htons(portTCPrem);
	addr.sin_addr.s_addr= inet_addr(IPrem);
	for(i=0;i<8;i++){ addr.sin_zero[i]='\0';}
    return connect(Sck, (struct sockaddr*)&addr, sizeof(addr));
}

/* El socket TCP “servidor” d’identificador “Sck” accepta fer una         */
/* connexió amb un socket TCP “client” remot, i crea un “nou” socket,     */
/* que és el que es farà servir per enviar i rebre dades a través de la   */
/* connexió (es diu que aquest nou socket es troba en l’estat “connectat” */
/* o establert – established –; el nou socket té la mateixa adreça que    */
/* “Sck”).                                                                */
/*                                                                        */
/* Omple “IPrem” i “portTCPrem*” amb respectivament, l’@IP i el #port     */
/* TCP del socket remot amb qui s’ha establert la connexió.               */
/*                                                                        */
/* "IPrem" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna -1 si hi ha error; l’identificador del socket connectat creat  */
/* si tot va bé.                                                          */
int TCP_AcceptaConnexio(int Sck, char *IPrem, int *portTCPrem)
{
	struct sockaddr_in adrrem;
	int scon;
	adrrem.sin_family=AF_INET;
	adrrem.sin_port=htons(*portTCPrem);
	adrrem.sin_addr.s_addr= inet_addr(IPrem);
	socklen_t long_adrrem=sizeof(adrrem);
    return accept(Sck,(struct sockaddr*)&adrrem, &long_adrrem);
}

/* Envia a través del socket TCP “connectat” d’identificador “Sck” la     */
/* seqüència de bytes escrita a “SeqBytes” (de longitud “LongSeqBytes”    */
/* bytes) cap al socket TCP remot amb qui està connectat.                 */
/*                                                                        */
/* "SeqBytes" és un vector de chars qualsevol (recordeu que en C, un      */
/* char és un enter de 8 bits) d'una longitud >= LongSeqBytes bytes.      */
/*                                                                        */
/* Retorna -1 si hi ha error; el nombre de bytes enviats si tot va bé.    */


int TCP_Envia(int Sck, const char *SeqBytes, int LongSeqBytes) {
	return (int) write(Sck,SeqBytes,LongSeqBytes);
}

/* Rep a través del socket TCP “connectat” d’identificador “Sck” una      */
/* seqüència de bytes que prové del socket remot amb qui està connectat,  */
/* i l’escriu a “SeqBytes” (que té una longitud de “LongSeqBytes” bytes), */
/* o bé detecta que la connexió amb el socket remot ha estat tancada.     */
/*                                                                        */
/* "SeqBytes" és un vector de chars qualsevol (recordeu que en C, un      */
/* char és un enter de 8 bits) d'una longitud >= LongSeqBytes bytes.      */
/*                                                                        */
/* Retorna -1 si hi ha error; 0 si la connexió està tancada; el nombre    */
/* de bytes rebuts si tot va bé.                                          */
int TCP_Rep(int Sck, char *SeqBytes, int LongSeqBytes) {
	return (int) read(Sck, SeqBytes, LongSeqBytes);
}

/* S’allibera (s’esborra) el socket TCP d’identificador “Sck”; si “Sck”   */
/* està connectat es tanca la connexió TCP que té establerta.             */
/*                                                                        */
/* Retorna -1 si hi ha error; 0 si tot va bé.                             */
int TCP_TancaSock(int Sck) {
	return close(Sck); //Close retorna 0 si tot va bé i -1 si hi ha error.
}

/* Donat el socket TCP d’identificador “Sck”, troba l’adreça d’aquest     */
/* socket, omplint “IPloc” i “portTCPloc” amb respectivament, la seva     */
/* @IP i #port TCP.                                                       */
/*                                                                        */
/* "IPloc" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna -1 si hi ha error; 0 si tot va bé.                             */
int TCP_TrobaAdrSockLoc(int Sck, char *IPloc, int *portTCPloc) {

    struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	if (getsockname(Sck, (struct sockaddr *)&sin, &len) == -1){
	    return -1;
	}
    strcpy(IPloc, inet_ntoa(sin.sin_addr));
	*portTCPloc = ntohs(sin.sin_port);
    return 0;
}

/* Donat el socket TCP “connectat” d’identificador “Sck”, troba l’adreça  */
/* del socket remot amb qui està connectat, omplint “IPrem” i             */
/* “portTCPrem*” amb respectivament, la seva @IP i #port TCP.             */
/*                                                                        */
/* "IPrem" és un "string" de C (vector de chars imprimibles acabat en     */
/* '\0') d'una longitud màxima de 16 chars (incloent '\0').               */
/*                                                                        */
/* Retorna -1 si hi ha error; 0 si tot va bé.                             */
int TCP_TrobaAdrSockRem(int Sck, char *IPrem, int *portTCPrem) {
	struct sockaddr_in srem;
	socklen_t len = sizeof(srem);
	if (getpeername(Sck, (struct sockaddr *)&srem, &len) == -1){
	    return -1;
	}
	strcpy(IPrem, inet_ntoa(srem.sin_addr));
	*portTCPrem = ntohs(srem.sin_port);
	return 0;
}

/* Obté un missatge de text que descriu l'error produït en la darrera     */
/* crida de sockets TCP.                                                  */
/*                                                                        */
/* Retorna aquest missatge de text en un "string" de C (vector de chars   */
/* imprimibles acabat en '\0')                                            */
char* TCP_ObteMissError(void) {
 return strerror(errno);
}

/* Examina simultàniament durant "Temps" (en [ms]) els sockets (poden ser */
/* TCP, UDP i teclat -stdin-) amb identificadors en la llista “LlistaSck” */
/* (de longitud “LongLlistaSck” sockets) per saber si hi ha arribat       */
/* alguna cosa per ser llegida. Si Temps és -1, s'espera indefinidament   */
/* fins que arribi alguna cosa.                                           */
/*                                                                        */
/* "LlistaSck" és un vector d'int d'una longitud d'almenys LongLlistaSck. */
/*                                                                        */
/* Retorna:                                                               */
/*  l'identificador del socket a través del qual ha arribat alguna cosa;  */
/*  -1 si hi ha error;                                                    */
/*  -2 si passa "Temps" sense que arribi res.                             */
int TCP_HaArribatAlgunaCosaEnTemps(const int *LlistaSck, int LongLlistaSck, int Temps) {
    fd_set fdset;
    struct timeval select_wait; select_wait.tv_sec = Temps;
    int descmax = -1;

    FD_ZERO(&fdset);
    for (int i = 0; i < LongLlistaSck; i++)
        FD_SET(LlistaSck[i], &fdset);                         // marcar sockets
    descmax = get_max_scon(LlistaSck, LongLlistaSck);  // agafar max socket
    printf("[DEBUG] select() descmax %d\n", descmax);
    int sockets_waiting = select(descmax + 1,              // agafar nombre de sockets esperant
                                 &fdset,
                                 NULL,
                                 NULL,
                                 &select_wait);
    printf("[DEBUG] select() %d\n", sockets_waiting);
    if (sockets_waiting == -1)                                  // si no hi ha cap socket esperant
        return -1;

    if (sockets_waiting == 0)
        return -2;

    for (int i = 0; i < LongLlistaSck; i++) {
        int bit = FD_ISSET(LlistaSck[i], &fdset);
        if (bit != 0)
            return LlistaSck[i];
    }

    return -1;
}

int get_max_scon(const int *LlistaSck, int LongLlistaSck) {
    int max = -1;
    for (int i = 0; i < LongLlistaSck; i++)
        if (LlistaSck[i] > max)
            max = LlistaSck[i];
    return max;
}

/* Si ho creieu convenient, feu altres funcions EXTERNES                  */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int TCP_FuncioExterna(arg1, arg2...)
{
	
} */

/* Definició de funcions INTERNES, és a dir, d'aquelles que es faran      */
/* servir només en aquest mateix fitxer. Les seves declaracions es troben */
/* a l'inici d'aquest fitxer.                                             */

/* Descripció de la funció, dels arguments, valors de retorn, etc.        */
/* int FuncioInterna(arg1, arg2...)
{
	
} */
