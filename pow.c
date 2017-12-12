#include <stdio.h>
#include <memory.h>
#include <string.h>
#include "sha256.h"

WORD hash2integer(BYTE h[32]);
static WORD pair2sci(WORD l[2]);
static BYTE* next_nonce(BYTE nonce[32]);
int check_pow(BYTE nonce[32], int, BYTE data[32]);
BYTE* mine(BYTE nonce[32], int, BYTE data[32]);
void test_hash();
void test_hash2integer();
void test_check_pow();

WORD hash2integer(BYTE h[32]) {
  WORD x = 0;
  WORD y[2];
  for (int i = 0; i < 31; i++) {
    if (h[i] == 0) {
      x += 8;
      y[1] = h[i+1];
      continue;
    } else if (h[i] < 2) {
      x += 7;
      y[1] = (h[i] * 128) + (h[i+1] / 2);
    } else if (h[i] < 4) {
      x += 6;
      y[1] = (h[i] * 64) + (h[i+1] / 4);
    } else if (h[i] < 8) {
      x += 5;
      y[1] = (h[i] * 32) + (h[i+1] / 8);
    } else if (h[i] < 16) {
      x += 4;
      y[1] = (h[i] * 16) + (h[i+1] / 16);
    } else if (h[i] < 32) {
      x += 3;
      y[1] = (h[i] * 8) + (h[i+1] / 32);
    } else if (h[i] < 64) {
      x += 2;
      y[1] = (h[i] * 4) + (h[i+1] / 64);
    } else if (h[i] < 128) {
      x += 1;
      y[1] = (h[i] * 2) + (h[i+1] / 128);
    } else {
      y[1] = h[i];
    }
    break;
  }
  y[0] = x;
  return(pair2sci(y));
}
static WORD pair2sci(WORD l[2]) {
  return((256*l[0]) + l[1]);
}
int check_pow(BYTE nonce[32], int difficulty, BYTE data[32]) {

  BYTE text[66];//32+2+32
  for (int i = 0; i < 32; i++) 
    text[i] = data[i];
  text[32] = difficulty / 256;
  text[33] = difficulty % 256;
  for (int i = 0; i < 32; i++) 
    text[i+34] = nonce[i];
  SHA256_CTX ctx;
  sha256_init(&ctx);
  sha256_update(&ctx, text, 66);
  BYTE buf[32];
  sha256_final(&ctx, buf);
  
  int i = hash2integer(buf);
  //printf("pow did this much work %d \n", i);
  return(i > difficulty);
}
static BYTE* next_nonce(BYTE nonce[32]){
  for (int i = 0; i < 32; i++) {
    if (nonce[i] == 255) {
      nonce[i] = 0;
    } else {
      nonce[i] += 1;
      return nonce;
    }
  }
  return(0);
}
BYTE* mine(BYTE nonce[32], int difficulty, BYTE data[32]) {
  while (1) {
    //printf("mining\n");
    if (check_pow(nonce, difficulty, data)) 
      return nonce;
    nonce = next_nonce(nonce);
  }
}
void write_nonce(BYTE x[32]) {
  FILE *f = fopen("nonce.txt", "w");
  if (f == NULL) {
      printf("Error opening file!\n");
      //exit(1);
    }
  rewind(f);
  //fprintf(f, "%s", x);
  fwrite(x, 1, 32, f);
  fclose(f);
  return;
}
int read_input(BYTE B[32], BYTE N[32]) {
  FILE *fileptr;
  //char *buffer;
  //long filelen;

  fileptr = fopen("mining_input", "rb");  // Open the file in binary mode
  fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
  int filelen = ftell(fileptr);             // Get the current byte offset in the file
  rewind(fileptr); 

  fread(B, 32, 1, fileptr);
  fread(N, 32, 1, fileptr);
  BYTE buffer[10];
  fread(buffer, filelen-64, 1, fileptr);
  int diff = 0;
  BYTE c = 1;
  for (int i = 0; i < 10; i++) {
    c = buffer[i];
    if (c == 0) {
      break;
    }
    diff *= 10;
    diff += (c - '0');
  }
  
  fclose(fileptr); // Close the file
  return diff;
}
int main()
{
  BYTE bhash[32];
  BYTE nonce[32];
  int diff = read_input(bhash, nonce);
  mine(nonce, diff, bhash); //nonce, difficulty, data
  write_nonce(nonce);
  //test_check_pow();
  return(0);
}
void test_check_pow() {
  //["pow","2w1EW/I07ZnVg8hK4TzPEUA2XXyh2MpdyLgntn/42dI=",6452,995842502870377667814772]
  BYTE data[32] = {
    70,131,192,39,4,93,79,150,232,178,119,91,87,183,101,245,
    48,56,180,196,47,44,184,68,130,255,91,39,44,98,51,216
  };
  BYTE nonce[32] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,210,224,186,
    241,224,230,139,188,229,116
  };
  int out = check_pow(nonce, 6452, data);
  printf("check pow gave %i\n", out);
  //should be a 1.
  return;
}
void test_hash() {
  BYTE bhash[32];
  SHA256_CTX ctx;
  for (int i = 0; i < 66; i++) {
    bhash[i] = 0;
  }
  sha256_init(&ctx);
  sha256_update(&ctx, bhash, 66);
  BYTE buf[32];
  sha256_final(&ctx, buf);
  printf("%i, %i, %i, %i, %i, %i, %i, %i, \n",
         buf[0], buf[1], buf[2], buf[3],
         buf[4], buf[5], buf[6], buf[7]);
  // should be 239, 187, 3, 183, 167, 246, 253, 60,
  return;
}

void test_hash2integer() {
  BYTE buf[32] = {
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    200,200,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
  };
  printf("%i\n", hash2integer(buf));
  // should be 32968
  return;
}

/*
["pow","fhRzEfYz34zyYf+76zvvvW3QQPjeFy6QhLcZd9NJCEM=",6452,114719552758006781619496202257071179514670008855271061514073092320978544337709]
pow:check_pow(x, 32).

//take hash of data.
//put nonce into binary.
check_pow(nonce, 6453, data);
    

check_pow(P, HashSize) ->
    HashSize = 32,
    N = P#pow.nonce,
    Diff = P#pow.difficulty,
    Data = P#pow.data,
    H1 = hash:doit(Data, HashSize),
    X = HashSize*8,
    Y = <<H1/binary, Diff:16, N:X>>,
    H2 = hash:doit(Y, HashSize),
    I = hash2integer(H2),
    I > Diff.
*/
