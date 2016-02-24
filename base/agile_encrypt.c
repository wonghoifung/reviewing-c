#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "agile_bit.h"
#include "agile_encrypt.h"

// key transform
static const int DesTransform[56] = {
   57, 49, 41, 33, 25, 17,  9,  1, 58, 50, 42, 34, 26, 18,
   10,  2, 59, 51, 43, 35, 27, 19, 11,  3, 60, 52, 44, 36,
   63, 55, 47, 39, 31, 23, 15,  7, 62, 54, 46, 38, 30, 22,
   14,  6, 61, 53, 45, 37, 29, 21, 13,  5, 28, 20, 12,  4
};
// key rotate
static const int DesRotations[16] = {
   1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1
};
// sub key
static const int DesPermuted[48] = {
   14, 17, 11, 24,  1,  5,  3, 28, 15,  6, 21, 10,
   23, 19, 12,  4, 26,  8, 16,  7, 27, 20, 13,  2,
   41, 52, 31, 37, 47, 55, 30, 40, 51, 45, 33, 48,
   44, 49, 39, 56, 34, 53, 46, 42, 50, 36, 29, 32
};
// data init permute
static const int DesInitial[64] = {
   58, 50, 42, 34, 26, 18, 10,  2, 60, 52, 44, 36, 28, 20, 12,  4,
   62, 54, 46, 38, 30, 22, 14,  6, 64, 56, 48, 40, 32, 24, 16,  8,
   57, 49, 41, 33, 25, 17,  9,  1, 59, 51, 43, 35, 27, 19, 11,  3,
   61, 53, 45, 37, 29, 21, 13,  5, 63, 55, 47, 39, 31, 23, 15,  7
};
// data expand permute
static const int DesExpansion[48] = {
   32,  1,  2,  3,  4,  5,  4,  5,  6,  7,  8,  9,
    8,  9, 10, 11, 12, 13, 12, 13, 14, 15, 16, 17,
   16, 17, 18, 19, 20, 21, 20, 21, 22, 23, 24, 25,
   24, 25, 26, 27, 28, 29, 28, 29, 30, 31, 32,  1
};
// sbox
static const int DesSbox[8][4][16] = {
   {
	   {14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7},
	   { 0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8},
	   { 4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0},
	   {15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13},
   },

   {
	   {15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10},
	   { 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5},
	   { 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15},
	   {13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9},
   },

   {
	   {10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8},
	   {13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1},
	   {13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7},
	   { 1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12},
   },

   {
	   { 7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15},
	   {13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9},
	   {10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4},
	   { 3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14},
   },

   {
	   { 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9},
	   {14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6},
	   { 4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14},
	   {11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3},
   },

   {
	   {12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11},
	   {10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8},
	   { 9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6},
	   { 4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13},
   },

   {
	   { 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1},
	   {13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6},
	   { 1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2},
	   { 6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12},
   },

   {
	   {13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7},
	   { 1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2},
	   { 7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8},
	   { 2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11},
   }
};
// pbox
static const int DesPbox[32] = {
   16,  7, 20, 21, 29, 12, 28, 17,  1, 15, 23, 26,  5, 18, 31, 10,
    2,  8, 24, 14, 32, 27,  3,  9, 19, 13, 30,  6, 22, 11,  4, 25
};
// data final permute
static const int DesFinal[64] = {
   40,  8, 48, 16, 56, 24, 64, 32, 39,  7, 47, 15, 55, 23, 63, 31,
   38,  6, 46, 14, 54, 22, 62, 30, 37,  5, 45, 13, 53, 21, 61, 29,
   36,  4, 44, 12, 52, 20, 60, 28, 35,  3, 43, 11, 51, 19, 59, 27,
   34,  2, 42, 10, 50, 18, 58, 26, 33,  1, 41,  9, 49, 17, 57, 25
};

typedef enum DesEorD_ {
   encipher,
   decipher
} DesEorD;

static void permute(unsigned char* bits, const int* mapping, int n) {
   unsigned char temp[8];
   int i;
   memset(temp, 0, (int)ceil(n/8));
   for (i=0; i<n; ++i) 
      agile_bit_set(temp, i, agile_bit_get(bits, mapping[i]-1));
   memcpy(bits, temp, (int)ceil(n/8));
}

static int des_main(const unsigned char* source, unsigned char* target, const unsigned char* key, DesEorD direction) {
   static unsigned char subkeys[16][7];
   unsigned char temp[8], lkey[4], rkey[4], lblk[6], rblk[6], fblk[6], xblk[6], sblk;
   int row, col, i, j, k, p;
   if (key != NULL) { // build 16 keys
      memcpy(temp, key, 8);
      permute(temp, DesTransform, 56);
      memset(lkey, 0, 4);
      memset(rkey, 0, 4);
      for (j=0; j<28; ++j) agile_bit_set(lkey, j, agile_bit_get(temp, j));
      for (j=0; j<28; ++j) agile_bit_set(rkey, j, agile_bit_get(temp, j+28));
      for (i=0; i<16; ++i) {
         agile_bit_rot_left(lkey, 28, DesRotations[i]);
         agile_bit_rot_left(rkey, 28, DesRotations[i]);
         for (j=0; j<28; ++j) agile_bit_set(subkeys[i], j, agile_bit_get(lkey, j));
         for (j=0; j<28; ++j) agile_bit_set(subkeys[i], j+28, agile_bit_get(rkey, j));
         permute(subkeys[i], DesPermuted, 48);
      }
   }
   memcpy(temp, source, 8);
   permute(temp, DesInitial, 64);
   memcpy(lblk, &temp[0], 4);
   memcpy(rblk, &temp[4], 4);
   for (i=0; i<16; ++i) { // encipher or decipher the source text
      memcpy(fblk, rblk, 4);
      permute(fblk, DesExpansion, 48);
      if (direction == encipher) {
         agile_bit_xor(fblk, subkeys[i], xblk, 48);
         memcpy(fblk, xblk, 6);
      } else {
         agile_bit_xor(fblk, subkeys[15-i], xblk, 48);
         memcpy(fblk, xblk, 6);
      }
      // sbox substitutions
      p = 0;
      for (j=0; j<8; ++j) {
         row = (agile_bit_get(fblk, (j*6)+0)*2) + (agile_bit_get(fblk, (j*6)+5)*1);
         col = (agile_bit_get(fblk, (j*6)+1)*8) + (agile_bit_get(fblk, (j*6)+2)*4) +
               (agile_bit_get(fblk, (j*6)+3)*2) + (agile_bit_get(fblk, (j*6)+4)*1);
         sblk = (unsigned char)DesSbox[j][row][col];
         for (k=4; k<8; ++k) {
            agile_bit_set(fblk, p, agile_bit_get(&sblk, k));
            p += 1;
         }
      }
      // pbox
      permute(fblk, DesPbox, 32);
      agile_bit_xor(lblk, fblk, xblk, 32);
      memcpy(lblk, rblk, 4);
      memcpy(rblk, xblk, 4);
   }
   memcpy(&target[0], rblk, 4);
   memcpy(&target[4], lblk, 4);
   permute(target, DesFinal, 64);
   return 0;
}

void agile_des_encipher(const unsigned char* plaintext, unsigned char* ciphertext, const unsigned char* key) {
   des_main(plaintext, ciphertext, key, encipher);
}

void agile_des_decipher(const unsigned char* ciphertext, unsigned char* plaintext, const unsigned char* key) {
   des_main(ciphertext, plaintext, key, decipher);
}

void agile_cbc_encipher(const unsigned char* plaintext, unsigned char* ciphertext, const unsigned char* key, int size) {
   unsigned char temp[8];
   int i;
   agile_des_encipher(&plaintext[0], &ciphertext[0], key);
   i = 8;
   while (i < size) {
      agile_bit_xor(&plaintext[i], &ciphertext[i-8], temp, 64);
      agile_des_encipher(temp, &ciphertext[i], NULL);
      i += 8;
   }
}

void agile_cbc_decipher(const unsigned char* ciphertext, unsigned char* plaintext, const unsigned char* key, int size) {
   unsigned char temp[8];
   int i;
   agile_des_decipher(&ciphertext[0], &plaintext[0], key);
   i = 8;
   while (i < size) {
      agile_des_decipher(&ciphertext[i], temp, NULL);
      agile_bit_xor(&ciphertext[i-8], temp, &plaintext[i], 64);
      i += 8;
   }
}

// pow(a,b)%n
static long long modexp(long long a, int b, int n) {
   long long y;
   y = 1;
   while (b != 0) {
      if (b & 1) y = (y * a) % n;
      a = (a * a) % n;
      b >>= 1;
   }
   return y;
}

void agile_rsa_encipher(int plaintext, long long* ciphertext, agile_rsa_pub_key pubkey) {
   *ciphertext = modexp(plaintext, pubkey.e, pubkey.n);
}

void agile_rsa_decipher(long long ciphertext, int* plaintext, agile_rsa_pri_key prikey) {
   *plaintext = modexp(ciphertext, prikey.d, prikey.n);
}

//////////////////////////////

void test_agile_encrypt() {
   #if 0
   unsigned char plaintext[8] = {'1','2','3','4','5','6','7','8'};
   dump_bits(plaintext, 64); printf("\n");
   unsigned char ciphertext[8] = {0};
   unsigned char key[8] = {'a','b','c','d','e','f','g','h'};
   agile_des_encipher(plaintext, ciphertext, key);
   dump_bits(ciphertext, 64); printf("\n");
   unsigned char plaintext2[8] = {0};
   agile_des_decipher(ciphertext, plaintext2, key);
   dump_bits(plaintext2, 64); printf("\n");
   #endif
   {
      unsigned char key[8] = {'a','b','c','d','e','f','g','h'};
      unsigned char* plaintext = (unsigned char*)"123456789";
      int size = strlen((char*)plaintext) + 1;
      printf("actual plaintext:\n");
      dump_bits(plaintext, size*8); printf("\n");
      printf("plaintext:\n");
      dump_bits(plaintext, size*8+64); printf("\n");
      unsigned char* ciphertext = (unsigned char*)malloc(size+8);
      memset(ciphertext, 0, size+8);
      agile_cbc_encipher(plaintext, ciphertext, key, size);
      printf("ciphertext:\n");
      dump_bits(ciphertext, size*8+64); printf("\n");
      unsigned char* plaintext2 = (unsigned char*)malloc(size+8);
      memset(plaintext2, 0, size+8);
      agile_cbc_decipher(ciphertext, plaintext2, key, size);
      printf("plaintext2:\n");
      dump_bits(plaintext2, size*8+64); printf("\n");
      free(ciphertext);
      free(plaintext2);
   }
   {
      int p = 11, q = 19; // big prime number
      int n = p * q;
      int e = 17; // odd number, having no same factor with (p-1)(q-1)
      int d = 1;
      while (1) {
         if ((e * d) % ((p-1) * (q-1)) == 1) break;
         d += 1;
      }
      // e = 5327;
      // d = 162623;
      // n = 345347;
      printf("e:%d, d:%d, n:%d\n", e, d, n);
      agile_rsa_pub_key pubkey;
      pubkey.e = e;
      pubkey.n = n;
      agile_rsa_pri_key prikey;
      prikey.d = d;
      prikey.n = n;
      int plaintext = 0;
      int fcnt = 0;
      while (fcnt < 10) {
         long long ciphertext = 0;
         agile_rsa_encipher(plaintext, &ciphertext, pubkey);
         int plaintext2 = 0;
         agile_rsa_decipher(ciphertext, &plaintext2, prikey);
         if (plaintext != plaintext2) {
            printf("%d failed\n", plaintext);
            fcnt += 1;
            plaintext += 1;
            continue;
         }
         printf("%d ok\n", plaintext);
         plaintext += 1;
      }
   }
}

