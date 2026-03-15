/* ==================================================================================
   Examen Parcial - Ejercicio II) Memoria Compartida Linux
   ================================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

#include "primos_fun.h"

int main(int argc, char* argv[]) 
{
    int NUM_HIJOS = argc > 1 ? atoi(argv[1]) : 3;
    int MODALIDAD = argc > 2 ? atoi(argv[2]) : 0;
    
    // 1. Crear la LLAVE única basada en un archivo existente (puedes usar "primos_fun.h")
    key_t key = ftok("primos_fun.h", 65);
    
    // 2. Crear el SEGMENTO (1024 bytes es suficiente para 10 hijos x 100 bytes)
    int shmid = shmget(key, 1024, 0666 | IPC_CREAT);
    
    // 3. ATTACH: El padre se conecta a la memoria
    char *shared_mem = (char*) shmat(shmid, (void*)0, 0);

    for(int hijo = 0; hijo < NUM_HIJOS; hijo++)
    {
        pid_t pid = fork();

        if (pid == 0) // Proceso HIJO
        {
            // El hijo hereda el 'shmid' y la dirección 'shared_mem' por ser un fork()
            int n_ini = 5 * (hijo + 1);
            int n_fin = 5 * (hijo + 2);
            char cad_res[100];
            char final_sms[256];

            long num_inicial = dos_a_la_n(n_ini) - 1L;
            long num_final   = dos_a_la_n(n_fin) - 1L;

            cantidad_de_primos(cad_res, 0, num_inicial, num_final);
            sprintf(final_sms, "quienSoy:%d, %s", hijo, cad_res);

            // 4. ESCRITURA CON OFFSET (Instrucción 127)
            // Calculamos la posición: base + (hijo * 100)
            char *posicion_escritura = shared_mem + (hijo * 100);
            strcpy(posicion_escritura, final_sms);

            // El hijo se desvía de la memoria antes de salir
            shmdt(shared_mem);
            exit(0);
        }
        else // Proceso PADRE
        {
            if (MODALIDAD == 0)
            {
                waitpid(pid, NULL, 0);
                // El padre lee del cajón correspondiente al hijo actual
                printf("Padre lee (Hijo %d): %s\n", hijo, shared_mem + (hijo * 100));
            }
        }
    }

    if (MODALIDAD == 1)
    {
        printf("Esperando a todos los hijos (Al final)...\n");
        for (int i = 0; i < NUM_HIJOS; i++) wait(NULL);
        
        for (int i = 0; i < NUM_HIJOS; i++)
        {
            // El padre recorre el "pizarrón" leyendo cada 100 bytes
            printf("Lectura Memoria Offset %d: %s\n", i * 100, shared_mem + (i * 100));
        }
    }

    // 5. LIMPIEZA FINAL
    shmdt(shared_mem);
    shmctl(shmid, IPC_RMID, NULL); // Borrar el segmento del sistema

    return 0;
}