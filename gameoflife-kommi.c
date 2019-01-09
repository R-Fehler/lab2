#include <omp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
// OPTIONAL: comment this out for console output
//#define CONSOLE_OUTPUT

#define calcIndex(width, x, y) ((y) * (width) + (x))
#define ALIVE 1
#define DEAD 0

#define X 0
#define Y 1
void myexit(const char *s, ...) {
  va_list args;
  va_start(args, s);
  vprintf(s, args);
  printf("\n");
  va_end(args);
  abort();
}

char vtk_header[2048];
void create_vtk_header(char *header, int width, int height, int timestep) {
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
  strcat(header, "ORIGIN 0 0 0\n");
  snprintf(buffer, sizeof(buffer), "POINT_DATA %ld\n", width * height);
  strcat(header, buffer);
  strcat(header, "SCALARS data char 1\n");
  strcat(header, "LOOKUP_TABLE default\n");
}

void write_vtk_data(FILE *f, char *data, int length) {
  if (fwrite(data, sizeof(char), length, f) != length) {
    myexit("Could not write vtk-Data");
  }
}

void write_field(char *currentfield, int width, int height, int timestep) {
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
  printf("writing timestep %d\n", timestep);
  FILE *fp;  // The current file handle.
  char filename[1024];
  snprintf(filename, 1024, "./gol/gol-%05d.vtk", timestep);
  fp = fopen(filename, "w");
  write_vtk_data(fp, vtk_header, strlen(vtk_header));
  write_vtk_data(fp, currentfield, width * height);
  fclose(fp);
  printf("finished writing timestep %d\n", timestep);
#endif
}

void evolve(char *currentfield, char *newfield, int starts[2], int ends[2],
            int width) {
  // void evolve (char* currentfield, char* newfield, int width, int height) {
  // TODO traverse through each voxel and implement game of live logic and

  //#pragma omp parallel for
  for (int y = starts[Y]; y < ends[Y]; y++) {
    int yi = y * width;
    char *ci = currentfield + yi;
    char *ni = newfield + yi;

    //#pragma omp parallel for num_threads(8)
    for (int x = starts[X]; x < ends[X]; x++) {
      // printf("%d;%d;%d\n" , omp_get_thread_num(),x, y);
      int neighbours = 0;
      //#pragma omp parallel shared(ci,x,width) reduction(+:neighbours)
      {
        for (int ni = 0; ni < 9; ni++) {
          if (ni == 4) {
            continue;
          }
          int ny = (ni / 3) - 1;
          int nx = (ni % 3) - 1;

          char *nyi = ci + x + (ny * width);
          char neighbourCell = *(nyi + nx);
          neighbours += neighbourCell;
        }
      }

      //			for (int ny = -1; ny <= 1; ny++) {
      //				char *nyi = ci+x+ (ny * width);
      //				for (int nx = - 1; nx <= 1; nx++) {
      //					if (nx == 0 && ny == 0) {
      //						continue;
      //					}
      //					char neighbourCell = *(nyi+nx);
      //					neighbours = neighbourCell;
      //				}
      //			}

      char cell = ci[x];
      switch (cell) {
        case ALIVE:
          if (neighbours < 2) {
            ni[x] = DEAD;
          } else if (neighbours > 3) {
            ni[x] = DEAD;
          } else {
            ni[x] = ALIVE;
          }
          break;
        case DEAD:
          if (neighbours == 3) {
            ni[x] = ALIVE;
          } else {
            ni[x] = DEAD;
          }
      }
    }
  }

  // parallelize using OpenMP.
  // HINT: use 'starts' and 'ends'
}

void filling_random(char *currentfield, int width, int height) {
  int i;
  for (int y = 1; y < height - 1; y++) {
    for (int x = 1; x < width - 1; x++) {
      i = calcIndex(width, x, y);
      currentfield[i] =
          (rand() < RAND_MAX / 10) ? 1 : 0;  ///< init domain randomly
    }
  }
}

void filling_runner(char *currentfield, int width, int height) {
  int offset_x = width / 3;
  int offset_y = height / 2;
  currentfield[calcIndex(width, offset_x + 0, offset_y + 1)] = ALIVE;
  currentfield[calcIndex(width, offset_x + 1, offset_y + 2)] = ALIVE;
  currentfield[calcIndex(width, offset_x + 2, offset_y + 0)] = ALIVE;
  currentfield[calcIndex(width, offset_x + 2, offset_y + 1)] = ALIVE;
  currentfield[calcIndex(width, offset_x + 2, offset_y + 2)] = ALIVE;
}

void apply_periodic_boundaries(char *field, int width, int height) {
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

void game(int width, int height, int num_timesteps) {
  char *currentfield = calloc(width * height, sizeof(char));
  char *newfield = calloc(width * height, sizeof(char));
  // TODO 1: use your favorite filling
  // filling_random (currentfield, width, height);
  filling_runner(currentfield, width, height);
  int starts[2];
  int ends[2];
  starts[X] = 1;
  starts[Y] = 1;
  ends[X] = width - 1;
  ends[Y] = height - 1;

  int s1[2];
  s1[X] = 1;
  s1[Y] = 1;
  int e1[2];
  e1[X] = (width / 2);
  e1[Y] = (height / 2);

  int s2[2];
  s2[X] = (width / 2);
  s2[Y] = 1;
  int e2[2];
  e2[X] = width - 1;
  e2[Y] = (height / 2);

  int s3[2];
  s3[X] = 1;
  s3[Y] = (height / 2);
  int e3[2];
  e3[X] = (width / 2);
  e3[Y] = height - 1;

  int s4[2];
  s4[X] = width / 2;
  s4[Y] = height / 2;
  int e4[2];
  e4[X] = width - 1;
  e4[Y] = height - 1;

  int time = 0;
  // write_field(currentfield, width, height, time);
  // TODO 3: implement periodic boundary condition
  apply_periodic_boundaries(currentfield, width, height);
  for (time = 1; time <= num_timesteps; time++) {
    // TODO 2: implement evolve function (see above)
    /*		#pragma omp parallel sections num_threads(4)
                    {
                            #pragma omp section
                            {
                            evolve(currentfield, newfield, s1, e1, width);
                            }
                            #pragma omp section
                            {
                            evolve(currentfield, newfield, s2, e2, width);
                            }
                            #pragma omp section
                            {
                            evolve(currentfield, newfield, s3, e3, width);
                            }
                            #pragma omp section
                            {
                            evolve(currentfield, newfield, s4, e4, width);
                            }
                    }*/
    evolve(currentfield, newfield, starts, ends, width);

    // TODO 3: implement periodic boundary condition
    apply_periodic_boundaries(newfield, width, height);
    // write_field(newfield, width, height, time);
    // TODO 4: implement SWAP of the fields
    char *temp = currentfield;
    currentfield = newfield;
    newfield = temp;
  }
  free(currentfield);
  free(newfield);
}

int main(int c, char **v) {
#pragma omp parallel
  {
    if (omp_get_thread_num() == 0) {
      printf("Running with %d threads\n", omp_get_num_threads());
    }
  }
  int sum;
#pragma omp parallel reduction(+ : sum)
  sum = omp_get_thread_num();

  printf("%d", sum);

  int width = 0, height = 0, num_timesteps;
  if (c == 4) {
    width = atoi(v[1]) + 2;  ///< read width + 2 boundary cells (low x, high x)
    height =
        atoi(v[2]) + 2;  ///< read height + 2 boundary cells (low y, high y)
    num_timesteps = atoi(v[3]);  ///< read timesteps
    if (width <= 0) {
      width = 32;  ///< default width
    }
    if (height <= 0) {
      height = 32;  ///< default height
    }
    game(width, height, num_timesteps);
  } else {
    myexit("Too less arguments");
  }
}
