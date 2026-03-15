/* ==================================================================================
   Examen Parcial - Ejercicio IV) Memoria Compartida Windows
   ================================================================================== */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUF_SIZE 1024 // Tamaño suficiente para 10 hijos x 100 bytes
TCHAR szName[] = TEXT("Local\\MemoriaPrimos_188653"); // Nombre del objeto de memoria

// Prototipos de funciones
long dos_a_la_n(int n);
void cantidad_de_primos(char* cad_res, int IMPRIME, long num_inicial, long num_final);

int main(int argc, char *argv[])
{
    // --- MODO PADRE ---
    if (argc <= 1) 
    {
        int NUM_HIJOS = 3; 
        int MODALIDAD = 1; // Por default los esperamos al final para ver la memoria llena
        HANDLE hMapFile;
        char* pBuf;
        HANDLE hHijos[10];

        printf("--- MODO PADRE (Windows Shared Memory) ---\n");

        // 1. Crear el objeto de memoria compartida (File Mapping)
        hMapFile = CreateFileMapping(
                     INVALID_HANDLE_VALUE,    // Usa el archivo de paginación
                     NULL,                    // Seguridad por default
                     PAGE_READWRITE,          // Acceso lectura/escritura
                     0,                       // Tamaño máximo (High DWORD)
                     BUF_SIZE,                // Tamaño máximo (Low DWORD)
                     szName);                 // Nombre del objeto

        if (hMapFile == NULL) {
            printf("No se pudo crear la memoria compartida (%lu).\n", GetLastError());
            return 1;
        }

        // 2. Mapear la vista para que el padre pueda leer
        pBuf = (char*) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUF_SIZE);

        for (int i = 0; i < NUM_HIJOS; i++) {
            STARTUPINFO si = { sizeof(si) };
            PROCESS_INFORMATION pi;
            char comando[512];

            // Pasamos: programa.exe quienSoy n_ini n_fin IMPRIME
            sprintf(comando, "%s %d %d %d %d", argv[0], i, 5*(i+1), 5*(i+2), 0);

            if (CreateProcess(NULL, comando, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                hHijos[i] = pi.hProcess;
                CloseHandle(pi.hThread);
            }

            if (MODALIDAD == 0) {
                WaitForSingleObject(pi.hProcess, INFINITE);
                // Leemos del offset correspondiente: pBuf + (100 * i)
                printf("Padre lee (Hijo %d): %s\n", i, pBuf + (100 * i));
            }
        }

        if (MODALIDAD == 1) {
            printf("Esperando a todos los hijos...\n");
            WaitForMultipleObjects(NUM_HIJOS, hHijos, TRUE, INFINITE);
            for (int i = 0; i < NUM_HIJOS; i++) {
                // El padre recorre la memoria saltando de 100 en 100 bytes
                printf("Lectura Memoria Offset %d: %s\n", i * 100, pBuf + (i * 100));
                CloseHandle(hHijos[i]);
            }
        }

        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
    }
    // --- MODO HIJO ---
    else 
    {
        int quienSoy = atoi(argv[1]);
        int n_ini = atoi(argv[2]);
        int n_fin = atoi(argv[3]);
        int IMPRIME = atoi(argv[4]);
        
        HANDLE hMapFile;
        char* pBuf;

        // 3. El hijo ABRE el mapeo de memoria que ya creó el padre
        hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, szName);
        if (hMapFile == NULL) exit(1);

        pBuf = (char*) MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUF_SIZE);

        char cad_res[100];
        char final_sms[256];
        cantidad_de_primos(cad_res, IMPRIME, dos_a_la_n(n_ini)-1, dos_a_la_n(n_fin)-1);
        
        sprintf(final_sms, "quienSoy:%d, %s", quienSoy, cad_res);

        // 4. ESCRITURA CON OFFSET (Instrucción 127 del examen)
        // Escribimos en: apuntador_base + (100 * quienSoy)
        char* destino = pBuf + (100 * quienSoy);
        CopyMemory(destino, final_sms, strlen(final_sms) + 1);

        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
        exit(0);
    }
    return 0;
}