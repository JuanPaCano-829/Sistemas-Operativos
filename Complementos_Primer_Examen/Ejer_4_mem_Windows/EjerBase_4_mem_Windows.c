#include <windows.h>
#include <stdio.h>

long dos_a_la_n(int n);
void cantidad_de_primos(char* cad_res, int IMPRIME, long num_inicial, long num_final);

int main(int argc, char *argv[]) {
    TCHAR szName[] = TEXT("Local\\MemPrimos188653");
    if (argc <= 1) { // PADRE
        int NUM = 3; 
        HANDLE hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1024, szName);
        char* pBuf = (char*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 1024);
        HANDLE hH[10];

        for (int i = 0; i < NUM; i++) {
            STARTUPINFO si = {sizeof(si)}; PROCESS_INFORMATION pi;
            char cmd[512]; sprintf(cmd, "%s %d", argv[0], i);
            CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
            hH[i] = pi.hProcess;
            if (i == 0) { // Ejemplo Modalidad 0 simplificada
                WaitForSingleObject(pi.hProcess, INFINITE);
                printf("%s\n", pBuf + (i*100));
            }
        }
        // ... (Lógica de espera similar al de Pipes Windows)
        UnmapViewOfFile(pBuf); CloseHandle(hMap);
    } else { // HIJO
        int id = atoi(argv[1]);
        HANDLE hM = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, szName);
        char* p = (char*)MapViewOfFile(hM, FILE_MAP_ALL_ACCESS, 0, 0, 1024);
        char res[100], sms[256];
        cantidad_de_primos(res, 0, dos_a_la_n(5*(id+1))-1, dos_a_la_n(5*(id+2))-1);
        sprintf(sms, "quienSoy:%d, %s", id, res);
        CopyMemory(p + (id*100), sms, strlen(sms)+1);
        UnmapViewOfFile(p); CloseHandle(hM);
        exit(0);
    }
    return 0;
}