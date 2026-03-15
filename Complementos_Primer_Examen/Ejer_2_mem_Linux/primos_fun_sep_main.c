#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "primos_fun.h"

// ------------------------------------------------------------------------
//                            main 
// ------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  clock_t start, end;
  double cpu_time_used;
  int cuantos = 0;
	
  if(argc == 1)
  {
      printf("-----------------------------------------------------------------------------------\n");	  
	  printf("obtiene los primos en un rango\nuso:\n");
	  printf("%s IMPRIME (0=>NoImprime) n_ini n_fin (para rango de 2**(n_ini) - 1 a 2**(n_fin)-1\n",argv[0]);
      printf("-----------------------------------------------------------------------------------n");	  
  }	  
	
  int IMPRIME      =   argc <= 1 ? 1 : atoi(argv[1]);
  
  long num_inicial = argc <= 2 ?  15 : dos_a_la_n(atoi(argv[2])) -1L;  
  long num_final   = argc <= 3 ? 255 : dos_a_la_n(atoi(argv[3])) -1L;   
  
  long k;
  
  printf("obteniendo los primos entre %ld y %ld\n",num_inicial,num_final);

  char cad_res[100];  // primos entre(15,255):48 en 0.000330 segs

  cantidad_de_primos(cad_res,IMPRIME,num_inicial,num_final);
  
  printf("%s --- %s\n",argv[0],cad_res);

  return 0;
  
}

