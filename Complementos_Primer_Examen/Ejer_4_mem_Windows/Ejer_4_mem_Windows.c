/* ===============================================================  
   Primer Examen Parcial - Ejercicio 4: Memoria Compartida en Windows
   =============================================================== */

#include <windows.h>   // API de Windows: procesos, memoria compartida, handles, tiempo
#include <stdio.h>     // printf(), sprintf()
#include <stdlib.h>    // atoi(), malloc(), free(), exit()
#include <time.h>      // clock()
#include <string.h>    // strcmp(), strlen()
#include <stdint.h>    // Tipos enteros como uintptr_t

#include "primos_fun.h" // Funciones propias: dos_a_la_n() y cantidad_de_primos()

#define TAM_BLOQUE 200  // Tamaño de bytes reservado para la respuesta de cada hijo

int main(int argc, char* argv[]) 
{
    // Si el programa fue lanzado en modo hijo, entra a esta rama
    if (argc > 1 && strcmp(argv[1], "child") == 0)
    {
        // Identificador del hijo, recibido por línea de comandos
        int quienSoy = atoi(argv[2]);

        // Bandera IMPRIME, recibida por línea de comandos
        int IMPRIME  = atoi(argv[3]);

        // Valor n inicial del rango, recibido por línea de comandos
        int n_ini    = atoi(argv[4]);

        // Valor n final del rango, recibido por línea de comandos
        int n_fin    = atoi(argv[5]);

        // Handle al objeto de memoria compartida
        HANDLE hMapFile;

        // Apuntador al buffer mapeado en memoria compartida
        char* pBuf;

        // Abre la memoria compartida ya creada por el padre
        hMapFile = OpenFileMappingA(
                        FILE_MAP_ALL_ACCESS,  // Permiso de lectura y escritura
                        FALSE,                // El handle no será heredable
                        "MiMemoriaCompartida" // Nombre del objeto de memoria compartida
                    );

        // Si no pudo abrir la memoria compartida, imprime error y termina
        if (hMapFile == NULL)
        {
            printf("Could not open file mapping object (%lu).\n", GetLastError());
            return 1;
        }

        // Mapea la memoria compartida al espacio de direcciones del proceso hijo
        pBuf = (char*) MapViewOfFile(
                    hMapFile,             // Handle al objeto de memoria compartida
                    FILE_MAP_ALL_ACCESS,  // Lectura y escritura
                    0,                    // Parte alta del offset
                    0,                    // Parte baja del offset
                    TAM_BLOQUE * 10       // Tamaño total mapeado
                );

        // Si no pudo mapear la memoria, imprime error, cierra handle y termina
        if (pBuf == NULL)
        {
            printf("Could not map view of file (%lu).\n", GetLastError());
            CloseHandle(hMapFile);
            return 1;
        }

        // Calcula el número inicial real del rango: 2^n_ini - 1
        long num_inicial = dos_a_la_n(n_ini) -1L; 

        // Calcula el número final real del rango: 2^n_fin - 1
        long num_final   = dos_a_la_n(n_fin) -1L; 

        // Buffer donde se guardará la respuesta de cantidad_de_primos()
        char cad_res[100];

        // Buffer final que el hijo escribirá en memoria compartida
        char final_sms[TAM_BLOQUE];

        // Calcula la cantidad de primos en el rango y deja el texto en cad_res
        cantidad_de_primos(cad_res,IMPRIME,num_inicial,num_final);

        // Construye la cadena final con el formato pedido
        sprintf(final_sms, "quienSoy:%d, %s", quienSoy, cad_res);

        // Copia la cadena final al bloque de memoria que le corresponde a este hijo
        CopyMemory((PVOID)(pBuf + quienSoy * TAM_BLOQUE), final_sms, strlen(final_sms) + 1);

        // Desmapea la memoria compartida del espacio del hijo
        UnmapViewOfFile(pBuf);

        // Cierra el handle del objeto de memoria compartida
        CloseHandle(hMapFile);

        // Termina el proceso hijo correctamente
        return 0;
    }

    // Buffer para guardar el nombre de la computadora
    char hostname[MAX_COMPUTERNAME_LENGTH + 1];

    // Variable con el tamaño del buffer para el hostname
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;

    // Obtiene el nombre de la computadora
    if (!GetComputerNameA(hostname, &size)) {
        // Si falla, imprime el error y termina
        printf("GetComputerName failed (%lu)\n", GetLastError());
        exit(EXIT_FAILURE);
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
	
    // Variable declarada pero no usada
    int int_deltaT;

    // Número de hijos a crear
    int NUM_HIJOS;

    // Modalidad de espera: 0 = esperar dentro del loop, 1 = esperar al final
    int MODALIDAD_OUT_LOOP;

    // Variables para medir tiempo de CPU
    clock_t start, end;

    // Tiempo de CPU usado
    double cpu_time_used;

    // Tiempo real transcurrido
    double elapsed_time;
	
    // Variable declarada pero no usada
    char cad[10]; 

    // Índice del hijo en el ciclo
    int hijo;

    // Variables declaradas pero luego se vuelven a declarar dentro del loop
    int n_ini, n_fin;
    
    // Toma NUM_HIJOS del argumento 1 o usa 5 por default
    NUM_HIJOS          = argc > 1 ? atoi(argv[1]) : 5;

    // Toma MODALIDAD_OUT_LOOP del argumento 2 o usa 0 por default
    MODALIDAD_OUT_LOOP = argc > 2 ? atoi(argv[2]) : 0;

    // Si NUM_HIJOS es mayor que 10, termina con error
    if(NUM_HIJOS > 10)
    {
        printf("NUM_HIJOS debe ser <= 10\n");
        return 1;
    }

    // Reserva memoria para guardar la información de cada proceso hijo
    PROCESS_INFORMATION* arr_pi = (PROCESS_INFORMATION*)malloc(NUM_HIJOS * sizeof(PROCESS_INFORMATION));

    // Handle al objeto de memoria compartida
    HANDLE hMapFile;

    // Apuntador al buffer mapeado por el padre
    char* pBuf;

    // Crea la memoria compartida nombrada
    hMapFile = CreateFileMappingA(
                    INVALID_HANDLE_VALUE,   // Usa el archivo de paginación del sistema
                    NULL,                   // Seguridad por default
                    PAGE_READWRITE,         // Lectura y escritura
                    0,                      // Tamaño alto
                    TAM_BLOQUE * 10,        // Tamaño total
                    "MiMemoriaCompartida"   // Nombre del objeto de memoria compartida
                );

    // Si no pudo crear la memoria compartida, imprime error y termina
    if (hMapFile == NULL)
    {
        printf("Could not create file mapping object (%lu).\n", GetLastError());
        return 1;
    }

    // Mapea la memoria compartida al espacio del proceso padre
    pBuf = (char*) MapViewOfFile(
                hMapFile,             // Handle del objeto
                FILE_MAP_ALL_ACCESS,  // Lectura y escritura
                0,                    // Offset alto
                0,                    // Offset bajo
                TAM_BLOQUE * 10       // Tamaño total del mapeo
            );

    // Si no pudo mapear la memoria, imprime error, cierra handle y termina
    if (pBuf == NULL)
    {
        printf("Could not map view of file (%lu).\n", GetLastError());
        CloseHandle(hMapFile);
        return 1;
    }

    // Variables para medir tiempo real con alta resolución
    LARGE_INTEGER freq, t1, t2;

    // Obtiene la frecuencia del contador de alta resolución
    QueryPerformanceFrequency(&freq);

    // Guarda el tiempo real inicial
    QueryPerformanceCounter(&t1);

    // Guarda el tiempo de CPU inicial
    start = clock();
	
    // Ciclo para crear todos los hijos
    for(hijo = 0; hijo < NUM_HIJOS ; hijo++)
    {
      // Estructura de arranque del proceso hijo
      STARTUPINFOA si;

      // Inicializa la estructura a cero
      ZeroMemory(&si, sizeof(si));

      // Coloca el tamaño correcto de la estructura
      si.cb = sizeof(si);

      // Inicializa a cero la estructura PROCESS_INFORMATION del hijo actual
      ZeroMemory(&arr_pi[hijo], sizeof(PROCESS_INFORMATION));

      {
          // Buffer para construir la línea de comandos del hijo
          char cmd[512];

          // n inicial para este hijo: 5, 10, 15, ...
          int n_ini   = 5 * ( hijo + 1);

          // n final para este hijo: 10, 15, 20, ...
          int n_fin   = 5 * ( hijo + 2);

          // Bandera IMPRIME fija en 0
          int IMPRIME = 0;

          // Construye la línea de comandos:
          // programa child quienSoy IMPRIME n_ini n_fin
          sprintf(cmd, "\"%s\" child %d %d %d %d",
                  argv[0],
                  hijo,
                  IMPRIME,
                  n_ini,
                  n_fin);

          // Crea el proceso hijo
          if (!CreateProcessA(
                  NULL,           // Nombre del ejecutable, viene en la línea cmd
                  cmd,            // Línea de comandos
                  NULL,           // Seguridad del proceso
                  NULL,           // Seguridad del hilo
                  FALSE,          // No hereda handles
                  0,              // Sin flags especiales
                  NULL,           // Ambiente por default
                  NULL,           // Directorio actual por default
                  &si,            // Estructura de arranque
                  &arr_pi[hijo])) // Información del proceso creado
          {
              // Si falla, imprime error y termina
              printf("CreateProcess failed (%lu)\n", GetLastError());
              exit(EXIT_FAILURE);
          }
      }

      // Si la modalidad es 0, espera a cada hijo dentro del loop
      if(MODALIDAD_OUT_LOOP == 0)
      {
          // Espera a que termine el hijo actual
          WaitForSingleObject(arr_pi[hijo].hProcess, INFINITE);

          // Imprime el contenido que ese hijo escribió en su bloque de memoria compartida
          printf("%s\n", pBuf + hijo * TAM_BLOQUE);

          // Cierra el handle del proceso hijo
          CloseHandle(arr_pi[hijo].hProcess);

          // Cierra el handle del hilo principal del hijo
          CloseHandle(arr_pi[hijo].hThread);
      }

    }  // del for para los procesos hijos
	
    // Si la modalidad es 1, espera a todos los hijos al final
    if(MODALIDAD_OUT_LOOP)
    {
      // Espera a que terminen todos los hijos
      for( hijo = 0; hijo < NUM_HIJOS; hijo++)
      {
        // Espera al hijo actual
        WaitForSingleObject(arr_pi[hijo].hProcess, INFINITE);

        // Cierra el handle del proceso hijo
        CloseHandle(arr_pi[hijo].hProcess);

        // Cierra el handle del hilo principal del hijo
        CloseHandle(arr_pi[hijo].hThread);
      }

      // Después de que todos terminaron, imprime todos los bloques de memoria compartida
      for( hijo = 0; hijo < NUM_HIJOS; hijo++)
      {
        printf("%s\n", pBuf + hijo * TAM_BLOQUE);
      }
         
    }     
	
    // Guarda el tiempo de CPU final
    end = clock();

    // Guarda el tiempo real final
    QueryPerformanceCounter(&t2);

    // Calcula el tiempo de CPU usado
    cpu_time_used = ((double)end - start) / CLOCKS_PER_SEC;

    // Calcula el tiempo real transcurrido en segundos
    elapsed_time  = (double)(t2.QuadPart - t1.QuadPart) / (double)freq.QuadPart;
	
    // Imprime resumen final del programa
    printf("-------------------------------------------------\n");
    printf("         %s\n",argv[0]);
    printf("-------------------------------------------------\n");	
    printf("NUM_HIJOS:          %d\n",NUM_HIJOS);
    printf("MODALIDAD_OUT_LOOP: %d\n",MODALIDAD_OUT_LOOP);
    printf("CPU Time:           %f seg.\n",cpu_time_used);
    printf("Elapsed time:       %f sec.\n",elapsed_time);

    // Desmapea la memoria compartida del espacio del padre
    UnmapViewOfFile(pBuf);

    // Cierra el handle del objeto de memoria compartida
    CloseHandle(hMapFile);

    // Libera el arreglo dinámico de PROCESS_INFORMATION
    free(arr_pi);
	
    // Termina el programa correctamente
    return 0;
}
// ===========================================================================
//                             Fin del Codigo
// ===========================================================================