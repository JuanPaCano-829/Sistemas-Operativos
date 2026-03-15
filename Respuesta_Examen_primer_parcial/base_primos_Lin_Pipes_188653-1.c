/* ============================================================================================================================== */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h> // For wait functions
#include <limits.h>
#include <time.h>

#include <string.h> // Es necesario para usar strlen y medir el tiempo de ejecución
#include "primos_fun.h"


int main(int argc, char* argv[]) 
{
    char hostname[HOST_NAME_MAX + 1];
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        perror("gethostname failed");
        exit(EXIT_FAILURE);
    }
    printf("Computadora: %s\n", hostname);
    
    if(argc == 1)
    {
      printf("-----------------------------------------------\n");
      printf("uso:\n");
      printf(" %s num_HIJOS MODALIDAD_OUT_LOOP\n",argv[0]);
      printf("ejemplo: (valores por default)\n");
      printf("-----------------------------------------------\n");
    }
	
    int int_deltaT;

    int NUM_HIJOS;
    int MODALIDAD_OUT_LOOP;
    clock_t start, end;
    struct timespec elapsed_start, elapsed_end;
    double cpu_time_used;
    double elapsed_time;
	
    char cad[10]; 
    int hijo;
    int n_ini, n_fin;
    
    NUM_HIJOS          = argc > 1 ? atoi(argv[1]) : 5;
    MODALIDAD_OUT_LOOP = argc > 2 ? atoi(argv[2]) : 0;
    //SEMILLA            = argc > 3 ? atoi(argv[3]) : (unsigned int)time(NULL);
	
    pid_t* arr_pid = (pid_t*)(malloc(NUM_HIJOS*sizeof(pid_t)));
    pid_t pid_fin;

    //PRIMERA MODIFICACIÓN
    int fd[10][2]; // para tener 3 hijos 
    char recibe_sms[256]; // Para recibir mensajes de los hijos
	
    start         = clock();
    clock_gettime(CLOCK_REALTIME, &elapsed_start);
	
    for(hijo = 0; hijo < NUM_HIJOS ; hijo++)
    {
      //SEGUNDA MODIFICACIÓN: por si hay un error al crear la pipe
      if (pipe(fd[hijo]) == -1) {
          perror("Error al crear pipe");
          exit(EXIT_FAILURE);
      }
      // =====================================================
      //                        fork()
      // =====================================================
      arr_pid[hijo] = fork();
      // =====================================================
      if (arr_pid[hijo] == -1) 
      {
          perror("fork failed");
          exit(EXIT_FAILURE);
      }

      if (arr_pid[hijo] == 0) 
      {   
          // Cerrar el extremo de lectura de la pipe para el hijo
          close(fd[hijo][0]);
          // proceso hijo...
          int n_ini   = 5 * ( hijo + 1);
          int n_fin   = 5 * ( hijo + 2);
          int IMPRIME = 0; 
          
          long num_inicial = dos_a_la_n(n_ini) -1L; 
          long num_final   = dos_a_la_n(n_fin) -1L; 

          // Comentamos para que el hijo no hable 
          //printf("Proceso hijo %d (PID %d) va a calcular primos.\n",hijo,(int)getpid());
		      //printf("n_ini:%d ,n_fin: %d\n",n_ini,n_fin);
          //printf("obteniendo los primos entre %ld y %ld\n",num_inicial,num_final);

          char cad_res[100];  // primos entre(15,255):48 en 0.000330 segs

          cantidad_de_primos(cad_res,IMPRIME,num_inicial,num_final);

          //TERCERA MODIFICACIÓN: crear el char para el mensaje final y lo imprimimos
          char final_sms[256];
          //sprintf porque write() solo escribe bytes, entonces necesitamos formatear el mensaje antes de enviarlo
          sprintf(final_sms, "quienSoy:%d, %s", hijo, cad_res); // Formatear el mensaje con el número del hijo y los primos encontrados
          write(fd[hijo][1], final_sms, strlen(final_sms) + 1); // Enviar el mensaje a través de la pipe

          //CUARTA MODIFICACIÓN: cerrar la parte de escritura de la pipe
          close(fd[hijo][1]);
		  exit(0);
      }
      else
      {
        //QUINTA MODIFICACIÓN: cerrar el extremo de escritura deL padre
        close(fd[hijo][1]);

        if(MODALIDAD_OUT_LOOP == 0)
        {
          // This is the parent process
          printf("Parent process (PID %d) got child PID: %d\n", (int)getpid(), (int)arr_pid[hijo]);
          // The parent can then use the child_pid for actions like waiting or sending signals
          waitpid(arr_pid[hijo], NULL, 0); // Wait for the child to finish
          printf("Child process %d finished.\n", (int)arr_pid[hijo]);

          //SEXTA MODIFICACIÓN: leer el mensaje del hijo
          // el padre ve la llave de lectura de la pipe y saca el mensaje 
          read(fd[hijo][0], recibe_sms, sizeof(recibe_sms)); //read() bloquea hasta que el hijo escriba algo en la pipe
          printf("%s\n", recibe_sms); // El padre imprime el mensaje que le mandó el hijo
          

          //SEPTIMA MODIFICACIÓN: cerrar la parte de lectura de la pipe
          close(fd[hijo][0]);
         }
      }
    }  // del for para los forks()
	
    if(MODALIDAD_OUT_LOOP)
    {
      printf("Esperando por mis hijos...\n");
         
      for( hijo = 0; hijo < NUM_HIJOS; hijo++)
      {
        //waitpid(arr_pid[hijo], NULL, 0); // Wait for the child to finish			
        //printf("Proceso hijo[%d] con p_id %d ha finalizado.\n", hijo,(int)arr_pid[hijo]);
        pid_fin = wait(0);
        printf("terminó hijo con p_id %d\n",pid_fin);
        //OCTAVA MODIFICACIÓN: leer el mensaje del hijo que terminó y mostrarlo
        read(fd[hijo][0], recibe_sms, sizeof(recibe_sms));
        printf("%s\n", recibe_sms);
        //NOVENA MODIFICACIÓN: cerrar la parte de lectura de la pipe
        close(fd[hijo][0]);
         
      }
         
    }     
	
    end = clock();
    clock_gettime(CLOCK_REALTIME, &elapsed_end);
    cpu_time_used = ((double)end - start) / CLOCKS_PER_SEC;
    elapsed_time  = (elapsed_end.tv_sec  - elapsed_start.tv_sec) + 
                    (elapsed_end.tv_nsec - elapsed_start.tv_nsec) / 1e9;
	
    printf("-------------------------------------------------\n");
    printf("         %s\n",argv[0]);
    printf("-------------------------------------------------\n");	
    printf("NUM_HIJOS:          %d\n",NUM_HIJOS);
    printf("MODALIDAD_OUT_LOOP: %d\n",MODALIDAD_OUT_LOOP);
    printf("CPU Time:           %f seg.\n",cpu_time_used);
    printf("Elapsed time:       %f sec.\n",elapsed_time);
	
    return 0;
}
// ===========================================================================
//                             Fin del Codigo
// ===========================================================================
