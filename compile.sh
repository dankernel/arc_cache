gcc -finput-charset=UTF-8  -D__KERNEL__ -pg -g -lm -O0 -o main main.c
ctags -R --exclude=dox
# ./main data/bit.csv 16
# ./main data/hm_1.csv 8
./main data/proj_4.csv 1
