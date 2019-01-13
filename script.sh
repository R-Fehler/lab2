#!/usr/bin/env bash
LOOP=10
NTIMESTEPS=50
FELD1=128
FELD2=$(($FELD1*2))
FELD3=$(($FELD1*4))
PATH_TO="./build/vorledit"
    echo "FeldNr. Zeit in sek CPU %">logtime 
    echo "LOG:" > logprogram
    printf "Feldgroesse 1: %d\n" $FELD1
    printf "Feldgroesse 2: %d\n" $FELD2
    printf "Feldgroesse 3: %d\n" $FELD3

    for (( i = 0; i < $LOOP; i++ )); do
    # printf "Iteration_Nr. %d" $i >> logtime
   (gtime -f "1 %e %P" $PATH_TO $FELD1 $FELD1 $NTIMESTEPS)>>logprogram 2>>logtime
    # printf "Iteration_Nr. %d" $i >> logtime
   (gtime -f "2 %e %P " $PATH_TO $FELD2 $FELD2 $NTIMESTEPS)>>logprogram 2>>logtime
    # printf "Iteration_Nr. %d" $i >> logtime
   (gtime -f "3 %e %P " $PATH_TO $FELD3 $FELD3 $NTIMESTEPS)>>logprogram 2>>logtime
    done
awk '$1=="1" {printf"%s, %s\n", $2, $3}' logtime > logtime1.csv
awk '$1=="2" {printf"%s, %s\n", $2, $3}' logtime > logtime2.csv
awk '$1=="3" {printf"%s, %s\n", $2, $3}' logtime > logtime3.csv


    
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
