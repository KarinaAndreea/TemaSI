#include<stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <stdbool.h>
#include "auxx.c"


#define IN "test.txt"
unsigned char *key3 = (unsigned char *)"\x6D\x65\x64\x69\x61\x6E\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20";
const char* k1 = "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f";
const char* k2 = "\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f\x0f";
int main(int argc, char* argv[]) {

    int  a_km_fd, km_a_fd;
    // 16 bytes pentru cheie
    char * k = (char *) malloc(16);
  //canale de comunicare cu nodul A
  //am folosit named pipes
    a_km_fd = open("tmp/a_km", O_RDONLY);
    km_a_fd = open("tmp/km_a", O_WRONLY);


    if (a_km_fd == -1) {
        printf("Error opening file");
        exit(1);
    }
//citim modul de criptare transmis de A
    char mode;
    int result = read(a_km_fd, &mode, 1);
    printf("%d Read the mode from A: %c\n", result, mode);
    if (mode == 'C')
//criptam cheia K1 folosind K3
        encrypt((void *) k1, k, key3);
    else
        encrypt((void *) k2, k, key3);
 //trimitam cheia pe blocuri
    int bytes_written = 0;
    while (bytes_written < BLOCK_SIZE) {
        bytes_written += write(km_a_fd, k, BLOCK_SIZE);
    }
    printf("Sent the key to A\n");
}
