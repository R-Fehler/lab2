for (( i = 0; i < 5; i++ )); do
    print i >>logtime
(time ./gameoflife 1024 1024 20)>logprogram 2>logtime
done
for (( i = 0; i < 5; i++ )); do
    print i >>logtime
(time ./gameoflife 2048 2048 20)>logprogram 2>logtime
done
for (( i = 0; i < 5; i++ )); do
    print i >>logtime
(time ./gameoflife 4096 4096 20)>logprogram 2>logtime
done
