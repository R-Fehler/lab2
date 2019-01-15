# README: Segmentation in 2D Felder

start mit folgenden Argumenten:
 num_threads_in_x, num_threads_in_y, ARRAYSIZE_PER_THREAD_X, ARRAYSIZE_PER_THREAD_Y,num_timesteps

Ab ca 12 tausend Zellen Feldgröße tritt ein buserror auf. ?

CMAKE als Buildtool: [CMAKELISTS](./CMakeLists.txt)

gameoflife OpenMP: [gamoflife](./gameoflife.c)

gameoflife singlethreaded: [singlethreaded-gol](./gameoflife-old.c)

script zum Ausführen der Auswertung [Shellscript](./script-segm-omp.sh)

