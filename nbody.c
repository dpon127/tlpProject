#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

typedef struct { float x, y, z, vx, vy, vz; } Body;

void randomizeBodies(float *data, int n) {
  srand(0);

  for (int i = 0; i < n; i++) {
    data[i] = 2.0f * (rand() / (float)RAND_MAX) - 1.0f;
  }
}

void nBody(Body *p, float dt, int n, int nIters) {  
  for (int iter = 0; iter < nIters; iter++) {
    for (int i = 0; i < n; i++) { 
      float Fx = 0.0f; float Fy = 0.0f; float Fz = 0.0f;
  
      for (int j = 0; j < n; j++) {
        float dx = p[j].x - p[i].x;
        float dy = p[j].y - p[i].y;
        float dz = p[j].z - p[i].z;
        float distSqr = dx*dx + dy*dy + dz*dz + 1e-9f;
        float invDist = 1.0f / sqrt(distSqr);
        float invDist3 = invDist * invDist * invDist;
  
        Fx += dx * invDist3; Fy += dy * invDist3; Fz += dz * invDist3;
      }
  
      p[i].vx += dt*Fx; p[i].vy += dt*Fy; p[i].vz += dt*Fz;
    }
  
    for (int i = 0 ; i < n; i++) { // integrate position
      p[i].x += p[i].vx*dt;
      p[i].y += p[i].vy*dt;
      p[i].z += p[i].vz*dt;
    }
  }
}

void printBodies(Body *p, int n) {
  for (int i = 0; i < n; i++) { 
    printf("%d %f %f %f\n", i, p[i].x, p[i].y, p[i].z);
  }
}

int main(const int argc, const char** argv) {
  
  int nBodies = 30000;
  if (argc > 1) nBodies = atoi(argv[1]);

  const float dt = 0.01f; // time step
  const int nIters = 10;  // simulation iterations

  int bytes = nBodies*sizeof(Body);
  float *buf = (float*)malloc(bytes);
  Body *p = (Body*)buf;

  randomizeBodies(buf, 6*nBodies); // Init pos / vel data

  double totalTime = 0.0;
  struct timeval  tv1, tv2;
  
  gettimeofday(&tv1, NULL);

  nBody(p, dt, nBodies, nIters); // compute interbody forces

  gettimeofday(&tv2, NULL);
  double tElapsed = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 +
       (double) (tv2.tv_sec - tv1.tv_sec);
  
  fprintf(stderr, "Total time: %.3f seconds\n", tElapsed);
  
  //printBodies(p, nBodies);
  
  free(buf);
}
