/* ===============================================================
   
   =============================================================== */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h> // For wait functions
#include <limits.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

#include "primos_fun.h"

#define TAM_BLOQUE 200

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

    key_t key = ftok("text", 60);
    int shmid = shmget(key, NUM_HIJOS * TAM_BLOQUE, 0666 | IPC_CREAT);
    char* shm = (char*) shmat(shmid, (void*)0, 0);
	
    start         = clock();
    clock_gettime(CLOCK_REALTIME, &elapsed_start);
	
    for(hijo = 0; hijo < NUM_HIJOS ; hijo++)
    {
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
          // proceso hijo...
          int n_ini   = 5 * ( hijo + 1);
          int n_fin   = 5 * ( hijo + 2);
          int IMPRIME = 0; 
          
          long num_inicial = dos_a_la_n(n_ini) -1L; 
          long num_final   = dos_a_la_n(n_fin) -1L; 

          printf("Proceso hijo %d (PID %d) va a calcular primos.\n",hijo,(int)getpid());
		  printf("n_ini:%d ,n_fin: %d\n",n_ini,n_fin);
          printf("obteniendo los primos entre %ld y %ld\n",num_inicial,num_final);

          char cad_res[100];  // primos entre(15,255):48 en 0.000330 segs
          char final_sms[TAM_BLOQUE];

          cantidad_de_primos(cad_res,IMPRIME,num_inicial,num_final);
          sprintf(final_sms, "quienSoy:%d, %s", hijo, cad_res);

          strcpy(shm + hijo * TAM_BLOQUE, final_sms);
  
          printf("hijo:%d, %s --- %s\n",hijo,argv[0],cad_res);

		  exit(0);
      }
      else
      {
        if(MODALIDAD_OUT_LOOP == 0)
        {
	  // This is the parent process
          printf("Parent process (PID %d) got child PID: %d\n", (int)getpid(), (int)arr_pid[hijo]);
	  // The parent can then use the child_pid for actions like waiting or sending signals
	  waitpid(arr_pid[hijo], NULL, 0); // Wait for the child to finish
          printf("%s\n", shm + hijo * TAM_BLOQUE);
	  printf("Child process %d finished.\n", (int)arr_pid[hijo]);
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
      }

      for( hijo = 0; hijo < NUM_HIJOS; hijo++)
      {
        printf("%s\n", shm + hijo * TAM_BLOQUE);
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

    shmdt(shm);
    shmctl(shmid, IPC_RMID, NULL);
    free(arr_pid);
	
    return 0;
}
// ===========================================================================
//                             Fin del Codigo
// ===========================================================================