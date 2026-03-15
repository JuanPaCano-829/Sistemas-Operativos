#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

long dos_a_la_n(int n);
void cantidad_de_primos(char* cad_res, int IMPRIME, long num_inicial, long num_final);

int main(int argc, char *argv[]) {
    if (argc <= 3) { // PADRE
        int NUM_HIJOS = argc > 1 ? atoi(argv[1]) : 3;
        int MODALIDAD = argc > 2 ? atoi(argv[2]) : 0;
        HANDLE hRead, hWrite, hHijos[10];
        SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };

        CreatePipe(&hRead, &hWrite, &sa, 0);
        SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

        for (int i = 0; i < NUM_HIJOS; i++) {
            STARTUPINFO si = { sizeof(si) };
            PROCESS_INFORMATION pi;
            char cmd[512];
            sprintf(cmd, "%s %d %p", argv[0], i, hWrite); // Pasamos ID y Handle
            CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
            hHijos[i] = pi.hProcess;
            if (MODALIDAD == 0) {
                WaitForSingleObject(pi.hProcess, INFINITE);
                char buf[256]; DWORD leidos;
                if (ReadFile(hRead, buf, 256, &leidos, NULL)) printf("%s\n", buf);
            }
        }
        if (MODALIDAD == 1) {
            WaitForMultipleObjects(NUM_HIJOS, hHijos, TRUE, INFINITE);
            for (int i = 0; i < NUM_HIJOS; i++) {
                char buf[256]; DWORD leidos;
                ReadFile(hRead, buf, 256, &leidos, NULL);
                printf("%s\n", buf);
            }
        }
    } else { // HIJO
        int id = atoi(argv[1]);
        HANDLE hW = (HANDLE)strtoll(argv[2], NULL, 16);
        char res[100], sms[256]; DWORD esc;
        cantidad_de_primos(res, 0, dos_a_la_n(5*(id+1))-1, dos_a_la_n(5*(id+2))-1);
        sprintf(sms, "quienSoy:%d, %s", id, res);
        WriteFile(hW, sms, strlen(sms)+1, &esc, NULL);
        exit(0);
    }
    return 0;
}