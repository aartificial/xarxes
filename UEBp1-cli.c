#include "UEBp1-aUEBc.h"
#include "UEBp2-aDNSc.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define OBTENIR "obtenir"
#define PATH "/Users/rocarmengoumartra/CLionProjects/xarxes/recieve"

int read_petition(int *portTCPser, char *IPrem, char *NomFitx);
void close_sck(int SckEsc);
//lkjhgfd
int main() {
    int portTCPser = 3000, portTCPcli;
    char MisRes[200], IPser[16], IPcli[16];
    printf("******************************************************************\n");
    printf("*  Introduce client ip(default: 0) and port(default: 0):         *\n");
    printf("******************************************************************\n");
    //printf("IP: "); scanf("%s", IPcli);
    //printf("PORT: "); scanf("%d", &portTCPcli);

   // if (strcmp(IPcli, "0") == 0) strcpy(IPcli, "10.0.0.23");
   // if (portTCPcli == 0) portTCPcli = 3000;

    strcpy(IPcli, "10.0.0.23");
    portTCPcli = 3000;


    while (1) {
        int file_size;
        char file[9999], file_name[200], target[200];

        int pet_s = read_petition(&portTCPser, IPser, file_name);
        if (pet_s == -1)
            continue;
        else if (pet_s == -2)
            return 0;

        int sck_s = UEBc_DemanaConnexio(IPser, portTCPser, IPcli, &portTCPcli, MisRes);
        printf("%s\n", MisRes);
        if (sck_s < 0) {
            continue;
        }

        int obt_s = UEBc_ObteFitxer(sck_s, file_name, file, &file_size, MisRes);
        printf("%s\n", MisRes);
        if (obt_s != 0) {
            close_sck(sck_s);
            continue;
        }

        strcpy(target, PATH);
        strcat(target, file_name);
        FILE *fitxer_fd = fopen(target, "w");
        if (fitxer_fd != NULL) {
            fwrite(file, file_size, 1, fitxer_fd);
            fclose(fitxer_fd);
            printf("[OK] File sucessfully saved.\n");
        } else {
            printf("[ER] Failed to save file at:\n%s\n", target);
        }

        sleep(5);
        close_sck(sck_s);
    }
}

int read_petition(int *portTCPser, char *IPrem, char *NomFitx) {
    printf("******************************************************************\n");
    printf("*  Introduce petition (URI)  *");
    printf("\n******************************************************************\n");
    printf("URI: ");
    //agafo uri
    char uri[200];
    scanf("%s", uri);
    printf("[DEBUG] URI rebuda: %s", uri);
    //variables per fer sscanf
    char esquema[100], nom_fitx[100], port_str[100], nom_host[100], TextRes[100];
    //sscanf, n retorna nombre de dades llegides
    int n = sscanf(uri,"%[^:]://%[^:]:%[^/]%s",esquema,nom_host,port_str,nom_fitx);
    //com només pot fallar port si no hi són totes port es posa al típic
    if(n<4){
        sscanf(uri,"%[^:]://%[^/]%s",esquema,nom_host,nom_fitx);
        *portTCPser = 3000;
    }else{
        //passar port a *int
        *portTCPser = atoi(port_str);
    }
    //PROVES
    //pueb://localhost:3000/primera.html
    //pueb://localhost/primera.html
    //posar nom DNS i torna IP a IPrem, TextRes es un ok que dona la funcio si tot va be (ve fet)
    if(0!=DNSc_ResolDNSaIP(nom_host, IPrem, TextRes))
        printf("[ER] Error al resoldre la IP.\n");
    printf("[DEBUG] Esquema %s.\n", esquema);
    printf("[DEBUG] IP %s.\n", IPrem);
    printf("[DEBUG] Port %d.\n", *portTCPser);
    printf("[DEBUG] Text final %s.\n", nom_fitx);
    return 0;
}

void close_sck(int sck) {
    char MisRes[200];
    UEBc_TancaConnexio(sck, MisRes);
    printf("%s\n", MisRes);
}

