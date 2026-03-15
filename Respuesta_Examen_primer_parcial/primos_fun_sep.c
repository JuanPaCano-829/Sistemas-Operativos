/* ===================================================
   primos versión función.
   
   ================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "primos_fun.h"

// ------------------------------------------------------------------------
//
// ------------------------------------------------------------------------
int esprimo(long n)
{
   long r = 1L;
   long div = 3L;
   while( (r = n % div ) && ( div * div <= n ) )
      div += 2L;
      
   return r;   
}
// ------------------------------------------------------------------------
//
// ------------------------------------------------------------------------
long dos_a_la_pot2(int pot2)
{
	long z  = 2L;
	int pot = 1;
	while(pot < pot2)
	{
		pot *= 2;
		z   *= z;
	}
	return z;
}
// ------------------------------------------------------------------------
//
// ------------------------------------------------------------------------
long dos_a_la_n(int n)
{
	long p = 2L;
	long s = 1L;
	while (n)
	{
		if( n % 2) s *=  p;
		p *= p;
	    n /= 2L;
	}
	return s;
}
// ------------------------------------------------------------------------
//     cantidad_de_primos 
// ------------------------------------------------------------------------
void cantidad_de_primos(char* cad_res, int IMPRIME, long num_inicial, long num_final)
{
   //       cad_res ... arreglo de caracters previamente alojado para la respuesta
   //       IMPRIME ... == 0 no imprime; != 0 imprime los primos encontrados
   //       num_ini ... inicio del rango de números de búsqueda
   //       num_fin ... final  del rango de números de búsqueda
   // NOTA: en cad_res entrega una cadena de caracteres como 
   //       "primos entre(15,255):48 en 0.000330 segs" 

  clock_t start, end;
  double cpu_time_used;
  int cuantos = 0;
  long k;
 
  start = clock();
  for( k = num_inicial; k <= num_final; k += 2L)
  { 
       if( esprimo(k) )
	   {  
          cuantos++;
          if(IMPRIME) printf("%ld es primo, cuantos: %d\n",k,cuantos);
	   }
  }
  
  end = clock();
   
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

  sprintf(cad_res,"primos entre(%ld,%ld):%d en %f segs\n",num_inicial,num_final,cuantos, cpu_time_used);
  
  return; 
}

