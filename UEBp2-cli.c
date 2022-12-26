#include "UEBp1-aUEBc.h"
#include "UEBp2-aDNSc.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define PATH "./recieve"

int read_petition(struct Data *data);
void close_sck(int sck);

int main() {
    struct Data data;
    strcpy(data.IPcli, "10.0.0.23");
    data.portTCPcli = 3000;

    while (1) {
        int pet_s = read_petition(&data);
        size_t ps = data.file_size;
        if (pet_s == -1)
            continue;

        data.sck_s = UEBc_DemanaConnexio(&data);
        printf("%s", data.MisRes);
        if (data.sck_s < 0) {
            continue;
        }

        int obt_s = UEBc_ObteFitxer(&data);
        printf("%s", data.MisRes);
        if (obt_s != 0) {
            close_sck(data.sck_s);
            continue;
        }

        strcpy(data.target, PATH);
        strcat(data.target, data.file_name);
        if(data.file_name[data.file_size-1] == '/' || ps == 1)
            strcat(data.target, "index.html");
        FILE *fitxer_fd = fopen(data.target, "w");
        if (fitxer_fd != NULL) {
            fwrite(data.file, data.file_size, 1, fitxer_fd);
            fclose(fitxer_fd);
            printf("\n[OK] File sucessfully saved.");
        } else {
            printf("\n[ER] Failed to save file at:\n%s", data.target);
        }

        sleep(10);
        close_sck(data.sck_s);
    }
}

int read_petition(struct Data *data) {
    printf("\n******************************************************************\n");
    printf("*  Introduce petition (URI)  *");
    printf("\n******************************************************************\n");
    printf("URI: ");
    char uri[200], esquema[100], nom_fitx[100], port_str[100], nom_host[100], TextRes[100];
    scanf("%s", uri);
    int n = sscanf(uri,"%[^:]://%[^:]:%[^/]%s",esquema,nom_host,port_str,nom_fitx);
    if(n<4){
        sscanf(uri,"%[^:]://%[^/]%s",esquema,nom_host,nom_fitx);
        data->portTCPser = 3000;
    }else{
        data->portTCPser = atoi(port_str);
    }

    strcpy(data->file_name, nom_fitx);
    data->file_size = strlen(nom_fitx);

    if (strcmp(esquema, "pueb") != 0) {
        printf("[ER] Invalid schema.\nExample : pueb://[DNSName]:[#Port]/[FileName]\n");
        return -1;
    }
    //pueb://localhost:3000/primera.html
    //pueb://localhost/primera.html
    if(0!=DNSc_ResolDNSaIP(nom_host, data->IPser, TextRes))
        printf("[ER] Error al resoldre la IP.\n");
    return 0;
}

void close_sck(int sck) {
    char MisResClose[200];
    UEBc_TancaConnexio(sck, MisResClose);
    printf("%s", MisResClose);
}

