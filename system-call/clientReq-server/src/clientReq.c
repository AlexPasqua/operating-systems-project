#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[]) {
    printf("Hi, I'm ClientReq program!\n");
    printf("Benvenuto! I servizi disponibili sono i seguenti:\n - stampa\n - salva\n - invia");

    char user_id[50];
    printf("\n\nInserire codice identificativo utente: ");
    scanf("%s", user_id);

    char service[50];
    do {
      printf("Inserire il nome del servizio richiesto: ");
      scanf("%s", service);

      if (strcmp(service, "stampa") != 0 &&
          strcmp(service, "salva") != 0 &&
          strcmp(service, "invia") != 0)
      { printf("\nErr. Il nome va scelto tra \"stampa\" - \"salva\" - \"invia\"\n"); }
    }
    while (strcmp(service, "stampa") != 0 && strcmp(service, "salva") != 0 && strcmp(service, "invia") != 0);
    // TO_DO: migliorare questo do - while con una funz che testa la condiz e stampa
    //        il messaggio d'errore da mettere dentro il while (in modo da eliminare l'if)

    return 0;
}
