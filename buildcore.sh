mkdir build

cc main.c ./src/*.c -o ./build/irc -Wall -ldl -std=gnu11 -g
cc main.c ./src/*.c -Wall -ldl -std=gnu11 -g -c -fPIC

ar rcs ./plugins/libbcirc.a *.o

#rm ./include/*.gch
rm *.o
