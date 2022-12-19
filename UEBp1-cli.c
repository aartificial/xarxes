#include "UEBp1-aUEBc.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define OBTENIR "obtenir"
#define PATH "/mnt/c/Users/jiesa/CLionProjects/xarxes/p2/server/recieve"

int read_petition(int *portTCPser, char *IPrem, char *NomFitx, char *message);
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
        char file[9999], file_name[200], message[200], target[200];

        int pet_s = read_petition(&portTCPser, IPser, file_name, message);
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

int read_petition(int *portTCPser, char *IPrem, char *NomFitx, char *message) {
    printf("******************************************************************\n");
    printf("*  Introduce petition (type, server ip, server port, file name)  *");
    printf("\n******************************************************************\n");
    printf("Petition: "); scanf("%s%s%d%s", message, IPrem, portTCPser, NomFitx);

    if (strcmp(message, "sortir") == 0)
        return -2;
    else if (strcmp(message, "obtenir") != 0) {
        printf("[ER] Invalid petition.\nExample : obtenir 0.0.0.0 3000 /filename.extension\n");
        return -1;
    }

    if (strcmp(IPrem, "0") == 0)
        strcpy(IPrem, "0.0.0.0");
    if (*portTCPser == 0)
        *portTCPser = 3000;

    printf("Petition requested: %s @%s:#%d%s\n", message, IPrem, (*portTCPser), NomFitx);
    return 0;
}

void close_sck(int sck) {
    char MisRes[200];
    UEBc_TancaConnexio(sck, MisRes);
    printf("%s\n", MisRes);
}