/* ==================================================================================
   Examen Parcial - Ejercicio III) Pipes Windows (API Nativa Win32)
   Mecanismo: Comunicación mediante tuberías con herencia de Handles.
   ================================================================================== */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Prototipos de las funciones matemáticas (viven en primos_fun_sep.c)
long dos_a_la_n(int n);
void cantidad_de_primos(char* cad_res, int IMPRIME, long num_inicial, long num_final);

int main(int argc, char *argv[]) {
    // Desactivamos el buffer para ver los mensajes en PowerShell de inmediato
    setvbuf(stdout, NULL, _IONBF, 0);

    /* Si el programa tiene menos de 3 argumentos, es el PADRE.
       Si tiene exactamente los argumentos que pasamos en CreateProcess, es el HIJO.
    */
    if (argc < 4) { 
        // --- SECCIÓN DEL PADRE ---
        int NUM_HIJOS = (argc > 1) ? atoi(argv[1]) : 3;
        int MODALIDAD = (argc > 2) ? atoi(argv[2]) : 0;
        
        HANDLE hRead, hWrite;
        HANDLE hHijos[10]; 
        SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE }; // Permite heredar el Pipe

        // 1. Crear el Pipe
        if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
            printf("Error al crear el pipe.\n");
            return 1;
        }

        // El extremo de lectura NO debe ser heredado por el hijo
        SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

        printf("Computadora: JuanPa (Padre lanzando %d hijos)\n", NUM_HIJOS);

        for (int i = 0; i < NUM_HIJOS; i++) {
            STARTUPINFO si = { sizeof(si) };
            PROCESS_INFORMATION pi;
            char comando[512];
            
            // Pasamos los argumentos según la rúbrica: ID, n_ini, n_fin, IMPRIME, Handle
            sprintf(comando, "%s %d %d %d %d %p", 
                    argv[0], i, 5*(i+1), 5*(i+2), 0, hWrite);
            
            // 2. Lanzar el proceso hijo
            if (!CreateProcess(NULL, comando, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
                printf("Error al crear hijo %d\n", i);
                continue;
            }
            hHijos[i] = pi.hProcess;
            CloseHandle(pi.hThread); 

            // Modalidad 0: Espera secuencial
            if (MODALIDAD == 0) {
                WaitForSingleObject(pi.hProcess, INFINITE);
                char buf[256]; DWORD leidos;
                if (ReadFile(hRead, buf, 256, &leidos, NULL)) {
                    printf("%s\n", buf);
                }
            }
        }

        // --- CAMBIO CRÍTICO DE SINCRONIZACIÓN ---
        // El padre DEBE cerrar su copia de escritura. Si no, ReadFile se bloquea
        // esperando que alguien más (el padre) escriba en el tubo.
        CloseHandle(hWrite);

        // Modalidad 1: Esperar a todos al final
        if (MODALIDAD == 1) {
            printf("Esperando por mis hijos (Al final)...\n");
            // Sincronización: esperamos a que todos los procesos terminen sus cálculos
            WaitForMultipleObjects(NUM_HIJOS, hHijos, TRUE, INFINITE); 
            
            for (int i = 0; i < NUM_HIJOS; i++) {
                char recibe_sms[256];
                DWORD leidos;
                // Leemos los resultados que los hijos dejaron en el pipe
                if (ReadFile(hRead, recibe_sms, sizeof(recibe_sms), &leidos, NULL)) {
                    printf("%s\n", recibe_sms);
                }
                CloseHandle(hHijos[i]);
            }
        }
        CloseHandle(hRead);

    } else { 
        // --- SECCIÓN DEL HIJO ---
        int id = atoi(argv[1]);
        int n_ini = atoi(argv[2]);
        int n_fin = atoi(argv[3]);
        int IMPRIME = atoi(argv[4]);
        HANDLE hW = (HANDLE)strtoll(argv[5], NULL, 16); // Recuperar el Handle

        long num_ini = dos_a_la_n(n_ini) - 1L;
        long num_fin = dos_a_la_n(n_fin) - 1L;
        char cad_res[100], final_sms[256];
        DWORD escritos;

        // Cálculo de la cantidad de primos
        cantidad_de_primos(cad_res, IMPRIME, num_ini, num_fin);
        
        // Formatear mensaje según instrucción 25 y 26
        sprintf(final_sms, "quienSoy:%d, %s", id, cad_res);

        // Enviar respuesta al padre a través del pipe
        WriteFile(hW, final_sms, (DWORD)strlen(final_sms) + 1, &escritos, NULL);
        CloseHandle(hW); 
        exit(0);
    }
    return 0;
}