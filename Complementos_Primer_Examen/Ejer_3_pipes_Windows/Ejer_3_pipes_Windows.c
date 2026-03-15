/* ===============================================================
   Primer Examen Parcial - Ejercicio 3: Pipes en Windows
   =============================================================== */

#include <windows.h>   // API de Windows: procesos, pipes, handles, tiempo de alta resolución
#include <stdio.h>     // printf(), fprintf(), sprintf()
#include <stdlib.h>    // atoi(), malloc(), free(), exit()
#include <time.h>      // clock()
#include <string.h>    // strcmp(), strlen()
#include <stdint.h>    // uintptr_t para conversiones seguras de punteros/handles

#include "primos_fun.h" // Funciones propias: dos_a_la_n() y cantidad_de_primos()

int main(int argc, char* argv[])
{
    // Si el programa fue invocado en modo "child", entra a esta rama
    if (argc > 1 && strcmp(argv[1], "child") == 0)
    {
        // argv[2] trae el identificador del hijo
        int quienSoy = atoi(argv[2]);

        // argv[3] trae la bandera IMPRIME
        int IMPRIME  = atoi(argv[3]);

        // argv[4] trae n_ini
        int n_ini    = atoi(argv[4]);

        // argv[5] trae n_fin
        int n_fin    = atoi(argv[5]);

        // argv[6] trae el handle del extremo de escritura del pipe, convertido desde texto
        HANDLE hWrite = (HANDLE)(uintptr_t)_strtoui64(argv[6], NULL, 10);

        // Calcula el límite inferior real del rango: 2^n_ini - 1
        long num_inicial = dos_a_la_n(n_ini) - 1L;

        // Calcula el límite superior real del rango: 2^n_fin - 1
        long num_final   = dos_a_la_n(n_fin) - 1L;

        // Buffer donde se guardará el resultado de cantidad_de_primos()
        char cad_res[100];

        // Buffer final que se enviará al padre por el pipe
        char final_sms[200];

        // Variable donde WriteFile guarda la cantidad de bytes escritos
        DWORD bytesWritten;

        // Calcula la cantidad de primos en el rango y deja el texto en cad_res
        cantidad_de_primos(cad_res, IMPRIME, num_inicial, num_final);

        // Construye la cadena final con el formato requerido
        sprintf(final_sms, "quienSoy:%d, %s", quienSoy, cad_res);

        // Escribe la cadena final en el pipe del hijo al padre, incluyendo el terminador '\0'
        WriteFile(hWrite, final_sms, (DWORD)(strlen(final_sms) + 1), &bytesWritten, NULL);

        // Cierra el handle de escritura del pipe
        CloseHandle(hWrite);

        // Termina el proceso hijo correctamente
        return 0;
    }

    // Buffer para guardar el nombre de la computadora
    char hostname[MAX_COMPUTERNAME_LENGTH + 1];

    // Tamaño del buffer del hostname
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;

    // Obtiene el nombre de la computadora
    if (!GetComputerNameA(hostname, &size)) {
        // Si falla, imprime error a stderr
        fprintf(stderr, "GetComputerName failed (%lu)\n", GetLastError());

        // Termina el programa con error
        exit(EXIT_FAILURE);
    }

    // Imprime el nombre de la computadora
    printf("Computadora: %s\n", hostname);

    // Si no se pasan argumentos, imprime ayuda básica de uso
    if (argc == 1) {
        printf("-----------------------------------------------\n");
        printf("uso:\n");
        printf(" %s num_HIJOS MODALIDAD_OUT_LOOP\n", argv[0]);
        printf("ejemplo: (valores por default)\n");
        printf("-----------------------------------------------\n");
    }

    // Número de hijos a crear
    int NUM_HIJOS;

    // Modalidad de espera del padre
    int MODALIDAD_OUT_LOOP;

    // Variables para medir tiempo de CPU
    clock_t start, end;

    // Tiempo de CPU usado
    double cpu_time_used;

    // Tiempo transcurrido real
    double elapsed_time;

    // Índice del hijo dentro del ciclo
    int hijo;

    // Toma NUM_HIJOS de argv[1] o usa 5 por default
    NUM_HIJOS          = argc > 1 ? atoi(argv[1]) : 5;

    // Toma MODALIDAD_OUT_LOOP de argv[2] o usa 0 por default
    MODALIDAD_OUT_LOOP = argc > 2 ? atoi(argv[2]) : 0;

    // Protege para no permitir más de 10 hijos
    if (NUM_HIJOS > 10) {
        printf("NUM_HIJOS debe ser <= 10\n");
        return 1;
    }

    // Reserva memoria para guardar la información de cada proceso hijo
    PROCESS_INFORMATION* arr_pi =
        (PROCESS_INFORMATION*)malloc(NUM_HIJOS * sizeof(PROCESS_INFORMATION));

    // Reserva memoria para guardar el extremo de lectura del pipe de cada hijo
    HANDLE* arr_read =
        (HANDLE*)malloc(NUM_HIJOS * sizeof(HANDLE));

    // Variables para medir tiempo real con alta resolución
    LARGE_INTEGER freq, t1, t2;

    // Obtiene la frecuencia del contador de alta resolución
    QueryPerformanceFrequency(&freq);

    // Guarda el tiempo real inicial
    QueryPerformanceCounter(&t1);

    // Guarda el tiempo de CPU inicial
    start = clock();

    // Ciclo para crear los NUM_HIJOS
    for (hijo = 0; hijo < NUM_HIJOS; hijo++)
    {
        // Handle del extremo de lectura del pipe para este hijo
        HANDLE hRead, hWrite;

        // Estructura para atributos de seguridad del pipe
        SECURITY_ATTRIBUTES sa;

        // Tamaño de la estructura
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);

        // Sin descriptor de seguridad personalizado
        sa.lpSecurityDescriptor = NULL;

        // Hace heredables los handles para que el hijo pueda usar el pipe
        sa.bInheritHandle = TRUE;

        // Crea un pipe anónimo: hRead para leer, hWrite para escribir
        if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
            // Si falla, imprime error
            fprintf(stderr, "CreatePipe failed (%lu)\n", GetLastError());

            // Sale con error
            exit(EXIT_FAILURE);
        }

        // Guarda el extremo de lectura en el arreglo para que el padre lo use después
        arr_read[hijo] = hRead;

        // Estructura STARTUPINFO del proceso hijo
        STARTUPINFOA si;

        // Inicializa toda la estructura a cero
        ZeroMemory(&si, sizeof(si));

        // Coloca el tamaño correcto de la estructura
        si.cb = sizeof(si);

        // Inicializa a cero la estructura PROCESS_INFORMATION del hijo actual
        ZeroMemory(&arr_pi[hijo], sizeof(PROCESS_INFORMATION));

        {
            // Buffer para construir la línea de comandos con la que se lanzará el hijo
            char cmd[512];

            // n_ini para este hijo: 5, 10, 15, ...
            int n_ini   = 5 * (hijo + 1);

            // n_fin para este hijo: 10, 15, 20, ...
            int n_fin   = 5 * (hijo + 2);

            // Bandera IMPRIME fija en 0
            int IMPRIME = 0;

            // Construye la línea de comandos:
            // programa child quienSoy IMPRIME n_ini n_fin hWrite
            sprintf(cmd, "\"%s\" child %d %d %d %d %llu",
                    argv[0],
                    hijo,
                    IMPRIME,
                    n_ini,
                    n_fin,
                    (unsigned long long)(uintptr_t)hWrite);

            // Crea el proceso hijo
            if (!CreateProcessA(
                    NULL,                  // nombre del módulo, NULL porque viene en cmd
                    cmd,                   // línea de comandos completa
                    NULL,                  // seguridad del proceso
                    NULL,                  // seguridad del hilo
                    TRUE,                  // permite heredar handles
                    0,                     // flags de creación
                    NULL,                  // ambiente
                    NULL,                  // directorio actual
                    &si,                   // startup info
                    &arr_pi[hijo]))        // info del proceso creado
            {
                // Si falla, imprime error
                fprintf(stderr, "CreateProcess failed (%lu)\n", GetLastError());

                // Termina con error
                exit(EXIT_FAILURE);
            }
        }

        // El padre ya no necesita el extremo de escritura del pipe, así que lo cierra
        CloseHandle(hWrite);

        // Si la modalidad es 0, espera al hijo inmediatamente dentro del loop
        if (MODALIDAD_OUT_LOOP == 0)
        {
            // Cantidad de bytes leídos desde el pipe
            DWORD bytesRead;

            // Buffer para recibir la respuesta del hijo
            char buffer[256];

            // Espera a que termine el hijo actual
            WaitForSingleObject(arr_pi[hijo].hProcess, INFINITE);

            // Limpia el buffer antes de usarlo
            ZeroMemory(buffer, sizeof(buffer));

            // Lee del pipe la respuesta que mandó el hijo
            if (ReadFile(arr_read[hijo], buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
                // Coloca terminador nulo al final de lo leído
                buffer[bytesRead] = '\0';

                // Imprime la cadena que envió el hijo
                printf("%s\n", buffer);
            }

            // Cierra el handle de lectura del pipe
            CloseHandle(arr_read[hijo]);

            // Cierra el handle del proceso hijo
            CloseHandle(arr_pi[hijo].hProcess);

            // Cierra el handle del hilo principal del hijo
            CloseHandle(arr_pi[hijo].hThread);
        }
    }

    // Si la modalidad es 1, el padre crea todos los hijos y luego espera por ellos al final
    if (MODALIDAD_OUT_LOOP)
    {
        // Mensaje informativo
        printf("Esperando por mis hijos...\n");

        // Recorre todos los hijos creados
        for (hijo = 0; hijo < NUM_HIJOS; hijo++)
        {
            // Cantidad de bytes leídos desde el pipe
            DWORD bytesRead;

            // Buffer para recibir la respuesta del hijo
            char buffer[256];

            // Espera a que termine el hijo actual
            WaitForSingleObject(arr_pi[hijo].hProcess, INFINITE);

            // Limpia el buffer
            ZeroMemory(buffer, sizeof(buffer));

            // Lee del pipe la respuesta del hijo
            if (ReadFile(arr_read[hijo], buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
                // Coloca terminador nulo
                buffer[bytesRead] = '\0';

                // Imprime la cadena que mandó el hijo
                printf("%s\n", buffer);
            }

            // Cierra el handle de lectura del pipe
            CloseHandle(arr_read[hijo]);

            // Cierra el handle del proceso hijo
            CloseHandle(arr_pi[hijo].hProcess);

            // Cierra el handle del hilo del hijo
            CloseHandle(arr_pi[hijo].hThread);
        }
    }

    // Guarda el tiempo de CPU final
    end = clock();

    // Guarda el tiempo real final
    QueryPerformanceCounter(&t2);

    // Calcula el tiempo de CPU usado
    cpu_time_used = ((double)end - start) / CLOCKS_PER_SEC;

    // Calcula el tiempo transcurrido real en segundos
    elapsed_time  = (double)(t2.QuadPart - t1.QuadPart) / (double)freq.QuadPart;

    // Imprime el resumen final
    printf("-------------------------------------------------\n");
    printf("         %s\n", argv[0]);
    printf("-------------------------------------------------\n");
    printf("NUM_HIJOS:          %d\n", NUM_HIJOS);
    printf("MODALIDAD_OUT_LOOP: %d\n", MODALIDAD_OUT_LOOP);
    printf("CPU Time:           %f seg.\n", cpu_time_used);
    printf("Elapsed time:       %f sec.\n", elapsed_time);

    // Libera la memoria reservada para el arreglo de procesos
    free(arr_pi);

    // Libera la memoria reservada para el arreglo de handles de lectura
    free(arr_read);

    // Termina correctamente
    return 0;
}