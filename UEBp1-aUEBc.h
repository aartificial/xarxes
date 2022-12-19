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
 
int UEBc_DemanaConnexio(const char *IPser, int portTCPser, char *IPcli, int *portTCPcli, char *MisRes);
int UEBc_ObteFitxer(int SckCon, const char *NomFitx, char *Fitx, int *LongFitx, char *MisRes);
int UEBc_TancaConnexio(int SckCon, char *MisRes);
/* int UEBc_FuncioExterna(arg1, arg2...);                                 */
