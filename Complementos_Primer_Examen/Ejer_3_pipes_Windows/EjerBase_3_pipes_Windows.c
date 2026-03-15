#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

long dos_a_la_n(int n);
void cantidad_de_primos(char* cad_res, int IMPRIME, long num_inicial, long num_final);

int main(int argc, char *argv[]) {
    // Si no tiene argumentos extras, es el PADRE
    if (argc < 2) { 
        int NUM_HIJOS = 3; 
        HANDLE hHijos[10];

        printf("--- CODIGO BASE WINDOWS (Evidencia 1.1) ---\n");
        printf("Computadora: JuanPa\n");

        for (int i = 0; i < NUM_HIJOS; i++) {
            STARTUPINFO si = { sizeof(si) };
            PROCESS_INFORMATION pi;
            char cmd[512];
            
            // El hijo solo recibe su ID
            sprintf(cmd, "%s %d", argv[0], i); 
            
            if (CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                hHijos[i] = pi.hProcess;
                CloseHandle(pi.hThread);
            }
        }

        // El padre simplemente espera a que todos terminen
        WaitForMultipleObjects(3, hHijos, TRUE, INFINITE);
        for(int i=0; i<3; i++) CloseHandle(hHijos[i]);
        
        printf("Padre: Todos los hijos terminaron.\n");

    } else { 
        // SECCIÓN DEL HIJO: Solo calcula e imprime él mismo
        int id = atoi(argv[1]);
        char res[100];
        
        cantidad_de_primos(res, 0, dos_a_la_n(5*(id+1))-1, dos_a_la_n(5*(id+2))-1);
        
        // En el base, el HIJO imprime directamente, no usa pipes
        printf("hijo:%d, ./base_win --- %s\n", id, res);
        exit(0);
    }
    return 0;
}