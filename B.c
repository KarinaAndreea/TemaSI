#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include <stdbool.h>
#include "auxx.c"

void decryptKey(char *k, unsigned char*  key3) {
    decrypt(k, k, key3);
}


//eliminam paddingul
int data_bytes(char *buffer) {
    int end = BLOCK_SIZE - 1;
    int padded_bytes = (int) buffer[end];
    return BLOCK_SIZE - (padded_bytes + 1);
}

void receive_file(int fd, char *k, unsigned char *iv, char *mode) {

    bool finished = false;
    void* cipherTextBuffer = malloc(BLOCK_SIZE);
    void* previousBlock = malloc(BLOCK_SIZE);
    memcpy(previousBlock, iv, BLOCK_SIZE);
    char* plainTextBuffer = (char *) malloc(BLOCK_SIZE);
    char* printBuffer = (char *) malloc(BLOCK_SIZE + 1);
   // printBuffer[BLOCK_SIZE] = 0;
    //printBuffer[0] = 0;
    if (mode[0] == 'C'){
      FILE* in = fdopen(fd, "rb");
      while (!feof(in)) {
//citim un bloc de 16 biti de ciphertext
        int bytes_read = 0;
        bytes_read += fread(cipherTextBuffer + bytes_read, 1, BLOCK_SIZE - bytes_read, in);

             int data_len;
        // daca bytes_read sunt 0 facem unpadding
        if (feof(in)) {
            data_len = data_bytes(printBuffer);
            printBuffer[data_len] = 0;
            break;
        }
            //nu este bloc final
        else data_len = BLOCK_SIZE;
        printf("%s", printBuffer);
        // decriptam blocul citit
        decrypt(cipherTextBuffer, plainTextBuffer, (unsigned char *) k);
        // facem un xor intre blocul decriptat si blocul anterior de ciphertext
    //initial acest bloc anterior de ciphertext este IV
        xorBlocks(plainTextBuffer, (char *) previousBlock);
        mempcpy(previousBlock, cipherTextBuffer, BLOCK_SIZE);
      //printBuffer retine plaintextul
        mempcpy(printBuffer, plainTextBuffer, data_len);
    }
    printf("%s", printBuffer);
    free(printBuffer);
    free(plainTextBuffer);
    free(previousBlock);
    fclose(in);
  }
  else  if (mode[0] == 'O'){
        FILE* in = fdopen(fd, "rb");
        while (!feof(in)) {
       //criptam previousBlock si retinem rezultatul in plainTextBuffer
     //initial previousBlock este IV
          encrypt(previousBlock,plainTextBuffer, (unsigned char *) k);
//citim un bloc de ciphertext
          int bytes_read = 0;
          bytes_read += fread(cipherTextBuffer + bytes_read, 1, BLOCK_SIZE - bytes_read, in);

               int data_len;
        if (feof(in)) {
            data_len = data_bytes(printBuffer);
            printBuffer[data_len] = 0;
            break;
        }
        else data_len = BLOCK_SIZE;

          printf("%s", printBuffer);
//transmitem mai departe blocul criptat care va fi folosit la pasul urmator
         mempcpy(previousBlock,plainTextBuffer, BLOCK_SIZE);
//operatia de xor intre ciphertext si blocul criptat
          xorBlocks(plainTextBuffer, (char *) cipherTextBuffer);
          mempcpy(printBuffer, plainTextBuffer, data_len);
      }
      printf("%s", printBuffer);
      free(printBuffer);
      free(plainTextBuffer);
      free(previousBlock);
      fclose(in);
    }
}


int main(int argc, char* argv[]) {

    int a_b_fd, b_a_fd, km_b_fd;
    // 16 bytes for the key
    char * k = (char *) malloc(16);
   unsigned char * k3 = (unsigned char * ) malloc(16);

    a_b_fd = open("tmp/a_b", O_RDONLY);
    b_a_fd = open("tmp/b_a", O_WRONLY);
    km_b_fd = open("tmp/km_b", O_RDONLY);

    if (a_b_fd == -1)
        printf("Error");
    char mode;
    int result;
    result = read(a_b_fd, &mode, 1);
    if (result == -1)
        printf("Error");
    printf(" %d Read the mode %c\n",result, mode);


    // asteptam cheia de la nodul A
    int read_bytes = 0;
    while(read_bytes < BLOCK_SIZE) {
        read_bytes += read(a_b_fd, k + read_bytes, (size_t) (BLOCK_SIZE - read_bytes));
    }
    printf("Read the key from A\n");
   

   read_bytes = 0;
    while(read_bytes < BLOCK_SIZE) {
        read_bytes += read(a_b_fd, k3 + read_bytes, (size_t) (BLOCK_SIZE - read_bytes));
    }
    printf("Read the K3 from A\n");

    decryptKey(k, k3);
    printKey(k);
    
    
    char signal = 0;
    write(b_a_fd, &signal, 1);

    receive_file(a_b_fd, k, iv, &mode);
    printf("\n\nReceived the file from A\n");
}

