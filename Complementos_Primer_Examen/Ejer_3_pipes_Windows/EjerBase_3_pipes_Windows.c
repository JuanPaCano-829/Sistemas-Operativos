/* ===============================================================
   VERSION WINDOWS con pipes
   =============================================================== */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "primos_fun.h"

int main(int argc, char* argv[])
{
    char hostname[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;

    if (!GetComputerNameA(hostname, &size)) {
        fprintf(stderr, "GetComputerName failed (%lu)\n", GetLastError());
        exit(EXIT_FAILURE);
    }
    printf("Computadora: %s\n", hostname);

    if (argc == 1) {
        printf("-----------------------------------------------\n");
        printf("uso:\n");
        printf(" %s num_HIJOS MODALIDAD_OUT_LOOP\n", argv[0]);
        printf("ejemplo: (valores por default)\n");
        printf("-----------------------------------------------\n");
    }

    int NUM_HIJOS;
    int MODALIDAD_OUT_LOOP;

    clock_t start, end;
    double cpu_time_used;
    double elapsed_time;

    int hijo;

    NUM_HIJOS          = argc > 1 ? atoi(argv[1]) : 5;
    MODALIDAD_OUT_LOOP = argc > 2 ? atoi(argv[2]) : 0;

    PROCESS_INFORMATION* arr_pi =
        (PROCESS_INFORMATION*)malloc(NUM_HIJOS * sizeof(PROCESS_INFORMATION));

    HANDLE* arr_read =
        (HANDLE*)malloc(NUM_HIJOS * sizeof(HANDLE));

    LARGE_INTEGER freq, t1, t2;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t1);
    start = clock();

    for (hijo = 0; hijo < NUM_HIJOS; hijo++)
    {
        HANDLE hRead, hWrite;
        SECURITY_ATTRIBUTES sa;

        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;

        if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
            fprintf(stderr, "CreatePipe failed (%lu)\n", GetLastError());
            exit(EXIT_FAILURE);
        }

        arr_read[hijo] = hRead;

        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);

        ZeroMemory(&arr_pi[hijo], sizeof(PROCESS_INFORMATION));

        {
            char cmd[512];
            int n_ini   = 5 * (hijo + 1);
            int n_fin   = 5 * (hijo + 2);
            int IMPRIME = 0;

            sprintf(cmd, "\"%s\" child %d %d %d %d %llu",
                    argv[0],
                    hijo,
                    IMPRIME,
                    n_ini,
                    n_fin,
                    (unsigned long long)(uintptr_t)hWrite);

            if (!CreateProcessA(
                    NULL,
                    cmd,
                    NULL,
                    NULL,
                    TRUE,
                    0,
                    NULL,
                    NULL,
                    &si,
                    &arr_pi[hijo]))
            {
                fprintf(stderr, "CreateProcess failed (%lu)\n", GetLastError());
                exit(EXIT_FAILURE);
            }
        }

        CloseHandle(hWrite);

        if (MODALIDAD_OUT_LOOP == 0)
        {
            DWORD bytesRead;
            char buffer[256];

            printf("Parent process got child PID: %lu\n",
                   (unsigned long)arr_pi[hijo].dwProcessId);

            WaitForSingleObject(arr_pi[hijo].hProcess, INFINITE);

            ZeroMemory(buffer, sizeof(buffer));
            if (ReadFile(arr_read[hijo], buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
                buffer[bytesRead] = '\0';
                printf("%s\n", buffer);
            }

            printf("Child process %lu finished.\n",
                   (unsigned long)arr_pi[hijo].dwProcessId);

            CloseHandle(arr_read[hijo]);
            CloseHandle(arr_pi[hijo].hProcess);
            CloseHandle(arr_pi[hijo].hThread);
        }
    }

    if (argc > 1 && strcmp(argv[1], "child") == 0)
    {
        int quienSoy = atoi(argv[2]);
        int IMPRIME  = atoi(argv[3]);
        int n_ini    = atoi(argv[4]);
        int n_fin    = atoi(argv[5]);
        HANDLE hWrite = (HANDLE)(uintptr_t)_strtoui64(argv[6], NULL, 10);

        long num_inicial = dos_a_la_n(n_ini) - 1L;
        long num_final   = dos_a_la_n(n_fin) - 1L;

        char cad_res[100];
        char final_sms[200];
        DWORD bytesWritten;

        printf("Proceso hijo %d (PID %lu) va a calcular primos.\n",
               quienSoy, (unsigned long)GetCurrentProcessId());
        printf("n_ini:%d ,n_fin: %d\n", n_ini, n_fin);
        printf("obteniendo los primos entre %ld y %ld\n", num_inicial, num_final);

        cantidad_de_primos(cad_res, IMPRIME, num_inicial, num_final);

        sprintf(final_sms, "quienSoy:%d, %s", quienSoy, cad_res);

        WriteFile(hWrite, final_sms, (DWORD)strlen(final_sms), &bytesWritten, NULL);
        CloseHandle(hWrite);
        return 0;
    }

    if (MODALIDAD_OUT_LOOP)
    {
        printf("Esperando por mis hijos...\n");

        for (hijo = 0; hijo < NUM_HIJOS; hijo++)
        {
            DWORD bytesRead;
            char buffer[256];

            WaitForSingleObject(arr_pi[hijo].hProcess, INFINITE);

            ZeroMemory(buffer, sizeof(buffer));
            if (ReadFile(arr_read[hijo], buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
                buffer[bytesRead] = '\0';
                printf("%s\n", buffer);
            }

            printf("terminó hijo con p_id %lu\n",
                   (unsigned long)arr_pi[hijo].dwProcessId);

            CloseHandle(arr_read[hijo]);
            CloseHandle(arr_pi[hijo].hProcess);
            CloseHandle(arr_pi[hijo].hThread);
        }
    }

    end = clock();
    QueryPerformanceCounter(&t2);

    cpu_time_used = ((double)end - start) / CLOCKS_PER_SEC;
    elapsed_time  = (double)(t2.QuadPart - t1.QuadPart) / (double)freq.QuadPart;

    printf("-------------------------------------------------\n");
    printf("         %s\n", argv[0]);
    printf("-------------------------------------------------\n");
    printf("NUM_HIJOS:          %d\n", NUM_HIJOS);
    printf("MODALIDAD_OUT_LOOP: %d\n", MODALIDAD_OUT_LOOP);
    printf("CPU Time:           %f seg.\n", cpu_time_used);
    printf("Elapsed time:       %f sec.\n", elapsed_time);

    free(arr_pi);
    free(arr_read);

    return 0;
}