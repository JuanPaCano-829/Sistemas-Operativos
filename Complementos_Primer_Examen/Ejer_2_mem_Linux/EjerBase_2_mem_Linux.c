#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include "primos_fun.h"

int main(int argc, char* argv[]) {
    int NUM_HIJOS = argc > 1 ? atoi(argv[1]) : 5;
    int MODALIDAD_OUT_LOOP = argc > 2 ? atoi(argv[2]) : 0;
    
    // Configuración de Memoria Compartida
    key_t key = ftok("primos_fun.h", 65);
    int shmid = shmget(key, 1024, 0666 | IPC_CREAT);
    char *shared_mem = (char*) shmat(shmid, (void*)0, 0);

    for(int hijo = 0; hijo < NUM_HIJOS ; hijo++) {
        pid_t pid = fork();
        if (pid == 0) { // Proceso Hijo
            int n_ini = 5 * (hijo + 1);
            int n_fin = 5 * (hijo + 2);
            char cad_res[100];
            cantidad_de_primos(cad_res, 0, dos_a_la_n(n_ini)-1, dos_a_la_n(n_fin)-1);
            
            // Escritura con offset de 100 bytes
            sprintf(shared_mem + (hijo * 100), "quienSoy:%d, %s", hijo, cad_res);
            shmdt(shared_mem);
            exit(0);
        } else { // Proceso Padre
            if(MODALIDAD_OUT_LOOP == 0) {
                waitpid(pid, NULL, 0);
                printf("%s\n", shared_mem + (hijo * 100));
            }
        }
    }

    if(MODALIDAD_OUT_LOOP) {
        for(int i = 0; i < NUM_HIJOS; i++) wait(NULL);
        for(int i = 0; i < NUM_HIJOS; i++) printf("%s\n", shared_mem + (i * 100));
    }

    shmdt(shared_mem);
    shmctl(shmid, IPC_RMID, NULL); // Limpieza
    return 0;
}