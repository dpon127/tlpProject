#include <stdio.h>
#include <malloc.h>
#include <sys/time.h>
#include <omp.h>


/*Define globally accessible variables */ 

#define MAXTHRDS 8
#define VECLEN 100000000

double wallTime() // returns time in MS as a double
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}


main (int argc, char* argv[])
{
   int i;
   double *a, *b, result = 0.0;
   double start, end;

/*Assign storage and initialize values */ 

   a = (double*) malloc (VECLEN*sizeof(double));
   b = (double*) malloc (VECLEN*sizeof(double));

   for (i=0; i<VECLEN; i++) {
     a[i]=1;
     b[i]=a[i];
   }

   omp_set_dynamic(0);     // Explicitly disable dynamic teams
   omp_set_num_threads(MAXTHRDS); // Use 4 threads for all consecutive parallel regions

   start = wallTime();

#pragma omp parallel
{
   int tid = omp_get_thread_num();
   printf("I am thread %d\n", tid);
}

#pragma omp parallel for reduction(+:result)  
   for (i=0; i < VECLEN; i++)
     result += (a[i] * b[i]);

   /* track exec time */
   end = wallTime();
   printf ("Runtime = %f msecs \n", end - start);

   printf ("Sum =  %f \n", result);
   free (a);
   free (b);
}


