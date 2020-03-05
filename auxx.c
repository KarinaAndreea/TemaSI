#include <openssl/aes.h>

const int BLOCK_SIZE = 16;
// vectorul de initializare
unsigned char *iv = (unsigned char *)"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f";


void encrypt(void *in, void *out, unsigned char *key) {
    AES_KEY encrypt;
    AES_set_encrypt_key(key, 128, &encrypt);
    AES_encrypt((const unsigned char *) in, (unsigned char *) out, &encrypt);
}
void decrypt(void *in, void *out, unsigned char* key) {
    AES_KEY decrypt_k;
    AES_set_decrypt_key(key, 128, &decrypt_k);
    AES_decrypt((const unsigned char *) in, (unsigned char *) out, &decrypt_k);
}
void xorBlocks(char *b1, char *b2) {

    for (int i = 0; i <BLOCK_SIZE; i++) {
        b1[i] ^= b2[i];
    }

}

void printKey(char* k) {
    for (int i = 0; i < BLOCK_SIZE; i++) {
        printf("%X ", k[i]);
    }
    printf("\n");
}