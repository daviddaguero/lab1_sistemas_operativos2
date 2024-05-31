Para compilar el server:
gcc -o server server.c cJSON.c -L. -lemergency -lsupplies_update -lpossible_infection


Para compilar librería estática:
gcc -c possible_infection.c -o possible_infection.o
ar rcs libpossible_infection.a possible_infection.o

Para compilar la librería dinámica:
gcc -shared -fPIC -o libemergency.so emergency.c
Pegar el archivo.so en la carpeta lib

Para compilar con cMake:
- crea un directorio de compilación fuera del directorio fuente (por ejemplo, build)
- cd build
- cmake ..
- cmake --build .

