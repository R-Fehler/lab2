#!/usr/bin/env bash
if [[ "$OSTYPE" == "linux-gnu" ]]; then
        alias gtime=time
fi
mkdir -p log
LOOP=1
NTIMESTEPS=50
num_threads_in_x=(1 2 4) #bash Array
num_threads_in_y=(1 2 2)
ARRAYSIZE_PER_THREAD_X=100
ARRAYSIZE_PER_THREAD_Y=100
PATH_TO="./build/vorledit"
    echo "FeldNr. Zeit in sek CPU %">log/logtime 
    echo "LOG:" > log/logprogram

    for (( i = 0; i < $LOOP; i++ )); do
    for (( j = 0; j < 3; j++ )); do
    cnt=$(($j+1))
   (gtime -f "$cnt %e %P" $PATH_TO ${num_threads_in_x[$j]} ${num_threads_in_y[$j]} $ARRAYSIZE_PER_THREAD_X $ARRAYSIZE_PER_THREAD_Y $NTIMESTEPS) >>log/logprogram 2>>log/logtime
    done
    done
awk '$1=="1" {printf"%s, %s\n", $2, $3}' log/logtime > log/logtime1.csv
awk '$1=="2" {printf"%s, %s\n", $2, $3}' log/logtime > log/logtime2.csv
awk '$1=="3" {printf"%s, %s\n", $2, $3}' log/logtime > log/logtime3.csv


    
# das funktioniert so
# (time ./build/vorledit 1024 1024 20)>>logprogram 2>>logtime
# for (( i = 0; i < $LOOP; i++ )); do
#     print i >>logtime
# (time ./build/vorledit 2048 2048 20)>logprogram 2>logtime
# done
# for (( i = 0; i < $LOOP; i++ )); do
#     print i >>logtime
# (time ./build/vorledit 4096 4096 20)>logprogram 2>logtime
# done
