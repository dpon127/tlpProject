#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <omp.h>
#include <mpi.h>
#include <openssl/sha.h>

#define LENGTH 6
#define NUM_CHARS 26

typedef unsigned char byte;

typedef struct 
{
   int num_found;
   int max_found;
} PASS;

PASS pass;

double wallTime() // returns time in MS as a double
{
   struct timeval tv;
   gettimeofday(&tv, 0);
   return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

int Match(byte *a, byte* b) {
   for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
      if (a[i] != b[i])
         return 0;
   return 1;
}


byte* StringHashToByteArray(const char* s) {
   byte* hash = (byte*) malloc(32);
   char two[3];
   two[2] = 0;
   for (int i = 0; i < 32; i++) {
      two[0] = s[i * 2];
      two[1] = s[i * 2 + 1];
      hash[i] = (byte)strtol(two, 0, 16);
   }
   return hash;
}


byte AlphaToChar(byte b) {
   return 97 + b;
}

// 6-character alphabetic password
void BruteForcePassword(byte* hash) {
   int myid; 
   double section, sec_start;
   MPI_Comm_rank (MPI_COMM_WORLD, &myid);
   section = (myid + 1) * NUM_CHARS / 4;
   sec_start = myid * NUM_CHARS / 4;
   
   for (byte b0 = sec_start; b0 < section; b0++) {
      for (byte b1 = 0; b1 < NUM_CHARS; b1++)
         for (byte b2 = 0; b2 < NUM_CHARS; b2++)
            for (byte b3 = 0; b3 < NUM_CHARS; b3++)
               for (byte b4 = 0; b4 < NUM_CHARS; b4++) 
                  for (byte b5 = 0; b5 < NUM_CHARS; b5++) { 

                     // extra NULL character at the end to make it a string
                     byte test_string[LENGTH + 1] = {AlphaToChar(b0), AlphaToChar(b1), AlphaToChar(b2), AlphaToChar(b3), AlphaToChar(b4), AlphaToChar(b5), 0};
                     
                     byte test_hash[SHA256_DIGEST_LENGTH];
                     SHA256(test_string, LENGTH, test_hash);

                     if (Match(hash, test_hash)) {
                        printf("%s\n", test_string);
                        pass.num_found = 1;  
			//MPI_Reduce(&(pass.num_found), &(pass.max_found), 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
                        return;
                     }

		     if (pass.max_found == 1){
			pass.num_found = 0;
			return;
		     }
                  }
   }
}


int main(int argc, char **argv)
{
   int num_hashes = 0;
   int hashes_read = 0;
   char hash_string[100];
   int numprocs, myid;
   double start, end;

   MPI_Init (&argc, &argv);
   MPI_Comm_size (MPI_COMM_WORLD, &numprocs);
   MPI_Comm_rank (MPI_COMM_WORLD, &myid);

   FILE* file = fopen(argv[1], "r");


   fscanf(file, "%d", &num_hashes);
   pass.num_found = 0;
   pass.max_found = 0;

   if(myid == 0)
     start = wallTime();  
//   struct timeval  tv1, tv2;

//   gettimeofday(&tv1, NULL);

   while (hashes_read < num_hashes) {
      fscanf(file, "%s", hash_string);
      hashes_read++;

      byte* hash = StringHashToByteArray(hash_string);

      BruteForcePassword(hash);
   }   

//   gettimeofday(&tv2, NULL);
//   double tElapsed = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
//      (double) (tv2.tv_sec - tv1.tv_sec);
   
//   fprintf(stderr, "Total time: %.3f seconds\n", tElapsed);
   
   if(myid == 0){
     end = wallTime();
     printf("time = %fms\n", end - start);  
   }

   fclose(file);
   MPI_Finalize(); 
   return 0;
}
