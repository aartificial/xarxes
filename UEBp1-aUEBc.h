/**************************************************************************/
/*                                                                        */
/* P1 - UEB amb sockets TCP/IP - Part I                                   */
/* Fitxer capçalera de aUEBc.c                                            */
/*                                                                        */
/* Autors:                                                                */
/* Data:                                                                  */
/*                                                                        */
/**************************************************************************/

/* Declaració de funcions EXTERNES de aUEBc.c, és a dir, d'aquelles       */
/* funcions que es faran servir en un altre fitxer extern a aUEBc.c,      */
/* p.e., int UEBc_FuncioExterna(arg1, arg2...) { }                        */
/* El fitxer extern farà un #include del fitxer aUEBc.h a l'inici, i      */
/* així les funcions seran conegudes en ell.                              */
/* En termes de capes de l'aplicació, aquest conjunt de funcions          */
/* EXTERNES formen la interfície de la capa UEB, en la part client.       */

struct Data{
    int portTCPser, portTCPcli, file_size, sck_s;
    char MisRes[200], IPser[16], IPcli[16], file[9999], file_name[200], target[200];
};

int UEBc_DemanaConnexio(struct Data *data);
int UEBc_ObteFitxer(struct Data *data);
int UEBc_TancaConnexio(int SckCon, char *MisRes);
/* int UEBc_FuncioExterna(arg1, arg2...);                                 */
