#include<stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <stdbool.h>
#include "auxx.c"
#define IN "text.txt"

unsigned char *key3 = (unsigned char *)"\x6D\x65\x64\x69\x61\x6E\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20";
//inputul nu este multiplu de BLOCK_SIZE=16 => facem padding
void padBuffer(char *buffer, int bytes) {
    int toPad = BLOCK_SIZE - bytes - 1;
    for (int i = bytes; i < BLOCK_SIZE -1; i++) {
        buffer[i] = 0;
    }
    buffer[BLOCK_SIZE - 1] = (char) toPad;
    printf("Padded with %d bytes\n", toPad + 1);
}
//decriptam cheia primita de la KM folosind K3
void decryptKey(char *k) {
    decrypt(k, k, key3);
}

void send_file(int fd, char *k, unsigned char *iv, char *mode) {
    bool finished = false;
    char* buffer = (char *) malloc(BLOCK_SIZE);
    FILE* in = fopen(IN, "rb");

    void* cipherTextBuffer = malloc(BLOCK_SIZE);
    void* encryptBuffer = malloc(BLOCK_SIZE);
    void* previousBlock = malloc(BLOCK_SIZE);
        memcpy(previousBlock, iv, BLOCK_SIZE);
    if (mode[0] == 'C') {
    while (!feof(in))
    {  //citim un bloc de plaintext
        int bytes_read = 0;
        bytes_read += (int) fread(buffer + bytes_read, 1, (size_t) (BLOCK_SIZE - bytes_read), in);
        printf("Read a block of %d bytes\n", bytes_read);
          if (bytes_read < BLOCK_SIZE)
        {
            // am ajuns la finalul fisierului si am citit un bloc de biti de dimensiune mai mica decat BLOCK_SIZE
            padBuffer((char *) buffer, bytes_read);
            finished = true;
        }
            // xor intre plaintext si blocul anterior de  ciphertext
           // la pasul 0 operatia de xor este facuta intre IV si plaintext
            xorBlocks((char *) buffer, (char *) previousBlock);
           //criptam rezultatul obtinut dupa xor
            encrypt(buffer, cipherTextBuffer, (unsigned char *) k);
           //retinem in previousBlock ciphertextul pentru a fi folosit in pasul urmator
            memcpy(previousBlock, cipherTextBuffer, BLOCK_SIZE);
       //trimitem un bloc de ciphertext
        int written_bytes = 0;
        while(written_bytes < BLOCK_SIZE) {
            written_bytes += write(fd, cipherTextBuffer + written_bytes, (size_t) (BLOCK_SIZE - written_bytes));
        }
        printf("Written %d bytes to B\n", written_bytes);
        if (finished) {
            free(buffer);
            free(cipherTextBuffer);
            free(previousBlock);
            fclose(in);
            close(fd);
            break;
        }
    }
} else  if (mode[0] == 'O') {
  while (!feof(in))
  {  //primul pas in modul OFB este criptarea
     //blocul previousBlock este criptat iar rezultatul este retinut in cipherTextBuffer
    //initial previousBlock este IV
      encrypt(previousBlock, cipherTextBuffer, (unsigned char *) k);
      memcpy(previousBlock, cipherTextBuffer, BLOCK_SIZE);
      int bytes_read = 0;
      bytes_read += (int) fread(buffer + bytes_read, 1, (size_t) (BLOCK_SIZE - bytes_read), in);
      printf("Read a block of %d bytes\n", bytes_read);
       if (bytes_read < BLOCK_SIZE)
        {
            padBuffer((char *) buffer, bytes_read);
            finished = true;
        }
     //xor intre plaintext si blocul anterior criptat cu cheia K2
      xorBlocks((char *) buffer, (char *) previousBlock);
    //trimitem blocul de ciphertext
      int written_bytes = 0;
      while(written_bytes < BLOCK_SIZE) {
          written_bytes += write(fd, buffer + written_bytes, (size_t) (BLOCK_SIZE - written_bytes));
      }

      printf("Written %d bytes to B\n", written_bytes);
      if (finished) {
          printf("Cipherte %d bytes to B\n", written_bytes);
          free(buffer);
          free(cipherTextBuffer);
          free(previousBlock);
          fclose(in);
          close(fd);
          break;
      }
  }

}
}


int main(int argc, char* argv[]) {

    int a_b_fd, a_km_fd, b_a_fd, km_a_fd, km_b_fd;
    char * k = (char *) malloc(16);
    //folosim named pipes pentru comunicarea intre nodurile A si B, pentru comunicarea lui A cu KM
    mkfifo("tmp/a_b", 0777);
    mkfifo("tmp/b_a", 0777);
    mkfifo("tmp/km_a", 0777);
    mkfifo("tmp/a_km", 0777);
    mkfifo("tmp/km_b", 0777);
    mkfifo("tmp/b_km", 0777);

    a_b_fd = open("tmp/a_b", O_WRONLY);
    a_km_fd = open("tmp/a_km", O_WRONLY);
    b_a_fd = open("tmp/b_a", O_RDONLY);
    km_a_fd = open("tmp/km_a", O_RDONLY);
    km_b_fd = open("tmp/km_b", O_WRONLY);


    int result;
    // trimitem modul de criptare catre B si KM
    while(!(result = write(a_b_fd, argv[1], 1)));
    printf("%d Sent the mode to B: %c\n", result, argv[1][0]);

    while(!(result = write(a_km_fd, argv[1], 1)));
    printf("%d Sent the mode to KM: %c\n", result, argv[1][0]);

    // citim cheia de la KM
    int read_bytes = 0;
    while(read_bytes < BLOCK_SIZE) {
        read_bytes += read(km_a_fd, k + read_bytes, (size_t) (BLOCK_SIZE - read_bytes));
    }

    printf("Read the key from KM\n");
//trimitem cheia primita de la KM nodului B folosind un FIFO
    int bytes_written = 0;
    while (bytes_written < BLOCK_SIZE) {
       bytes_written += write(a_b_fd, k, BLOCK_SIZE);
   }
    printf("Sent the key to B\n");
    sleep(10);
    decryptKey(k);
    printKey(k);
//trimitem nodului B cheia K3
    bytes_written = 0;
    while (bytes_written < BLOCK_SIZE) {
       bytes_written += write(a_b_fd, key3, BLOCK_SIZE);
   }
    printf("Sent the K3 to B\n");
    // asteptam confirmarea de la B
    int response_code;
    while(!read(b_a_fd, &response_code, 1));
    printf("Got the response from B\n");
  //trimitem lui B fisierul criptat pe blocuri
    send_file(a_b_fd, k, iv, argv[1]);
    printf("Sent the file to B\n");
}

