gcc -finput-charset=UTF-8  -D__KERNEL__ -pg -g -lm -O4 -o main main.c
ctags -R --exclude=dox
# ./main data/bit.csv 16
# ./main data/hm_1.csv 2
./main data/proj_4.csv 4
# ./main data/dk.csv 1024
