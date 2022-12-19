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

int UEBs_IniciaServ(int *SckEsc, int portTCPser, char *MisRes, const char* source);
int UEBs_AcceptaConnexio(int SckEsc, char *IPser, int *portTCPser, char *IPcli, int *portTCPcli, char *MisRes);
int UEBs_ServeixPeticio(int SckCon, char *TipusPeticio, char *NomFitx, char *MisRes);
int UEBs_TancaConnexio(int SckCon, char *MisRes);
int UEBs_HaArribatAlgunaCosa(const int *LlistaSck, int LongLlistaSck, char *MisRes);
/* int UEBs_FuncioExterna(arg1, arg2...);                                 */
