#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <omp.h>
//OPTIONAL: comment this out for console output
//#define CONSOLE_OUTPUT

#define calcIndex(width, x,y)  ((y)*(width) + (x))
#define ALIVE 1
#define DEAD 0

#define X 0
#define Y 1
void myexit (const char * s, ...) {
  va_list args;
  va_start(args, s);
  vprintf (s, args);
  printf ("\n");
  va_end(args);
  abort ();
}

char vtk_header[2048];
void create_vtk_header (char * header, int width, int height, int timestep) {
  char buffer[1024];
  header[0] = '\0';
  strcat (header, "# vtk DataFile Version 3.0\n");
  snprintf (buffer, sizeof(buffer), "Gameoflife timestep %d \n", timestep);
  strcat (header, buffer);
  strcat (header, "BINARY\n");
  strcat (header, "DATASET STRUCTURED_POINTS\n");
  snprintf (buffer, sizeof(buffer), "DIMENSIONS %d %d 1\n", width, height);
  strcat (header, buffer);
  strcat (header, "SPACING 1.0 1.0 1.0\n");
  strcat (header, "ORIGIN 0 0 0\n");
  snprintf (buffer, sizeof(buffer), "POINT_DATA %ld\n", width * height);
  strcat (header, buffer);
  strcat (header, "SCALARS data char 1\n");
  strcat (header, "LOOKUP_TABLE default\n");
}

void write_vtk_data (FILE * f, char * data, int length) {
  if (fwrite (data, sizeof(char), length, f) != length) {
    myexit ("Could not write vtk-Data");
  }
}

void write_field (char* currentfield, int width, int height, int timestep) {
#ifdef CONSOLE_OUTPUT
  printf("\033[H");
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) printf(ALIVE == currentfield[calcIndex(width, x,y)] ? "\033[07m  \033[m" : "  ");
    printf("\033[E");
  }
  fflush(stdout);
  printf("\ntimestep=%d",timestep);
  usleep(80000);
#else
  if (timestep == 0) {
    mkdir("./gol/", 0777);
    create_vtk_header (vtk_header, width, height, timestep);
  }
  printf ("writing timestep %d\n", timestep);
  FILE *fp;       // The current file handle.
  char filename[1024];
  snprintf (filename, 1024, "./gol/gol-%05d.vtk", timestep);
  fp = fopen (filename, "w");
  write_vtk_data (fp, vtk_header, strlen (vtk_header));
  write_vtk_data (fp, currentfield, width * height);
  fclose (fp);
  printf ("finished writing timestep %d\n", timestep);
#endif
}


void evolve (char* currentfield, char* newfield, int starts[2], int ends[2], int width) {
//void evolve (char* currentfield, char* newfield, int width, int height) {
// TODO traverse through each voxel and implement game of live logic and
// parallelize using OpenMP.
// HINT: use 'starts' and 'ends'
}

void filling_random (char * currentfield, int width, int height) {
  int i;
  for (int y = 1; y < height - 1; y++) {
    for (int x = 1; x < width - 1; x++) {
      i = calcIndex(width, x, y);
      currentfield[i] = (rand () < RAND_MAX / 10) ? 1 : 0; ///< init domain randomly
    }
  }
}

void filling_runner (char * currentfield, int width, int height) {
  currentfield[calcIndex(width, width/2+0, height/2+1)] = ALIVE;
  currentfield[calcIndex(width, width/2+1, height/2+2)] = ALIVE;
  currentfield[calcIndex(width, width/2+2, height/2+0)] = ALIVE;
  currentfield[calcIndex(width, width/2+2, height/2+1)] = ALIVE;
  currentfield[calcIndex(width, width/2+2, height/2+2)] = ALIVE;
}

void apply_periodic_boundaries(char * field, int width, int height){
  //TODO: implement periodic boundary copies
}

void game (int width, int height, int num_timesteps) {
  char *currentfield = calloc (width * height, sizeof(char));
  char *newfield = calloc (width * height, sizeof(char));
  // TODO 1: use your favorite filling
  //filling_random (currentfield, width, height);
  //filling_runner (currentfield, width, height);
  int starts[2];
  int ends[2];
  starts[X] = 1;
  starts[Y] = 1;
  ends[X] = width - 1;
  ends[Y] = height - 1;
  int time = 0;
  write_field (currentfield, width, height, time);
  //TODO 3: implement periodic boundary condition
  apply_periodic_boundaries(currentfield,width,height);
#pragma omp parallel default(shared) firstprivate(time)
  for (time = 1; time <= num_timesteps; time++) {
    //TODO 2: implement evolve function (see above)
    evolve (currentfield, newfield, starts, ends, width);
    //TODO 3: implement periodic boundary condition
    apply_periodic_boundaries(newfield,width,height);
    write_field (newfield, width, height, time);
    //TODO 4: implement SWAP of the fields
  }
  free (currentfield);
  free (newfield);
}

int main (int c, char **v) {
#pragma omp parallel
  {
    if(omp_get_thread_num() == 0){
      printf("Running with %d threads\n",omp_get_num_threads());
    }
  }
  int width = 0, height = 0, num_timesteps;
  if (c == 4) {
    width = atoi (v[1]) + 2; ///< read width + 2 boundary cells (low x, high x)
    height = atoi (v[2]) + 2; ///< read height + 2 boundary cells (low y, high y)
    num_timesteps = atoi (v[3]); ///< read timesteps
    if (width <= 0) {
      width = 32; ///< default width
    }
    if (height <= 0) {
      height = 32; ///< default height
    }
    game (width, height, num_timesteps);
  }
  else {
   myexit("Too less arguments");
  }
}
