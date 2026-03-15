#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Prototipos necesarios
long dos_a_la_n(int n);
void cantidad_de_primos(char* cad_res, int IMPRIME, long num_inicial, long num_final);

int main(int argc, char *argv[])
{
    // --- MODO PADRE ---
    if (argc <= 3) // Si tiene pocos argumentos, es el padre
    {
        int NUM_HIJOS = (argc > 1) ? atoi(argv[1]) : 3;
        int MODALIDAD = (argc > 2) ? atoi(argv[2]) : 0;
        
        HANDLE hRead, hWrite;
        SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
        HANDLE hHijos[10]; // Arreglo para guardar los procesos (máximo 10)

        if (!CreatePipe(&hRead, &hWrite, &sa, 0)) return 1;
        SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

        for (int i = 0; i < NUM_HIJOS; i++) {
            STARTUPINFO si = { sizeof(si) };
            PROCESS_INFORMATION pi;
            char comando[512];

            // Pasamos: programa.exe quienSoy n_ini n_fin IMPRIME handle_pipe
            sprintf(comando, "%s %d %d %d %d %p", 
                    argv[0], i, 5*(i+1), 5*(i+2), 0, hWrite);

            CreateProcess(NULL, comando, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
            hHijos[i] = pi.hProcess;
            CloseHandle(pi.hThread);

            // MODALIDAD 0: Esperar a cada hijo inmediatamente
            if (MODALIDAD == 0) {
                WaitForSingleObject(pi.hProcess, INFINITE);
                char recibe_sms[256];
                DWORD leidos;
                if (ReadFile(hRead, recibe_sms, sizeof(recibe_sms), &leidos, NULL))
                    printf("%s\n", recibe_sms); // El padre despliega la cadena
            }
        }

        // MODALIDAD 1: Esperarlos a todos al final
        if (MODALIDAD == 1) {
            printf("Esperando por mis hijos (Al final)...\n");
            WaitForMultipleObjects(NUM_HIJOS, hHijos, TRUE, INFINITE);
            for (int i = 0; i < NUM_HIJOS; i++) {
                char recibe_sms[256];
                DWORD leidos;
                if (ReadFile(hRead, recibe_sms, sizeof(recibe_sms), &leidos, NULL))
                    printf("%s\n", recibe_sms);
                CloseHandle(hHijos[i]);
            }
        }

        CloseHandle(hRead);
        CloseHandle(hWrite);
    }
    // --- MODO HIJO ---
    else {
        int quienSoy = atoi(argv[1]);
        int n_ini = atoi(argv[2]);
        int n_fin = atoi(argv[3]);
        int IMPRIME = atoi(argv[4]);
        HANDLE hWriteHijo = (HANDLE)strtoll(argv[5], NULL, 16);

        long num_ini = dos_a_la_n(n_ini) - 1L;
        long num_fin = dos_a_la_n(n_fin) - 1L;
        char cad_res[100];
        char final_sms[256];
        DWORD escritos;

        cantidad_de_primos(cad_res, IMPRIME, num_ini, num_fin);
        sprintf(final_sms, "quienSoy:%d, %s", quienSoy, cad_res);

        WriteFile(hWriteHijo, final_sms, (DWORD)strlen(final_sms) + 1, &escritos, NULL);
        exit(0);
    }
    return 0;
}