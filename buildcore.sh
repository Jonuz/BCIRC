mkdir build

gcc main.c ./include/*.h ./src/*.c -o ./build/irc -Wall -ldl -std=gnu11 -g
gcc main.c ./include/*.h ./src/*.c  -Wall -ldl -std=gnu11 -g -c -fPIC

ar rcs ./plugins/libbcirc.a *.o

rm ./include/*.gch
rm *.o
