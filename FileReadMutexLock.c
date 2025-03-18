#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> // Include for pthread
#include <string.h> // Include for strlen
#include <ctype.h>  // Include for isdigit

int balanse = 0; // Global variabel for balanse
pthread_mutex_t lock; // Mutex lås
 
/*
Les inn som string og feilsjekke før konvertering til tall.(isdigit) (atoi)
Overføre feilmeldinger(linjenummer) til main(husk heap)(malloc)(free)
*/

void* lesFraFilInngaaendeBalanse(void* filename) {
    FILE* file = fopen((char*)filename, "r");
    int linjenr = 0;
    char verdi[10];
    int* ptr_feil_kode = (int*) malloc(sizeof(int));
    *ptr_feil_kode = 0;

    if (ptr_feil_kode == NULL) {
        printf("Memory not allocated.\n");
        pthread_exit(NULL);
    }

    if (file == NULL) {
        printf("Feil ved åpning av fil. I funksjonen lesFraFilInngaaendeBalanse.\n");
        printf("Pass på at inn.txt er i samme mappe som programmet.\n");
        *ptr_feil_kode = 4;
        return (void*)ptr_feil_kode;
    }

    while (fscanf(file, "%5s", verdi) == 1) {
        linjenr++;

        // Check if the input string contains only digits
        for (int index = 0; index < strlen(verdi); index++) {
            if (!isdigit(verdi[index])) {
                printf("Feil linjenr: %d. Input er ikke et tall i inn.txt.\n", linjenr);
                *ptr_feil_kode = 5;
                break;
            }
        }

        if (pthread_mutex_lock(&lock) != 0) {
            printf("Feil ved låsing av mutex. I funksjonen lesFraFilInngaaendeBalanse.\n");
            *ptr_feil_kode = 6;
            break;
        }
  
        // Convert the input string to an integer and add it to balanse
        balanse += atoi(verdi);

        if (pthread_mutex_unlock(&lock) != 0) {
            printf("Feil ved opplåsing av mutex. I funksjonen lesFraFilInngaaendeBalanse.\n");
            *ptr_feil_kode = 7;
            break;
        }
    }

    if (fclose(file) != 0) {
        printf("Feil ved lukking av fil. I funksjonen lesFraFilInngaaendeBalanse.\n");
        *ptr_feil_kode = 8;
    }

    return (void*)ptr_feil_kode;
}
        
void* lesFraFilUtgaaendeBalanse(void* filename) {
    FILE* file = fopen((char*)filename, "r");
    int linjenr = 0;
    char verdi[10]; // Assuming a maximum string length of 99 characters
    int* ptr_feil_kode = (int*) malloc(sizeof(int));
    *ptr_feil_kode = 0;

    if (ptr_feil_kode == NULL) {
        printf("Memory not allocated.\n");
        pthread_exit(NULL);
    }

    if (file == NULL) {
        printf("Feil ved åpning av fil. I funksjonen lesFraFilUtgaaendeBalanse.\n");
        printf("Pass på at inn.txt er i samme mappe som programmet.\n");
        *ptr_feil_kode = 9;
        return (void*)ptr_feil_kode;
    }      

    while (fscanf(file, "%5s", verdi) == 1) {
            linjenr++;

            // Check if the input string contains only digits
            for (int index = 0; index < strlen(verdi); index++) {
                if (!isdigit(verdi[index])) {
                    printf("Feil linjenr: %d. Input er ikke et tall i ut.txt.\n", linjenr);
                    *ptr_feil_kode = 10;
                    break;
                }
            }

            if (pthread_mutex_lock(&lock) != 0) {
                printf("Feil ved låsing av mutex. I funksjonen lesFraFilUtgaaendeBalanse.\n");
                *ptr_feil_kode = 11;
                break;
            }

            // Convert the input string to an integer and add it to balanse
            balanse -= atoi(verdi);

            if (pthread_mutex_unlock(&lock) != 0) {
                printf("Feil ved opplåsing av mutex. I funksjonen lesFraFilUtgaaendeBalanse.\n");
                *ptr_feil_kode = 12;
                break;
            }
    }

    if (fclose(file) != 0) {
        printf("Feil ved lukking av fil. I funksjonen lesFraFilUtgaaendeBalanse.\n");
        *ptr_feil_kode = 13;
    }

    return (void*)ptr_feil_kode;
}

int main(int argc, char *argv[]) {
    pthread_t inngaendeBalanseTrad, utgaendeBalanseTrad;
    char* innFil = "inn.txt";
    char* utFil = "ut.txt";
    int* ptrTrad1 = NULL;
    int* ptrTrad2 = NULL;

   
    // Initialiser mutex
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Initialisering av mutex mislyktes. I main-funksjonen.\n");
        return 1;
    }

    // Opprett tråd for å lese fra inn.txt
    if (pthread_create(&inngaendeBalanseTrad, NULL, &lesFraFilInngaaendeBalanse, (void*) innFil)  != 0) {
        printf("Feil - pthread_create() returnerte kode.\n");
        pthread_mutex_destroy(&lock);
        return 2;
    }

    // Opprett tråd for å lese fra ut.txt
    if (pthread_create(&utgaendeBalanseTrad, NULL, &lesFraFilUtgaaendeBalanse, (void*) utFil) != 0) {
        printf("Feil - pthread_create() returnerte kode\n");
        pthread_mutex_destroy(&lock);
        return 3;
    }

    // Vent på at trådene skal bli ferdig
    // endre fra Null til en variabel for å lagre return-verdien
    if (pthread_join(inngaendeBalanseTrad, (void **) &ptrTrad1) != 0) {
        printf("Feil med tråd 1 - pthread_join() returnerte kode.\n");
        if (pthread_mutex_destroy(&lock) != 0) {
            printf("Ødeleggelse av mutex mislyktes.\n");
            return 14;
        };
        return 15;
    }

    if (pthread_join(utgaendeBalanseTrad,  (void **) &ptrTrad2) != 0) {
        printf("Feil med tråd 2 - pthread_join() returnerte kode.\n");
        
        if (pthread_mutex_destroy(&lock) != 0) {
            printf("Ødeleggelse av mutex mislyktes.\n");
            return 16;
        }
        return 17;
    }

// Ødelegg mutex
    if (pthread_mutex_destroy(&lock) != 0) {
        printf("Ødeleggelse av mutex mislyktes\n");
        return 18;
    }

    if (*ptrTrad1 != 0){
        printf("Trad1_error: %d\n", *ptrTrad1);
    }

    if (*ptrTrad2 != 0){
        printf("Trad2_error: %d\n", *ptrTrad2);
    }

    if (*ptrTrad1 == 0 && *ptrTrad2 == 0) {
        printf("Balanse: %d\n", balanse);
    }

    free(ptrTrad1);
    free(ptrTrad2);

    return 0;
}