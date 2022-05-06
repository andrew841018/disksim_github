use c call python need to use following command:
gcc $(python3.8-config --cflags --ldflags) main.c -o main -fPIE -lpython3.8

