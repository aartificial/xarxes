/**************************************************************************/
/*                                                                        */
/* P1 - UEB amb sockets TCP/IP - Part I                                   */
/* Fitxer capçalera de aUEBs.c                                            */
/*                                                                        */
/* Autors:                                                                */
/* Data:                                                                  */
/*                                                                        */
/**************************************************************************/

/* Declaració de funcions EXTERNES de aUEBs.c, és a dir, d'aquelles       */
/* funcions que es faran servir en un altre fitxer extern a aUEBs.c,      */
/* p.e., int UEBs_FuncioExterna(arg1, arg2...) { }                        */
/* El fitxer extern farà un #include del fitxer aUEBs.h a l'inici, i      */
/* així les funcions seran conegudes en ell.                              */
/* En termes de capes de l'aplicació, aquest conjunt de funcions          */
/* EXTERNES formen la interfície de la capa UEB, en la part servidora.    */

struct connection {
    int socket;
    char ip[16];
    int port;
};

struct petition {
    char type[4];
    char data[9999];
    int size;
};

struct UEB_DataTO {
    struct connection cli;
    struct connection ser;
    char path[200];
    struct petition pet;
    char response[9999];
    int scon_i[3];
    int scon_n;
};

int UEBs_IniciaServ(struct UEB_DataTO *dto);
int UEBs_AcceptaConnexio(struct UEB_DataTO *dto);
int UEBs_ServeixPeticio(struct UEB_DataTO *dto);
int UEBs_TancaConnexio(struct UEB_DataTO *dto);
int UEBs_HaArribatAlgunaCosa(struct UEB_DataTO *dto);
/* int UEBs_FuncioExterna(arg1, arg2...);                                 */
