#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// funz per controllare l'inserimento del servizio richiesto
bool check_service_input(char *service){
  char *avail_servs[] = {"stampa", "salva", "invia"};

  for (int i = 0; i < 3; i++)
    if (strcmp(service, avail_servs[i]) == 0)
      return true;

  printf("\nErr. Il nome va scelto tra \"stampa\" - \"salva\" - \"invia\"\n");
  return false;
}

int main (int argc, char *argv[]) {
    printf("Benvenuto! I servizi disponibili sono i seguenti:\n - stampa\n - salva\n - invia");

    // inserimento user_id
    char user_id[50];
    printf("\n\nInserire codice identificativo utente: ");
    scanf("%s", user_id);

    // selezione servizio
    char service[50];
    do {
      printf("Inserire il nome del servizio richiesto: ");
      scanf("%s", service);
    }
    while (!check_service_input(service));

    return 0;
}
