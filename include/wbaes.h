#ifndef GENTABLES_H
#define GENTABLES_H

#include "aes.h"
#include "WBMatrix/WBMatrix.h"

u32 TypeII_to[6][16][256];//Type II
u32 TypeIII_to[6][16][256];//Type III

u32 TypeII_left[3][16][256];//Type II
u32 TypeIII_left[2][16][256];//Type III

u32 TypeII_right[3][16][256];//Type II
u32 TypeIII_right[2][16][256];//Type III

u32 TypeIII_final[16][256];//Type III
u32 TypeII_final[16][256];//Type II

u8 TypeIV[16][16];

void wbaes_gen(u8 key[16]);
void wbaes_encrypt(u8 input[16], u8 output[16]);
void printstate(unsigned char * in);

#endif // GENTABLES_H