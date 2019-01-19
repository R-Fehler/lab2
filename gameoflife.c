#include <omp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
// Programm ARGS:  num_threads_in_x, num_threads_in_y,
// arraysize_per_thread_x, arraysize_per_thread_y,num_timesteps
//
//
// OPTIONAL: comment this out for console output
//#define CONSOLE_OUTPUT

#define calcIndex(width, x, y) ((y) * (width) + (x))
#define ALIVE 1
#define DEAD 0

#define X 0
#define Y 1
void myexit(const char* s, ...) {
  va_list args;
  va_start(args, s);
  vprintf(s, args);
  printf("\n");
  va_end(args);
  abort();
}

char vtk_header[2048];
void create_vtk_header(char* header, int width, int height, int timestep) {
  char buffer[1024];
  header[0] = '\0';
  strcat(header, "# vtk DataFile Version 3.0\n");
  snprintf(buffer, sizeof(buffer), "Gameoflife timestep %d \n", timestep);
  strcat(header, buffer);
  strcat(header, "BINARY\n");
  strcat(header, "DATASET STRUCTURED_POINTS\n");
  snprintf(buffer, sizeof(buffer), "DIMENSIONS %d %d 1\n", width, height);
  strcat(header, buffer);
  strcat(header, "SPACING 1.0 1.0 1.0\n");
  strcat(header, "ORIGIN 0 0 0\n");  // multithread gebietsursprung
  snprintf(buffer, sizeof(buffer), "POINT_DATA %ld\n", width * height);
  strcat(header, buffer);
  strcat(header, "SCALARS data char 1\n");
  strcat(header, "LOOKUP_TABLE default\n");
}

void write_vtk_data(FILE* f, char* data, int length) {
  if (fwrite(data, sizeof(char), length, f) != length) {
    myexit("Could not write vtk-Data");
  }
}

void write_field(char* currentfield, int width, int height, int timestep) {
#ifdef CONSOLE_OUTPUT
  printf("\033[H");
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++)
      printf(ALIVE == currentfield[calcIndex(width, x, y)] ? "\033[07m  \033[m"
                                                           : "  ");
    printf("\033[E");
  }
  fflush(stdout);
  printf("\ntimestep=%d", timestep);
  usleep(80000);
#else
  if (timestep == 0) {
    mkdir("./gol/", 0777);
    create_vtk_header(vtk_header, width, height, timestep);
  }
  // printf("writing timestep %d\n", timestep);
  FILE* fp;  // The current file handle.
  char filename[1024];
  snprintf(filename, 1024, "./gol/gol-%05d.vtk", timestep);
  fp = fopen(filename, "w");
  write_vtk_data(fp, vtk_header, strlen(vtk_header));
  write_vtk_data(fp, currentfield, width * height);
  fclose(fp);
  // printf("finished writing timestep %d\n", timestep);
#endif
}

void evolve(char* currentfield, char* newfield, int thread_id, int num_thr_x,
            int num_thr_y, int arrsize_X_p_thr, int arrsize_Y_p_thr,
            int width) {
  // void evolve (char* currentfield, char* newfield, int width, int height) {
  // TODO traverse through each voxel and implement game of live logic and
  // parallelize using OpenMP.
  // HINT: use 'starts' and 'ends'
  // void evolve(char* currentfield, char* newfield, int width, int height) {
  // TODO traverse through each voxel and implement game of live logic

  // segmentation in subarrays die ~ zu NUMBERTHREADS sind
  //#pragma omp for collapse(2)
  //
  // calcIndex of 2D Segment
  int block_index[2];  // index des teilfelds per thread
  int start[2];        // start koords des blocks
  int end[2];          // end koords des blocks
  block_index[X] = (thread_id % num_thr_x);
  block_index[Y] = (thread_id / num_thr_x);
  start[X] = block_index[X] * arrsize_X_p_thr;
  start[Y] = block_index[Y] * arrsize_Y_p_thr;
  end[X] = (block_index[X] + 1) * arrsize_X_p_thr;
  end[Y] = (block_index[Y] + 1) * arrsize_Y_p_thr;

  int summe_der_Nachbarn;
  // start -1 --> bus error bei feld >>120000
  // start +1 --> kein bus error
  if (block_index[X] == 0) {
    start[X]++;
  }
  if (block_index[Y] == 0) {
    start[Y]++;
  }
  if (block_index[X] == (num_thr_x - 1)) {
    end[X]--;
  }
  if (block_index[Y] == (num_thr_y - 1)) {
    end[Y]--;
  }
  for (int y = start[Y]; y <= end[Y] + 1; y++) {
    for (int x = start[X]; x <= end[X] + 1; x++) {
      summe_der_Nachbarn = 0;
      long cell_index = calcIndex(width, x, y);
      // Durchlaufen der 9 Felder des aktuellen "Stempels"
      for (int x1 = -1; x1 <= 1; x1++) {
        for (int y1 = -1; y1 <= 1; y1++) {
          summe_der_Nachbarn +=
              currentfield[calcIndex(width, (x + x1), (y + y1))];
        }
      }
      // Wert der untersuchten Zelle von der Summe der Felder abziehen

      if (currentfield[cell_index]) {
        summe_der_Nachbarn--;
      }

      if (summe_der_Nachbarn <= 1) {
        newfield[cell_index] = DEAD;
        // cellindex > als feldgrösse was ist da los?
      }

      else if ((summe_der_Nachbarn == 2 && currentfield[cell_index]) == ALIVE ||
               summe_der_Nachbarn == 3) {
        newfield[cell_index] = ALIVE;
      }

      else if (summe_der_Nachbarn >= 4) {
        newfield[cell_index] = DEAD;
      } else {
        newfield[cell_index] = DEAD;
      }
      // HINT: avoid boundaries
    }
  }
}

void swap_field(char** currentfield, char** newfield) {
  char* temp = *currentfield;
  *currentfield = *newfield;
  *newfield = temp;
}

void filling_random(char* currentfield, int width, int height) {
  int i;
  for (int y = 1; y < height - 1; y++) {
    for (int x = 1; x < width - 1; x++) {
      i = calcIndex(width, x, y);
      currentfield[i] =
          (rand() < RAND_MAX / 10) ? 1 : 0;  ///< init domain randomly
    }
  }
}

void filling_runner(char* currentfield, int width, int height) {
  currentfield[calcIndex(width, width / 2 + 0, height / 2 + 1)] = ALIVE;
  currentfield[calcIndex(width, width / 2 + 1, height / 2 + 2)] = ALIVE;
  currentfield[calcIndex(width, width / 2 + 2, height / 2 + 0)] = ALIVE;
  currentfield[calcIndex(width, width / 2 + 2, height / 2 + 1)] = ALIVE;
  currentfield[calcIndex(width, width / 2 + 2, height / 2 + 2)] = ALIVE;
}

void apply_periodic_boundaries(char* field, int width, int height) {
  // TODO: implement periodic boundary copies
  int neFieldCorner = calcIndex(width, width - 2, 1);
  int nwFieldCorner = calcIndex(width, 1, 1);
  int seFieldCorner = calcIndex(width, width - 2, height - 2);
  int swFieldCorner = calcIndex(width, 1, height - 2);

  int neBoundCorner = calcIndex(width, width - 1, 0);
  int nwBoundCorner = calcIndex(width, 0, 0);
  int seBoundCorner = calcIndex(width, width - 1, height - 1);
  int swBoundCorner = calcIndex(width, 0, height - 1);

  for (int xi = 1; xi < width - 1; xi++) {
    int lowerBoundIndex = calcIndex(width, xi, height - 1);
    int upperFieldIndex = calcIndex(width, xi, 1);
    field[lowerBoundIndex] = field[upperFieldIndex];

    int upperBoundIndex = calcIndex(width, xi, 0);
    int lowerFieldIndex = calcIndex(width, xi, height - 2);
    field[upperBoundIndex] = field[lowerFieldIndex];
  }
  for (int yi = 1; yi < height - 1; yi++) {
    int leftBoundIndex = calcIndex(width, 0, yi);
    int rightFieldIndex = calcIndex(width, width - 2, yi);
    field[leftBoundIndex] = field[rightFieldIndex];

    int rightBoundIndex = calcIndex(width, width - 1, yi);
    int leftFieldIndex = calcIndex(width, 1, yi);
    field[rightBoundIndex] = field[leftFieldIndex];
  }
  field[nwBoundCorner] = field[seFieldCorner];
  field[neBoundCorner] = field[swFieldCorner];
  field[seBoundCorner] = field[nwFieldCorner];
  field[swBoundCorner] = field[neFieldCorner];
}

void game(int width, int height, int num_timesteps, int num_threads_in_x,
          int num_threads_in_y, int arraysize_per_thread_x,
          int arraysize_per_thread_y) {
  char* currentfield = calloc(width * height, sizeof(char));
  char* newfield = calloc(width * height, sizeof(char));
  // TODO 1: use your favorite filling
  filling_random(currentfield, width, height);
  // filling_runner(currentfield, width, height);
  // int starts[2];
  // int ends[2];
  // starts[X] = 1;
  // starts[Y] = 1;
  // ends[X] = width - 1;
  // ends[Y] = height - 1;

  int time = 0;
  write_field(currentfield, width, height, time);
  // TODO 3: implement periodic boundary condition
  apply_periodic_boundaries(currentfield, width, height);
#pragma omp parallel default(shared) firstprivate(time)
  {
    int thread_id = omp_get_thread_num();
    for (time = 1; time <= num_timesteps; time++) {
      // TODO 2: implement evolve function (see above)

      evolve(currentfield, newfield, thread_id, num_threads_in_x,
             num_threads_in_y, arraysize_per_thread_x, arraysize_per_thread_y,
             width);

      // TODO 3: implement periodic boundary condition
#pragma omp barrier
#pragma omp single
      {
        apply_periodic_boundaries(newfield, width, height);

        write_field(newfield, width, height, time);
        // TODO 4: implement SWAP of the fields
        swap_field(&currentfield, &newfield);
      }
    }
  }
  free(currentfield);
  free(newfield);
}

int main(int c, char** v) {
  int width = 0, height = 0, num_timesteps;
  int num_threads_in_x, num_threads_in_y;
  int arraysize_per_thread_x, arraysize_per_thread_y;
  if (c == 6) {
    num_threads_in_x = atoi(v[1]);
    num_threads_in_y = atoi(v[2]);
    arraysize_per_thread_x = atoi(v[3]);
    arraysize_per_thread_y = atoi(v[4]);
    num_timesteps = atoi(v[5]);  ///< read timesteps
    width = arraysize_per_thread_x * num_threads_in_x + 2;
    height = arraysize_per_thread_y * num_threads_in_y + 2;

    if (width <= 0) {
      width = 32;  ///< default width
    }
    if (height <= 0) {
      height = 32;  ///< default height
    }
    omp_set_num_threads((num_threads_in_x * num_threads_in_y));
#pragma omp parallel
    {  // start omp
      if (omp_get_thread_num() == 0) {
        printf("Running with %d threads\n", omp_get_num_threads());
      }
    }  // end omp
    printf("Spielfeldgroesse: %d x %d, = %d\n", width, height,
           (width * height));

    game(width, height, num_timesteps, num_threads_in_x, num_threads_in_y,
         arraysize_per_thread_x, arraysize_per_thread_y);
  } else {
    myexit("Too less arguments");
  }
}
