/* ===============================================================  
   
   =============================================================== */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>

#include "primos_fun.h"

#define TAM_BLOQUE 200

int main(int argc, char* argv[]) 
{
    if (argc > 1 && strcmp(argv[1], "child") == 0)
    {
        int quienSoy = atoi(argv[2]);
        int IMPRIME  = atoi(argv[3]);
        int n_ini    = atoi(argv[4]);
        int n_fin    = atoi(argv[5]);

        HANDLE hMapFile;
        char* pBuf;

        hMapFile = OpenFileMappingA(
                        FILE_MAP_ALL_ACCESS,
                        FALSE,
                        "MiMemoriaCompartida");

        if (hMapFile == NULL)
        {
            printf("Could not open file mapping object (%lu).\n", GetLastError());
            return 1;
        }

        pBuf = (char*) MapViewOfFile(
                    hMapFile,
                    FILE_MAP_ALL_ACCESS,
                    0,
                    0,
                    TAM_BLOQUE * 10);

        if (pBuf == NULL)
        {
            printf("Could not map view of file (%lu).\n", GetLastError());
            CloseHandle(hMapFile);
            return 1;
        }

        long num_inicial = dos_a_la_n(n_ini) -1L; 
        long num_final   = dos_a_la_n(n_fin) -1L; 

        char cad_res[100];
        char final_sms[TAM_BLOQUE];

        cantidad_de_primos(cad_res,IMPRIME,num_inicial,num_final);
        sprintf(final_sms, "quienSoy:%d, %s", quienSoy, cad_res);

        CopyMemory((PVOID)(pBuf + quienSoy * TAM_BLOQUE), final_sms, strlen(final_sms) + 1);

        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);

        return 0;
    }

    char hostname[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;

    if (!GetComputerNameA(hostname, &size)) {
        printf("GetComputerName failed (%lu)\n", GetLastError());
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
    double cpu_time_used;
    double elapsed_time;
	
    char cad[10]; 
    int hijo;
    int n_ini, n_fin;
    
    NUM_HIJOS          = argc > 1 ? atoi(argv[1]) : 5;
    MODALIDAD_OUT_LOOP = argc > 2 ? atoi(argv[2]) : 0;

    if(NUM_HIJOS > 10)
    {
        printf("NUM_HIJOS debe ser <= 10\n");
        return 1;
    }

    PROCESS_INFORMATION* arr_pi = (PROCESS_INFORMATION*)malloc(NUM_HIJOS * sizeof(PROCESS_INFORMATION));

    HANDLE hMapFile;
    char* pBuf;

    hMapFile = CreateFileMappingA(
                    INVALID_HANDLE_VALUE,
                    NULL,
                    PAGE_READWRITE,
                    0,
                    TAM_BLOQUE * 10,
                    "MiMemoriaCompartida");

    if (hMapFile == NULL)
    {
        printf("Could not create file mapping object (%lu).\n", GetLastError());
        return 1;
    }

    pBuf = (char*) MapViewOfFile(
                hMapFile,
                FILE_MAP_ALL_ACCESS,
                0,
                0,
                TAM_BLOQUE * 10);

    if (pBuf == NULL)
    {
        printf("Could not map view of file (%lu).\n", GetLastError());
        CloseHandle(hMapFile);
        return 1;
    }

    LARGE_INTEGER freq, t1, t2;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t1);
    start = clock();
	
    for(hijo = 0; hijo < NUM_HIJOS ; hijo++)
    {
      STARTUPINFOA si;
      ZeroMemory(&si, sizeof(si));
      si.cb = sizeof(si);

      ZeroMemory(&arr_pi[hijo], sizeof(PROCESS_INFORMATION));

      {
          char cmd[512];
          int n_ini   = 5 * ( hijo + 1);
          int n_fin   = 5 * ( hijo + 2);
          int IMPRIME = 0;

          sprintf(cmd, "\"%s\" child %d %d %d %d",
                  argv[0],
                  hijo,
                  IMPRIME,
                  n_ini,
                  n_fin);

          if (!CreateProcessA(
                  NULL,
                  cmd,
                  NULL,
                  NULL,
                  FALSE,
                  0,
                  NULL,
                  NULL,
                  &si,
                  &arr_pi[hijo]))
          {
              printf("CreateProcess failed (%lu)\n", GetLastError());
              exit(EXIT_FAILURE);
          }
      }

      if(MODALIDAD_OUT_LOOP == 0)
      {
          WaitForSingleObject(arr_pi[hijo].hProcess, INFINITE);
          printf("%s\n", pBuf + hijo * TAM_BLOQUE);
          CloseHandle(arr_pi[hijo].hProcess);
          CloseHandle(arr_pi[hijo].hThread);
      }

    }  // del for para los procesos hijos
	
    if(MODALIDAD_OUT_LOOP)
    {
      for( hijo = 0; hijo < NUM_HIJOS; hijo++)
      {
        WaitForSingleObject(arr_pi[hijo].hProcess, INFINITE);
        CloseHandle(arr_pi[hijo].hProcess);
        CloseHandle(arr_pi[hijo].hThread);
      }

      for( hijo = 0; hijo < NUM_HIJOS; hijo++)
      {
        printf("%s\n", pBuf + hijo * TAM_BLOQUE);
      }
         
    }     
	
    end = clock();
    QueryPerformanceCounter(&t2);

    cpu_time_used = ((double)end - start) / CLOCKS_PER_SEC;
    elapsed_time  = (double)(t2.QuadPart - t1.QuadPart) / (double)freq.QuadPart;
	
    printf("-------------------------------------------------\n");
    printf("         %s\n",argv[0]);
    printf("-------------------------------------------------\n");	
    printf("NUM_HIJOS:          %d\n",NUM_HIJOS);
    printf("MODALIDAD_OUT_LOOP: %d\n",MODALIDAD_OUT_LOOP);
    printf("CPU Time:           %f seg.\n",cpu_time_used);
    printf("Elapsed time:       %f sec.\n",elapsed_time);

    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
    free(arr_pi);
	
    return 0;
}
// ===========================================================================
//                             Fin del Codigo
// ===========================================================================