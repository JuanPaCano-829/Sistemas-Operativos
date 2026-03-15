/* =============================================================== 
   Primer Examen Parcial - Ejercicio 2: Memoria Compartida en Linux
   =============================================================== */

#include <sys/types.h>   // Define tipos del sistema como pid_t
#include <unistd.h>      // Contiene funciones POSIX como fork() y gethostname()
#include <stdio.h>       // Entrada y salida estándar: printf(), perror()
#include <stdlib.h>      // Funciones generales: atoi(), malloc(), free(), exit()
#include <sys/wait.h>    // Funciones wait() y waitpid() para esperar hijos
#include <limits.h>      // Constantes de límites del sistema, como HOST_NAME_MAX
#include <time.h>        // Funciones y tipos para medir tiempo: clock(), timespec, clock_gettime()
#include <sys/ipc.h>     // Funciones y tipos de IPC, como ftok()
#include <sys/shm.h>     // Funciones de memoria compartida: shmget(), shmat(), shmctl()
#include <string.h>      // Funciones de manejo de cadenas: strcpy(), sprintf()

#include "primos_fun.h"  // Header propio donde vienen dos_a_la_n() y cantidad_de_primos()

#define TAM_BLOQUE 200   // Tamaño en bytes reservado para cada respuesta de cada hijo en memoria compartida

int main(int argc, char* argv[]) 
{
    char hostname[HOST_NAME_MAX + 1];  // Arreglo para guardar el nombre de la computadora

    // Intenta obtener el hostname de la máquina
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        perror("gethostname failed");  // Si falla, imprime error del sistema
        exit(EXIT_FAILURE);            // Sale del programa con error
    }

    // Imprime el nombre de la computadora
    printf("Computadora: %s\n", hostname);
    
    // Si no se pasaron argumentos, imprime ayuda básica de uso
    if(argc == 1)
    {
      printf("-----------------------------------------------\n");
      printf("uso:\n");
      printf(" %s num_HIJOS MODALIDAD_OUT_LOOP\n",argv[0]);
      printf("ejemplo: (valores por default)\n");
      printf("-----------------------------------------------\n");
    }
	
    int int_deltaT;  // Variable declarada pero no usada en este código

    int NUM_HIJOS;            // Cantidad de procesos hijo a crear
    int MODALIDAD_OUT_LOOP;   // Modo de espera del padre: 0 dentro del loop, 1 al final
    clock_t start, end;       // Variables para tiempo de CPU
    struct timespec elapsed_start, elapsed_end; // Variables para tiempo transcurrido real
    double cpu_time_used;     // Tiempo de CPU usado
    double elapsed_time;      // Tiempo transcurrido real
	
    char cad[10];  // Variable declarada pero no usada
    int hijo;      // Índice del hijo dentro del ciclo
    int n_ini, n_fin; // Variables declaradas pero luego se vuelven a declarar dentro del hijo
    
    // Si se pasó el argumento 1, lo toma como número de hijos; si no, usa 5
    NUM_HIJOS          = argc > 1 ? atoi(argv[1]) : 5;

    // Si se pasó el argumento 2, lo toma como modalidad; si no, usa 0
    MODALIDAD_OUT_LOOP = argc > 2 ? atoi(argv[2]) : 0;

    // Protege el programa para no permitir más de 10 hijos
    if(NUM_HIJOS > 10)
    {
      printf("NUM_HIJOS debe ser <= 10\n");
      return 1;  // Termina el programa con código de error 1
    }
	
    // Reserva memoria dinámica para guardar los PID de los hijos
    pid_t* arr_pid = (pid_t*)(malloc(NUM_HIJOS*sizeof(pid_t)));

    // Variable para guardar el PID de un hijo que termina cuando se usa wait()
    pid_t pid_fin;

    // Genera una llave IPC a partir del archivo "text" y el id 60
    key_t key = ftok("text", 60);

    // Crea o abre un segmento de memoria compartida con tamaño NUM_HIJOS * TAM_BLOQUE
    int shmid = shmget(key, NUM_HIJOS * TAM_BLOQUE, 0666 | IPC_CREAT);

    // Adjunta la memoria compartida al espacio de direcciones del proceso y guarda el apuntador base en shm
    char* shm = (char*) shmat(shmid, (void*)0, 0);
	
    // Guarda el tiempo de CPU inicial
    start = clock();

    // Guarda el tiempo real inicial
    clock_gettime(CLOCK_REALTIME, &elapsed_start);
	
    // Ciclo para crear cada hijo
    for(hijo = 0; hijo < NUM_HIJOS ; hijo++)
    {
      // =====================================================
      //                        fork()
      // =====================================================

      // Crea un proceso hijo; en el padre regresa el PID del hijo, en el hijo regresa 0
      arr_pid[hijo] = fork();

      // =====================================================

      // Si fork() regresa -1, hubo error al crear el hijo
      if (arr_pid[hijo] == -1) 
      {
          perror("fork failed");  // Imprime el error del sistema
          exit(EXIT_FAILURE);     // Sale del programa con error
      }

      // Si fork() regresó 0, estamos dentro del proceso hijo
      if (arr_pid[hijo] == 0) 
      {   
          // proceso hijo...

          int quienSoy = hijo;             // Identificador del hijo actual
          int n_ini    = 5 * ( hijo + 1);  // Valor de n inicial para este hijo
          int n_fin    = 5 * ( hijo + 2);  // Valor de n final para este hijo
          int IMPRIME  = 0;                // Bandera para que cantidad_de_primos no imprima internamente
          
          // Calcula el rango inferior como 2^n_ini - 1
          long num_inicial = dos_a_la_n(n_ini) -1L; 

          // Calcula el rango superior como 2^n_fin - 1
          long num_final   = dos_a_la_n(n_fin) -1L; 

          // Cadena donde se guardará el resultado del conteo de primos
          char cad_res[100];

          // Cadena final que el hijo escribirá en memoria compartida
          char final_sms[TAM_BLOQUE];

          // Llama a la función que cuenta primos en el rango y guarda el resultado en cad_res
          cantidad_de_primos(cad_res,IMPRIME,num_inicial,num_final);

          // Construye la cadena final incluyendo quién es el hijo
          sprintf(final_sms, "quienSoy:%d, %s", quienSoy, cad_res);

          // Copia la cadena final en la zona de memoria compartida correspondiente a este hijo
          strcpy(shm + quienSoy * TAM_BLOQUE, final_sms);

          // El hijo termina su ejecución
		  exit(0);
      }
      else
      {
        // Si estamos en el padre y la modalidad es 0...
        if(MODALIDAD_OUT_LOOP == 0)
        {
	      // Espera a que termine el hijo actual antes de continuar con el siguiente
          waitpid(arr_pid[hijo], NULL, 0); // Wait for the child to finish

          // Imprime el mensaje que el hijo escribió en su bloque de memoria compartida
          printf("%s\n", shm + hijo * TAM_BLOQUE);
         }
      }
    }  // del for para los forks()
	
    // Si la modalidad es 1, el padre espera por todos los hijos al final
    if(MODALIDAD_OUT_LOOP)
    {
      // Espera a que terminen todos los hijos
      for( hijo = 0; hijo < NUM_HIJOS; hijo++)
      {
        // wait(0) espera a cualquier hijo y regresa su PID
        pid_fin = wait(0);
      }

      // Una vez que todos terminaron, imprime las respuestas de todos desde memoria compartida
      for( hijo = 0; hijo < NUM_HIJOS; hijo++)
      {
        printf("%s\n", shm + hijo * TAM_BLOQUE);
      }
         
    }     
	
    // Guarda el tiempo de CPU final
    end = clock();

    // Guarda el tiempo real final
    clock_gettime(CLOCK_REALTIME, &elapsed_end);

    // Calcula el tiempo de CPU usado
    cpu_time_used = ((double)end - start) / CLOCKS_PER_SEC;

    // Calcula el tiempo real transcurrido en segundos
    elapsed_time  = (elapsed_end.tv_sec  - elapsed_start.tv_sec) + 
                    (elapsed_end.tv_nsec - elapsed_start.tv_nsec) / 1e9;
	
    // Imprime un resumen final
    printf("-------------------------------------------------\n");
    printf("         %s\n",argv[0]);
    printf("-------------------------------------------------\n");	
    printf("NUM_HIJOS:          %d\n",NUM_HIJOS);
    printf("MODALIDAD_OUT_LOOP: %d\n",MODALIDAD_OUT_LOOP);
    printf("CPU Time:           %f seg.\n",cpu_time_used);
    printf("Elapsed time:       %f sec.\n",elapsed_time);

    // Desvincula la memoria compartida del proceso actual
    shmdt(shm);

    // Marca el segmento de memoria compartida para ser destruido
    shmctl(shmid, IPC_RMID, NULL);

    // Libera la memoria dinámica usada para guardar los PID
    free(arr_pid);
	
    return 0;  // Termina correctamente
}

// ===========================================================================
//                             Fin del Codigo
// ===========================================================================