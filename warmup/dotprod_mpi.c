/*
This is an elementary program to illustrate the use of 
MPI in a program obtained by modifying a sequential
code that performs a dot product. In this code, we
have chosen to use the SPMD model and for convenience
have replicated the main data on all nodes.
*/ 

#include <mpi.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/time.h>



/*
The following structure contains the necessary information 
to allow the function "dotprod" to access its input data and 
place its output into the structure. Note that this structure 
is unchanged from the sequential version.
*/ 

typedef struct
 {
   double      *a;
   double      *b;
   double     sum;
   int     veclen;
 } DOTDATA;

/*Define globally accessible variables */ 

#define VECLEN 10000000
   DOTDATA dotstr;

double wallTime() // returns time in MS as a double
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

/*
The function dotprod is very similar to the sequential version 
except that we now have each node working on a different part 
of the data. As before, all access to the input is through 
a structure of type DOTDATA and all output from this function
is written into this same structure.
*/ 

void* dotprod()
{

/*Define and use local variables for convenience */ 

   int i, start, end, myid, len;
   double mysum, *x, *y;

/*Obtain rank of this node */ 

   MPI_Comm_rank (MPI_COMM_WORLD, &myid);

   len = dotstr.veclen;
   start = myid*len;
   end   = start + len;
   x = dotstr.a;
   y = dotstr.b;

/*
Perform the dot product and assign result
to the appropriate variable in the structure. 
*/ 

   mysum = 0;
   for (i=start; i<end ; i++)
    {
      mysum += (x[i] * y[i]);
    }

   dotstr.sum += mysum;

}

/*
As before, the main program does very little computation. It does 
however make all the calls to the MPI routines. This is not a 
master-slave arrangement and all nodes participate equally in the
work.
*/ 

main (int argc, char* argv[])
{
   int i,len=VECLEN;
   int myid, numprocs;
   double *a, *b;
   double mysum, allsum;
   double startTime, endTime;

/*MPI Initialization */ 

   MPI_Init (&argc, &argv);
   MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
   MPI_Comm_rank (MPI_COMM_WORLD, &myid);


/*Assign storage and initialize values */ 

   a = (double*) malloc (numprocs*len*sizeof(double));
   b = (double*) malloc (numprocs*len*sizeof(double));

   for (i=0; i<len*numprocs; i++)
    {
     a[i]=1;
     b[i]=a[i];
    }

   dotstr.veclen = len;
   dotstr.a = a;
   dotstr.b = b;
   dotstr.sum=0;

/* record start time, data is all ready */
   if (myid == 0)
	startTime = wallTime();

/*Call the dot product routine */ 
   dotprod();
   mysum = dotstr.sum;
   printf ("Node %d, mysum  =  %f \n", myid, mysum);
   system("/bin/hostname");
   printf ("Node %d \n", myid);

/*After the dot product, perform a summation of results on each node
*/ 

   MPI_Reduce (&mysum, &allsum, 1, MPI_DOUBLE, MPI_SUM, 0,
MPI_COMM_WORLD);
   if (myid == 0)
   {
      printf ("Allsum  =  %f \n", allsum);
      endTime = wallTime();
      printf ("Runtime  =  %f msec\n", endTime-startTime);
   }
   free (a);
   free (b);
   /*free (dotstr); martyon*/
   MPI_Finalize();
}

