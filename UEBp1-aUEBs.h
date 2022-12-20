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



struct Data{
    int SckEsc, SckCon, portTCPser, portTCPcli;
    char MisRes[200], IPser[16], IPcli[16], source[200];
    int* LlistaSck;
    char TipusPeticio[4], NomFitx[9999];
    int LongLlistaSck;
};

int UEBs_IniciaServ(struct Data *data);
int UEBs_AcceptaConnexio(struct Data *data);
int UEBs_ServeixPeticio(struct Data *data);
int UEBs_TancaConnexio(int SckCon, char *MisRes);
int UEBs_HaArribatAlgunaCosa(struct Data *data);
/* int UEBs_FuncioExterna(arg1, arg2...);                                 */
