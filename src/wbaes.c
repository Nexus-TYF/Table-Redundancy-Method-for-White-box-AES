#include "wbaes.h"

void printstate(unsigned char * in)
{
    for(int i = 0; i < 16; i++)
    {
        printf("%.2X", in[i]);
    }
    printf("\n");
}

void wbaes_gen(u8 key[16])
{
    u8 expandedKey[176];
    expandKey (key, expandedKey);
    
    static u8 nibble[16] = {0x01, 0x02, 0x0C, 0x05, 0x07, 0x08, 0x0A, 0x0F, 0x04, 0x0D, 0x0B, 0x0E, 0x09, 0x06, 0x00, 0x03};
    static u8 nibble_inv[16] = {0x0e, 0x00, 0x01, 0x0f, 0x08, 0x03, 0x0d, 0x04, 0x05, 0x0c, 0x06, 0x0a, 0x02, 0x09, 0x0b, 0x07}; 

    M8 L_to[6][16];
    M8 L_inv_to[6][16];

    M8 L_left[2][16];
    M8 L_inv_left[2][16];

    M8 L_right[2][16];
    M8 L_inv_right[2][16];

    M32 MB_to[6][4];
    M32 MB_inv_to[6][4];

    M32 MB_left[3][4];
    M32 MB_inv_left[2][4];

    M32 MB_right[3][4];
    M32 MB_inv_right[2][4];

    M8 L_final[16];
    M8 L_inv_final[16];

    M32 MB_inv_final[4];

    for(int i = 0; i < 6; i++)
    {
        for(int j = 0; j < 16; j++)
        {
            genMatpairM8(&L_to[i][j], &L_inv_to[i][j]);
        }
    }
    for(int i = 0; i < 2; i++)
    {
        for(int j = 0; j < 16; j++)
        {
            genMatpairM8(&L_left[i][j], &L_inv_left[i][j]);
            genMatpairM8(&L_right[i][j], &L_inv_right[i][j]);
        }
    }
    for(int j = 0; j < 16; j++)
    {
        genMatpairM8(&L_final[j], &L_inv_final[j]);
    }
    for(int i = 0; i < 6; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            genMatpairM32(&MB_to[i][j], &MB_inv_to[i][j]);
        }
    }
    for(int i = 0; i < 2; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            genMatpairM32(&MB_left[i][j], &MB_inv_left[i][j]);
            genMatpairM32(&MB_right[i][j], &MB_inv_right[i][j]);
        }
    }
    M32 combine_MB[4];
    for(int j = 0; j < 4; j++)
    {
        genMatpairM32(&combine_MB[j], &MB_inv_final[j]);
        randM32(&MB_left[2][j]);
        initM32(&MB_right[2][j]);
        for(int r = 0; r < 32; r++)
        {
            MB_right[2][j].M[r] = MB_left[2][j].M[r] ^ combine_MB[j].M[r];
        }
    }

    u32 Tyi[4][256];
    for (int x = 0; x < 256; x++)
    {
      Tyi[0][x] = (gMul(2, x) << 24) | (x << 16) | (x << 8) | gMul(3, x);
      Tyi[1][x] = (gMul(3, x) << 24) | (gMul(2, x) << 16) | (x << 8) | x;
      Tyi[2][x] = (x << 24) | (gMul(3, x) << 16) | (gMul(2, x) << 8) | x;
      Tyi[3][x] = (x << 24) | (x << 16) | (gMul(3, x) << 8) | gMul(2, x);
    }

    M32 Out_L_to[6][4];
    for(int i = 0; i < 6; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            MatrixcomM8to32(L_to[i][4 * j], L_to[i][4 * j + 1], L_to[i][4 * j + 2], L_to[i][4 * j + 3], &Out_L_to[i][j]);
        }
    }
    M32 Out_L_left[2][4];
    M32 Out_L_right[2][4];
    for(int i = 0; i < 2; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            MatrixcomM8to32(L_left[i][4 * j], L_left[i][4 * j + 1], L_left[i][4 * j + 2], L_left[i][4 * j + 3], &Out_L_left[i][j]);
            MatrixcomM8to32(L_right[i][4 * j], L_right[i][4 * j + 1], L_right[i][4 * j + 2], L_right[i][4 * j + 3], &Out_L_right[i][j]);
        }
    }
    M32 Out_L_final[4];
    for(int j = 0; j < 4; j++)
    {
        MatrixcomM8to32(L_final[4 * j], L_final[4 * j + 1], L_final[4 * j + 2], L_final[4 * j + 3], &Out_L_final[j]);
    }
    
    int columnindex[]={0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3};
    //Round 1
    shiftRows (expandedKey + 16 * 0);
    for(int j = 0; j < 16; j++)//type_II
    {
        u8 temp_u8;
        u32 temp_u32;
        for(int x = 0; x < 256; x++)
        {
            temp_u8 = SBox[x ^ expandedKey[16 * 0 + j]];
            temp_u32 = Tyi[j % 4][temp_u8];
            temp_u32 = MatMulNumM32(MB_to[0][columnindex[j]], temp_u32);
            TypeII_to[0][j][x] = (nibble[(temp_u32 & 0xf0000000) >> 28] << 28) | (nibble[(temp_u32 & 0x0f000000) >> 24] << 24) | (nibble[(temp_u32 & 0x00f00000) >> 20] << 20) | (nibble[(temp_u32 & 0x000f0000) >> 16] << 16) | (nibble[(temp_u32 & 0x0000f000) >> 12] << 12) | (nibble[(temp_u32 & 0x00000f00) >> 8] << 8) | (nibble[(temp_u32 & 0x000000f0) >> 4] << 4) | (nibble[(temp_u32 & 0x0000000f)]);
        }
    }
    for(int j = 0; j < 16; j++)//type_III
    {
        u8 temp_u8;
        u32 temp_u32;
        int shiftbit[] = {24, 16, 8, 0};
        for(int x = 0; x < 256; x++)
        {
            temp_u8 = x;
            temp_u8 = (nibble_inv[(temp_u8 & 0xf0) >> 4] << 4) | (nibble_inv[(temp_u8 & 0x0f)]); 
            temp_u32 = temp_u8;
            temp_u32 = temp_u32 << shiftbit[j % 4];
            temp_u32 = MatMulNumM32(MB_inv_to[0][columnindex[j]], temp_u32);
            temp_u32 = MatMulNumM32(Out_L_to[0][columnindex[j]], temp_u32);
            TypeIII_to[0][j][x] = (nibble[(temp_u32 & 0xf0000000) >> 28] << 28) | (nibble[(temp_u32 & 0x0f000000) >> 24] << 24) | (nibble[(temp_u32 & 0x00f00000) >> 20] << 20) | (nibble[(temp_u32 & 0x000f0000) >> 16] << 16) | (nibble[(temp_u32 & 0x0000f000) >> 12] << 12) | (nibble[(temp_u32 & 0x00000f00) >> 8] << 8) | (nibble[(temp_u32 & 0x000000f0) >> 4] << 4) | (nibble[(temp_u32 & 0x0000000f)]);
        }
    }

    //Round 2-6
    int shiftindex[] = {0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12, 1, 6, 11};
    for (int i = 1; i < 6; i++)//Type_II
    {
        shiftRows (expandedKey + 16 * i);
        for(int j = 0; j < 16; j++)
        {
            u8 temp_u8;
            u32 temp_u32;
            for(int x = 0; x < 256; x++)
            {
                temp_u8 = x;
                temp_u8 = (nibble_inv[(temp_u8 & 0xf0) >> 4] << 4) | (nibble_inv[(temp_u8 & 0x0f)]);
                temp_u8 = MatMulNumM8(L_inv_to[i - 1][shiftindex[j]], temp_u8);
                temp_u8 = SBox[temp_u8 ^ expandedKey[16 * i + j]];
                temp_u32 = Tyi[j % 4][temp_u8];
                temp_u32 = MatMulNumM32(MB_to[i][columnindex[j]], temp_u32);
                TypeII_to[i][j][x] = (nibble[(temp_u32 & 0xf0000000) >> 28] << 28) | (nibble[(temp_u32 & 0x0f000000) >> 24] << 24) | (nibble[(temp_u32 & 0x00f00000) >> 20] << 20) | (nibble[(temp_u32 & 0x000f0000) >> 16] << 16) | (nibble[(temp_u32 & 0x0000f000) >> 12] << 12) | (nibble[(temp_u32 & 0x00000f00) >> 8] << 8) | (nibble[(temp_u32 & 0x000000f0) >> 4] << 4) | (nibble[(temp_u32 & 0x0000000f)]);
            }
        }
    
        for(int j = 0; j < 16; j++)//type_III
        {
            u8 temp_u8;
            u32 temp_u32;
            int shiftbit[] = {24, 16, 8, 0};
            for(int x = 0; x < 256; x++)
            {
                temp_u8 = x;
                temp_u8 = (nibble_inv[(temp_u8 & 0xf0) >> 4] << 4) | (nibble_inv[(temp_u8 & 0x0f)]);
                temp_u32 = temp_u8;
                temp_u32 = temp_u32 << shiftbit[j % 4];
                temp_u32 = MatMulNumM32(MB_inv_to[i][columnindex[j]], temp_u32);
                temp_u32 = MatMulNumM32(Out_L_to[i][columnindex[j]], temp_u32);
                TypeIII_to[i][j][x] = (nibble[(temp_u32 & 0xf0000000) >> 28] << 28) | (nibble[(temp_u32 & 0x0f000000) >> 24] << 24) | (nibble[(temp_u32 & 0x00f00000) >> 20] << 20) | (nibble[(temp_u32 & 0x000f0000) >> 16] << 16) | (nibble[(temp_u32 & 0x0000f000) >> 12] << 12) | (nibble[(temp_u32 & 0x00000f00) >> 8] << 8) | (nibble[(temp_u32 & 0x000000f0) >> 4] << 4) | (nibble[(temp_u32 & 0x0000000f)]);
            }
        }
    }

    //left Round 7
    shiftRows (expandedKey + 16 * 6);
    for(int j = 0; j < 16; j++)//type_II
    {
        u8 temp_u8;
        u32 temp_u32;
        for(int x = 0; x < 256; x++)
        {
            temp_u8 = x;
            temp_u8 = (nibble_inv[(temp_u8 & 0xf0) >> 4] << 4) | (nibble_inv[(temp_u8 & 0x0f)]);
            temp_u8 = MatMulNumM8(L_inv_to[5][shiftindex[j]], temp_u8);
            temp_u8 = SBox[temp_u8 ^ expandedKey[16 * 6 + j]];
            temp_u32 = Tyi[j % 4][temp_u8];
            temp_u32 = MatMulNumM32(MB_left[0][columnindex[j]], temp_u32);
            TypeII_left[0][j][x] = (nibble[(temp_u32 & 0xf0000000) >> 28] << 28) | (nibble[(temp_u32 & 0x0f000000) >> 24] << 24) | (nibble[(temp_u32 & 0x00f00000) >> 20] << 20) | (nibble[(temp_u32 & 0x000f0000) >> 16] << 16) | (nibble[(temp_u32 & 0x0000f000) >> 12] << 12) | (nibble[(temp_u32 & 0x00000f00) >> 8] << 8) | (nibble[(temp_u32 & 0x000000f0) >> 4] << 4) | (nibble[(temp_u32 & 0x0000000f)]);
        }
    }

    for(int j = 0; j < 16; j++)//type_III
    {
        u8 temp_u8;
        u32 temp_u32;
        int shiftbit[] = {24, 16, 8, 0};
        for(int x = 0; x < 256; x++)
        {
            temp_u8 = x;
            temp_u8 = (nibble_inv[(temp_u8 & 0xf0) >> 4] << 4) | (nibble_inv[(temp_u8 & 0x0f)]);
            temp_u32 = temp_u8;
            temp_u32 = temp_u32 << shiftbit[j % 4];
            temp_u32 = MatMulNumM32(MB_inv_left[0][columnindex[j]], temp_u32);
            temp_u32 = MatMulNumM32(Out_L_left[0][columnindex[j]], temp_u32);
            TypeIII_left[0][j][x] = (nibble[(temp_u32 & 0xf0000000) >> 28] << 28) | (nibble[(temp_u32 & 0x0f000000) >> 24] << 24) | (nibble[(temp_u32 & 0x00f00000) >> 20] << 20) | (nibble[(temp_u32 & 0x000f0000) >> 16] << 16) | (nibble[(temp_u32 & 0x0000f000) >> 12] << 12) | (nibble[(temp_u32 & 0x00000f00) >> 8] << 8) | (nibble[(temp_u32 & 0x000000f0) >> 4] << 4) | (nibble[(temp_u32 & 0x0000000f)]);
        }
    }

    //right Round 7
    for(int j = 0; j < 16; j++)//type_II
    {
        u8 temp_u8;
        u32 temp_u32;
        for(int x = 0; x < 256; x++)
        {
            temp_u8 = x;
            temp_u8 = (nibble_inv[(temp_u8 & 0xf0) >> 4] << 4) | (nibble_inv[(temp_u8 & 0x0f)]);
            temp_u8 = MatMulNumM8(L_inv_to[5][shiftindex[j]], temp_u8);
            temp_u8 = SBox[temp_u8 ^ expandedKey[16 * 6 + j]];
            temp_u32 = Tyi[j % 4][temp_u8];
            temp_u32 = MatMulNumM32(MB_right[0][columnindex[j]], temp_u32);
            TypeII_right[0][j][x] = (nibble[(temp_u32 & 0xf0000000) >> 28] << 28) | (nibble[(temp_u32 & 0x0f000000) >> 24] << 24) | (nibble[(temp_u32 & 0x00f00000) >> 20] << 20) | (nibble[(temp_u32 & 0x000f0000) >> 16] << 16) | (nibble[(temp_u32 & 0x0000f000) >> 12] << 12) | (nibble[(temp_u32 & 0x00000f00) >> 8] << 8) | (nibble[(temp_u32 & 0x000000f0) >> 4] << 4) | (nibble[(temp_u32 & 0x0000000f)]);
        }
    }

    for(int j = 0; j < 16; j++)//type_III
    {
        u8 temp_u8;
        u32 temp_u32;
        int shiftbit[] = {24, 16, 8, 0};
        for(int x = 0; x < 256; x++)
        {
            temp_u8 = x;
            temp_u8 = (nibble_inv[(temp_u8 & 0xf0) >> 4] << 4) | (nibble_inv[(temp_u8 & 0x0f)]);
            temp_u32 = temp_u8;
            temp_u32 = temp_u32 << shiftbit[j % 4];
            temp_u32 = MatMulNumM32(MB_inv_right[0][columnindex[j]], temp_u32);
            temp_u32 = MatMulNumM32(Out_L_right[0][columnindex[j]], temp_u32);
            TypeIII_right[0][j][x] = (nibble[(temp_u32 & 0xf0000000) >> 28] << 28) | (nibble[(temp_u32 & 0x0f000000) >> 24] << 24) | (nibble[(temp_u32 & 0x00f00000) >> 20] << 20) | (nibble[(temp_u32 & 0x000f0000) >> 16] << 16) | (nibble[(temp_u32 & 0x0000f000) >> 12] << 12) | (nibble[(temp_u32 & 0x00000f00) >> 8] << 8) | (nibble[(temp_u32 & 0x000000f0) >> 4] << 4) | (nibble[(temp_u32 & 0x0000000f)]);
        }
    }

    //left Round 8
    shiftRows (expandedKey + 16 * 7);
    for(int j = 0; j < 16; j++)//type_II
    {
        u8 temp_u8;
        u32 temp_u32;
        for(int x = 0; x < 256; x++)
        {
            temp_u8 = x;
            temp_u8 = (nibble_inv[(temp_u8 & 0xf0) >> 4] << 4) | (nibble_inv[(temp_u8 & 0x0f)]);
            temp_u8 = MatMulNumM8(L_inv_left[0][shiftindex[j]], temp_u8);
            temp_u8 = SBox[temp_u8 ^ expandedKey[16 * 7 + j]];
            temp_u32 = Tyi[j % 4][temp_u8];
            temp_u32 = MatMulNumM32(MB_left[1][columnindex[j]], temp_u32);
            TypeII_left[1][j][x] = (nibble[(temp_u32 & 0xf0000000) >> 28] << 28) | (nibble[(temp_u32 & 0x0f000000) >> 24] << 24) | (nibble[(temp_u32 & 0x00f00000) >> 20] << 20) | (nibble[(temp_u32 & 0x000f0000) >> 16] << 16) | (nibble[(temp_u32 & 0x0000f000) >> 12] << 12) | (nibble[(temp_u32 & 0x00000f00) >> 8] << 8) | (nibble[(temp_u32 & 0x000000f0) >> 4] << 4) | (nibble[(temp_u32 & 0x0000000f)]);
        }
    }

    for(int j = 0; j < 16; j++)//type_III
    {
        u8 temp_u8;
        u32 temp_u32;
        int shiftbit[]={24, 16, 8, 0};
        for(int x = 0; x < 256; x++)
        {
            temp_u8 = x;
            temp_u8 = (nibble_inv[(temp_u8 & 0xf0) >> 4] << 4) | (nibble_inv[(temp_u8 & 0x0f)]);
            temp_u32 = temp_u8;
            temp_u32 = temp_u32 << shiftbit[j % 4];
            temp_u32 = MatMulNumM32(MB_inv_left[1][columnindex[j]], temp_u32);
            temp_u32 = MatMulNumM32(Out_L_left[1][columnindex[j]], temp_u32);
            TypeIII_left[1][j][x] = (nibble[(temp_u32 & 0xf0000000) >> 28] << 28) | (nibble[(temp_u32 & 0x0f000000) >> 24] << 24) | (nibble[(temp_u32 & 0x00f00000) >> 20] << 20) | (nibble[(temp_u32 & 0x000f0000) >> 16] << 16) | (nibble[(temp_u32 & 0x0000f000) >> 12] << 12) | (nibble[(temp_u32 & 0x00000f00) >> 8] << 8) | (nibble[(temp_u32 & 0x000000f0) >> 4] << 4) | (nibble[(temp_u32 & 0x0000000f)]);
        }
    }

    //right Round 8
    for(int j = 0; j < 16; j++)//type_II
    {
        u8 temp_u8;
        u32 temp_u32;
        for(int x = 0; x < 256; x++)
        {
            temp_u8 = x;
            temp_u8 = (nibble_inv[(temp_u8 & 0xf0) >> 4] << 4) | (nibble_inv[(temp_u8 & 0x0f)]);
            temp_u8 = MatMulNumM8(L_inv_right[0][shiftindex[j]], temp_u8);
            temp_u8 = SBox[temp_u8 ^ expandedKey[16 * 7 + j]];
            temp_u32 = Tyi[j % 4][temp_u8];
            temp_u32 = MatMulNumM32(MB_right[1][columnindex[j]], temp_u32);
            TypeII_right[1][j][x] = (nibble[(temp_u32 & 0xf0000000) >> 28] << 28) | (nibble[(temp_u32 & 0x0f000000) >> 24] << 24) | (nibble[(temp_u32 & 0x00f00000) >> 20] << 20) | (nibble[(temp_u32 & 0x000f0000) >> 16] << 16) | (nibble[(temp_u32 & 0x0000f000) >> 12] << 12) | (nibble[(temp_u32 & 0x00000f00) >> 8] << 8) | (nibble[(temp_u32 & 0x000000f0) >> 4] << 4) | (nibble[(temp_u32 & 0x0000000f)]);
        }
    }

    for(int j = 0; j < 16; j++)//type_III
    {
        u8 temp_u8;
        u32 temp_u32;
        int shiftbit[] = {24, 16, 8, 0};
        for(int x = 0; x < 256; x++)
        {
            temp_u8 = x;
            temp_u8 = (nibble_inv[(temp_u8 & 0xf0) >> 4] << 4) | (nibble_inv[(temp_u8 & 0x0f)]);
            temp_u32 = temp_u8;
            temp_u32 = temp_u32 << shiftbit[j % 4];
            temp_u32 = MatMulNumM32(MB_inv_right[1][columnindex[j]], temp_u32);
            temp_u32 = MatMulNumM32(Out_L_right[1][columnindex[j]], temp_u32);
            TypeIII_right[1][j][x] = (nibble[(temp_u32 & 0xf0000000) >> 28] << 28) | (nibble[(temp_u32 & 0x0f000000) >> 24] << 24) | (nibble[(temp_u32 & 0x00f00000) >> 20] << 20) | (nibble[(temp_u32 & 0x000f0000) >> 16] << 16) | (nibble[(temp_u32 & 0x0000f000) >> 12] << 12) | (nibble[(temp_u32 & 0x00000f00) >> 8] << 8) | (nibble[(temp_u32 & 0x000000f0) >> 4] << 4) | (nibble[(temp_u32 & 0x0000000f)]);
        }
    }

    //left Round 9
    shiftRows (expandedKey + 16 * 8);
    for(int j = 0; j < 16; j++)//type_II
    {
        u8 temp_u8;
        u32 temp_u32;
        for(int x = 0; x < 256; x++)
        {
            temp_u8 = x;
            temp_u8 = (nibble_inv[(temp_u8 & 0xf0) >> 4] << 4) | (nibble_inv[(temp_u8 & 0x0f)]);
            temp_u8 = MatMulNumM8(L_inv_left[1][shiftindex[j]], temp_u8);
            temp_u8 = SBox[temp_u8 ^ expandedKey[16 * 8 + j]];
            temp_u32 = Tyi[j % 4][temp_u8];
            temp_u32 = MatMulNumM32(MB_left[2][columnindex[j]], temp_u32);
            TypeII_left[2][j][x] = (nibble[(temp_u32 & 0xf0000000) >> 28] << 28) | (nibble[(temp_u32 & 0x0f000000) >> 24] << 24) | (nibble[(temp_u32 & 0x00f00000) >> 20] << 20) | (nibble[(temp_u32 & 0x000f0000) >> 16] << 16) | (nibble[(temp_u32 & 0x0000f000) >> 12] << 12) | (nibble[(temp_u32 & 0x00000f00) >> 8] << 8) | (nibble[(temp_u32 & 0x000000f0) >> 4] << 4) | (nibble[(temp_u32 & 0x0000000f)]);
        }
    }

    //right Round 9
    for(int j = 0; j < 16; j++)//type_II
    {
        u8 temp_u8;
        u32 temp_u32;
        for(int x = 0; x < 256; x++)
        {
            temp_u8 = x;
            temp_u8 = (nibble_inv[(temp_u8 & 0xf0) >> 4] << 4) | (nibble_inv[(temp_u8 & 0x0f)]);
            temp_u8 = MatMulNumM8(L_inv_right[1][shiftindex[j]], temp_u8);
            temp_u8 = SBox[temp_u8 ^ expandedKey[16 * 8 + j]];
            temp_u32 = Tyi[j % 4][temp_u8];
            temp_u32 = MatMulNumM32(MB_right[2][columnindex[j]], temp_u32);
            TypeII_right[2][j][x] = (nibble[(temp_u32 & 0xf0000000) >> 28] << 28) | (nibble[(temp_u32 & 0x0f000000) >> 24] << 24) | (nibble[(temp_u32 & 0x00f00000) >> 20] << 20) | (nibble[(temp_u32 & 0x000f0000) >> 16] << 16) | (nibble[(temp_u32 & 0x0000f000) >> 12] << 12) | (nibble[(temp_u32 & 0x00000f00) >> 8] << 8) | (nibble[(temp_u32 & 0x000000f0) >> 4] << 4) | (nibble[(temp_u32 & 0x0000000f)]);
        }
    }

    //final Round 9
    for(int j = 0; j < 16; j++)//type_III
    {
        u8 temp_u8;
        u32 temp_u32;
        int shiftbit[] = {24, 16, 8, 0};
        for(int x = 0; x < 256; x++)
        {
            temp_u8 = x;
            temp_u8 = (nibble_inv[(temp_u8 & 0xf0) >> 4] << 4) | (nibble_inv[(temp_u8 & 0x0f)]);
            temp_u32 = temp_u8;
            temp_u32 = temp_u32 << shiftbit[j % 4];
            temp_u32 = MatMulNumM32(MB_inv_final[columnindex[j]], temp_u32);
            temp_u32 = MatMulNumM32(Out_L_final[columnindex[j]], temp_u32);
            TypeIII_final[j][x] = (nibble[(temp_u32 & 0xf0000000) >> 28] << 28) | (nibble[(temp_u32 & 0x0f000000) >> 24] << 24) | (nibble[(temp_u32 & 0x00f00000) >> 20] << 20) | (nibble[(temp_u32 & 0x000f0000) >> 16] << 16) | (nibble[(temp_u32 & 0x0000f000) >> 12] << 12) | (nibble[(temp_u32 & 0x00000f00) >> 8] << 8) | (nibble[(temp_u32 & 0x000000f0) >> 4] << 4) | (nibble[(temp_u32 & 0x0000000f)]);
        }
    }

    //Round 10
    shiftRows (expandedKey + 16 * 9);
    for(int j = 0; j < 16; j++)//type_II
    {
        u8 temp_u8;
        for(int x = 0; x < 256; x++)
        {
            temp_u8 = x;
            temp_u8 = (nibble_inv[(temp_u8 & 0xf0) >> 4] << 4) | (nibble_inv[(temp_u8 & 0x0f)]);
            temp_u8 = MatMulNumM8(L_inv_final[shiftindex[j]], temp_u8);
            temp_u8 = SBox[temp_u8 ^ expandedKey[16 * 9 + j]];
            TypeII_final[j][x] = temp_u8 ^ expandedKey[16 * 10 + j];
        }
    }

    for (int x = 0; x < 16; x++)
    {
        for (int y = 0; y < 16; y++)
        {
            TypeIV[x][y] = nibble[nibble_inv[x] ^ nibble_inv[y]];
        }
    }
}
void wbaes_encrypt(u8 input[16], u8 output[16]) 
{
    u32 a, b, c, d, aa, bb, cc, dd;
    u8 state[16];
    for(int i = 0; i < 16; i++)
    {
        state[i] = input[i];
    }

    //Round 1-6
    for (int i = 0; i < 6; i++) 
    {
        shiftRows (state);

        for (int j = 0; j < 4; j++)
        {
            a = TypeII_to[i][4*j + 0][state[4*j + 0]];
            b = TypeII_to[i][4*j + 1][state[4*j + 1]];
            c = TypeII_to[i][4*j + 2][state[4*j + 2]];
            d = TypeII_to[i][4*j + 3][state[4*j + 3]];

            aa = TypeIV[(a >> 28) & 0xf][(b >> 28) & 0xf];
            bb = TypeIV[(c >> 28) & 0xf][(d >> 28) & 0xf];
            cc = TypeIV[(a >> 24) & 0xf][(b >> 24) & 0xf];
            dd = TypeIV[(c >> 24) & 0xf][(d >> 24) & 0xf];
            state[4*j + 0] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

            aa = TypeIV[(a >> 20) & 0xf][(b >> 20) & 0xf];
            bb = TypeIV[(c >> 20) & 0xf][(d >> 20) & 0xf];
            cc = TypeIV[(a >> 16) & 0xf][(b >> 16) & 0xf];
            dd = TypeIV[(c >> 16) & 0xf][(d >> 16) & 0xf];
            state[4*j + 1] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

            aa = TypeIV[(a >> 12) & 0xf][(b >> 12) & 0xf];
            bb = TypeIV[(c >> 12) & 0xf][(d >> 12) & 0xf];
            cc = TypeIV[(a >>  8) & 0xf][(b >>  8) & 0xf];
            dd = TypeIV[(c >>  8) & 0xf][(d >>  8) & 0xf];
            state[4*j + 2] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

            aa = TypeIV[(a >>  4) & 0xf][(b >>  4) & 0xf];
            bb = TypeIV[(c >>  4) & 0xf][(d >>  4) & 0xf];
            cc = TypeIV[(a >>  0) & 0xf][(b >>  0) & 0xf];
            dd = TypeIV[(c >>  0) & 0xf][(d >>  0) & 0xf];
            state[4*j + 3] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];


            a = TypeIII_to[i][4*j + 0][state[4*j + 0]];
            b = TypeIII_to[i][4*j + 1][state[4*j + 1]];
            c = TypeIII_to[i][4*j + 2][state[4*j + 2]];
            d = TypeIII_to[i][4*j + 3][state[4*j + 3]];

            aa = TypeIV[(a >> 28) & 0xf][(b >> 28) & 0xf];
            bb = TypeIV[(c >> 28) & 0xf][(d >> 28) & 0xf];
            cc = TypeIV[(a >> 24) & 0xf][(b >> 24) & 0xf];
            dd = TypeIV[(c >> 24) & 0xf][(d >> 24) & 0xf];
            state[4*j + 0] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

            aa = TypeIV[(a >> 20) & 0xf][(b >> 20) & 0xf];
            bb = TypeIV[(c >> 20) & 0xf][(d >> 20) & 0xf];
            cc = TypeIV[(a >> 16) & 0xf][(b >> 16) & 0xf];
            dd = TypeIV[(c >> 16) & 0xf][(d >> 16) & 0xf];
            state[4*j + 1] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

            aa = TypeIV[(a >> 12) & 0xf][(b >> 12) & 0xf];
            bb = TypeIV[(c >> 12) & 0xf][(d >> 12) & 0xf];
            cc = TypeIV[(a >>  8) & 0xf][(b >>  8) & 0xf];
            dd = TypeIV[(c >>  8) & 0xf][(d >>  8) & 0xf];
            state[4*j + 2] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

            aa = TypeIV[(a >>  4) & 0xf][(b >>  4) & 0xf];
            bb = TypeIV[(c >>  4) & 0xf][(d >>  4) & 0xf];
            cc = TypeIV[(a >>  0) & 0xf][(b >>  0) & 0xf];
            dd = TypeIV[(c >>  0) & 0xf][(d >>  0) & 0xf];
            state[4*j + 3] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];
        }
    }
    u8 depart_left[16];
    u8 depart_right[16];
    for(int k = 0; k < 16; k++)
    {
        depart_left[k] = state[k];
        depart_right[k] = state[k];
    }
    //left Round 7-8
    for(int i = 0; i < 2; i++)
    {
        shiftRows (depart_left);
        for (int j = 0; j < 4; j++)
        {
        a = TypeII_left[i][4*j + 0][depart_left[4*j + 0]];
        b = TypeII_left[i][4*j + 1][depart_left[4*j + 1]];
        c = TypeII_left[i][4*j + 2][depart_left[4*j + 2]];
        d = TypeII_left[i][4*j + 3][depart_left[4*j + 3]];

        aa = TypeIV[(a >> 28) & 0xf][(b >> 28) & 0xf];
        bb = TypeIV[(c >> 28) & 0xf][(d >> 28) & 0xf];
        cc = TypeIV[(a >> 24) & 0xf][(b >> 24) & 0xf];
        dd = TypeIV[(c >> 24) & 0xf][(d >> 24) & 0xf];
        depart_left[4*j + 0] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

        aa = TypeIV[(a >> 20) & 0xf][(b >> 20) & 0xf];
        bb = TypeIV[(c >> 20) & 0xf][(d >> 20) & 0xf];
        cc = TypeIV[(a >> 16) & 0xf][(b >> 16) & 0xf];
        dd = TypeIV[(c >> 16) & 0xf][(d >> 16) & 0xf];
        depart_left[4*j + 1] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

        aa = TypeIV[(a >> 12) & 0xf][(b >> 12) & 0xf];
        bb = TypeIV[(c >> 12) & 0xf][(d >> 12) & 0xf];
        cc = TypeIV[(a >>  8) & 0xf][(b >>  8) & 0xf];
        dd = TypeIV[(c >>  8) & 0xf][(d >>  8) & 0xf];
        depart_left[4*j + 2] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

        aa = TypeIV[(a >>  4) & 0xf][(b >>  4) & 0xf];
        bb = TypeIV[(c >>  4) & 0xf][(d >>  4) & 0xf];
        cc = TypeIV[(a >>  0) & 0xf][(b >>  0) & 0xf];
        dd = TypeIV[(c >>  0) & 0xf][(d >>  0) & 0xf];
        depart_left[4*j + 3] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];


        a = TypeIII_left[i][4*j + 0][depart_left[4*j + 0]];
        b = TypeIII_left[i][4*j + 1][depart_left[4*j + 1]];
        c = TypeIII_left[i][4*j + 2][depart_left[4*j + 2]];
        d = TypeIII_left[i][4*j + 3][depart_left[4*j + 3]];

        aa = TypeIV[(a >> 28) & 0xf][(b >> 28) & 0xf];
        bb = TypeIV[(c >> 28) & 0xf][(d >> 28) & 0xf];
        cc = TypeIV[(a >> 24) & 0xf][(b >> 24) & 0xf];
        dd = TypeIV[(c >> 24) & 0xf][(d >> 24) & 0xf];
        depart_left[4*j + 0] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

        aa = TypeIV[(a >> 20) & 0xf][(b >> 20) & 0xf];
        bb = TypeIV[(c >> 20) & 0xf][(d >> 20) & 0xf];
        cc = TypeIV[(a >> 16) & 0xf][(b >> 16) & 0xf];
        dd = TypeIV[(c >> 16) & 0xf][(d >> 16) & 0xf];
        depart_left[4*j + 1] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

        aa = TypeIV[(a >> 12) & 0xf][(b >> 12) & 0xf];
        bb = TypeIV[(c >> 12) & 0xf][(d >> 12) & 0xf];
        cc = TypeIV[(a >>  8) & 0xf][(b >>  8) & 0xf];
        dd = TypeIV[(c >>  8) & 0xf][(d >>  8) & 0xf];
        depart_left[4*j + 2] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

        aa = TypeIV[(a >>  4) & 0xf][(b >>  4) & 0xf];
        bb = TypeIV[(c >>  4) & 0xf][(d >>  4) & 0xf];
        cc = TypeIV[(a >>  0) & 0xf][(b >>  0) & 0xf];
        dd = TypeIV[(c >>  0) & 0xf][(d >>  0) & 0xf];
        depart_left[4*j + 3] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];
        
        }
    }
    //left Round 9
    shiftRows (depart_left);
    for (int j = 0; j < 4; j++)
    {
        a = TypeII_left[2][4*j + 0][depart_left[4*j + 0]];
        b = TypeII_left[2][4*j + 1][depart_left[4*j + 1]];
        c = TypeII_left[2][4*j + 2][depart_left[4*j + 2]];
        d = TypeII_left[2][4*j + 3][depart_left[4*j + 3]];

        aa = TypeIV[(a >> 28) & 0xf][(b >> 28) & 0xf];
        bb = TypeIV[(c >> 28) & 0xf][(d >> 28) & 0xf];
        cc = TypeIV[(a >> 24) & 0xf][(b >> 24) & 0xf];
        dd = TypeIV[(c >> 24) & 0xf][(d >> 24) & 0xf];
        depart_left[4*j + 0] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

        aa = TypeIV[(a >> 20) & 0xf][(b >> 20) & 0xf];
        bb = TypeIV[(c >> 20) & 0xf][(d >> 20) & 0xf];
        cc = TypeIV[(a >> 16) & 0xf][(b >> 16) & 0xf];
        dd = TypeIV[(c >> 16) & 0xf][(d >> 16) & 0xf];
        depart_left[4*j + 1] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

        aa = TypeIV[(a >> 12) & 0xf][(b >> 12) & 0xf];
        bb = TypeIV[(c >> 12) & 0xf][(d >> 12) & 0xf];
        cc = TypeIV[(a >>  8) & 0xf][(b >>  8) & 0xf];
        dd = TypeIV[(c >>  8) & 0xf][(d >>  8) & 0xf];
        depart_left[4*j + 2] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

        aa = TypeIV[(a >>  4) & 0xf][(b >>  4) & 0xf];
        bb = TypeIV[(c >>  4) & 0xf][(d >>  4) & 0xf];
        cc = TypeIV[(a >>  0) & 0xf][(b >>  0) & 0xf];
        dd = TypeIV[(c >>  0) & 0xf][(d >>  0) & 0xf];
        depart_left[4*j + 3] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];
    }

    //right Round 7-8
    for(int i = 0; i < 2; i++)
    {
        shiftRows (depart_right);
        for (int j = 0; j < 4; j++)
        {
            a = TypeII_right[i][4*j + 0][depart_right[4*j + 0]];
            b = TypeII_right[i][4*j + 1][depart_right[4*j + 1]];
            c = TypeII_right[i][4*j + 2][depart_right[4*j + 2]];
            d = TypeII_right[i][4*j + 3][depart_right[4*j + 3]];

            aa = TypeIV[(a >> 28) & 0xf][(b >> 28) & 0xf];
            bb = TypeIV[(c >> 28) & 0xf][(d >> 28) & 0xf];
            cc = TypeIV[(a >> 24) & 0xf][(b >> 24) & 0xf];
            dd = TypeIV[(c >> 24) & 0xf][(d >> 24) & 0xf];
            depart_right[4*j + 0] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

            aa = TypeIV[(a >> 20) & 0xf][(b >> 20) & 0xf];
            bb = TypeIV[(c >> 20) & 0xf][(d >> 20) & 0xf];
            cc = TypeIV[(a >> 16) & 0xf][(b >> 16) & 0xf];
            dd = TypeIV[(c >> 16) & 0xf][(d >> 16) & 0xf];
            depart_right[4*j + 1] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

            aa = TypeIV[(a >> 12) & 0xf][(b >> 12) & 0xf];
            bb = TypeIV[(c >> 12) & 0xf][(d >> 12) & 0xf];
            cc = TypeIV[(a >>  8) & 0xf][(b >>  8) & 0xf];
            dd = TypeIV[(c >>  8) & 0xf][(d >>  8) & 0xf];
            depart_right[4*j + 2] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

            aa = TypeIV[(a >>  4) & 0xf][(b >>  4) & 0xf];
            bb = TypeIV[(c >>  4) & 0xf][(d >>  4) & 0xf];
            cc = TypeIV[(a >>  0) & 0xf][(b >>  0) & 0xf];
            dd = TypeIV[(c >>  0) & 0xf][(d >>  0) & 0xf];
            depart_right[4*j + 3] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];


            a = TypeIII_right[i][4*j + 0][depart_right[4*j + 0]];
            b = TypeIII_right[i][4*j + 1][depart_right[4*j + 1]];
            c = TypeIII_right[i][4*j + 2][depart_right[4*j + 2]];
            d = TypeIII_right[i][4*j + 3][depart_right[4*j + 3]];

            aa = TypeIV[(a >> 28) & 0xf][(b >> 28) & 0xf];
            bb = TypeIV[(c >> 28) & 0xf][(d >> 28) & 0xf];
            cc = TypeIV[(a >> 24) & 0xf][(b >> 24) & 0xf];
            dd = TypeIV[(c >> 24) & 0xf][(d >> 24) & 0xf];
            depart_right[4*j + 0] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

            aa = TypeIV[(a >> 20) & 0xf][(b >> 20) & 0xf];
            bb = TypeIV[(c >> 20) & 0xf][(d >> 20) & 0xf];
            cc = TypeIV[(a >> 16) & 0xf][(b >> 16) & 0xf];
            dd = TypeIV[(c >> 16) & 0xf][(d >> 16) & 0xf];
            depart_right[4*j + 1] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

            aa = TypeIV[(a >> 12) & 0xf][(b >> 12) & 0xf];
            bb = TypeIV[(c >> 12) & 0xf][(d >> 12) & 0xf];
            cc = TypeIV[(a >>  8) & 0xf][(b >>  8) & 0xf];
            dd = TypeIV[(c >>  8) & 0xf][(d >>  8) & 0xf];
            depart_right[4*j + 2] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

            aa = TypeIV[(a >>  4) & 0xf][(b >>  4) & 0xf];
            bb = TypeIV[(c >>  4) & 0xf][(d >>  4) & 0xf];
            cc = TypeIV[(a >>  0) & 0xf][(b >>  0) & 0xf];
            dd = TypeIV[(c >>  0) & 0xf][(d >>  0) & 0xf];
            depart_right[4*j + 3] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];
        }
    }
    //right Round 9
    shiftRows (depart_right);
    for (int j = 0; j < 4; j++)
    {
        a = TypeII_right[2][4*j + 0][depart_right[4*j + 0]];
        b = TypeII_right[2][4*j + 1][depart_right[4*j + 1]];
        c = TypeII_right[2][4*j + 2][depart_right[4*j + 2]];
        d = TypeII_right[2][4*j + 3][depart_right[4*j + 3]];

        aa = TypeIV[(a >> 28) & 0xf][(b >> 28) & 0xf];
        bb = TypeIV[(c >> 28) & 0xf][(d >> 28) & 0xf];
        cc = TypeIV[(a >> 24) & 0xf][(b >> 24) & 0xf];
        dd = TypeIV[(c >> 24) & 0xf][(d >> 24) & 0xf];
        depart_right[4*j + 0] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

        aa = TypeIV[(a >> 20) & 0xf][(b >> 20) & 0xf];
        bb = TypeIV[(c >> 20) & 0xf][(d >> 20) & 0xf];
        cc = TypeIV[(a >> 16) & 0xf][(b >> 16) & 0xf];
        dd = TypeIV[(c >> 16) & 0xf][(d >> 16) & 0xf];
        depart_right[4*j + 1] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

        aa = TypeIV[(a >> 12) & 0xf][(b >> 12) & 0xf];
        bb = TypeIV[(c >> 12) & 0xf][(d >> 12) & 0xf];
        cc = TypeIV[(a >>  8) & 0xf][(b >>  8) & 0xf];
        dd = TypeIV[(c >>  8) & 0xf][(d >>  8) & 0xf];
        depart_right[4*j + 2] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

        aa = TypeIV[(a >>  4) & 0xf][(b >>  4) & 0xf];
        bb = TypeIV[(c >>  4) & 0xf][(d >>  4) & 0xf];
        cc = TypeIV[(a >>  0) & 0xf][(b >>  0) & 0xf];
        dd = TypeIV[(c >>  0) & 0xf][(d >>  0) & 0xf];
        depart_right[4*j + 3] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];
    }

    //combine to Round 9 type III
    for(int j = 0; j < 4; j++)
    {
        aa = TypeIV[(depart_left[4*j + 0] >> 4) & 0xf][(depart_right[4*j + 0] >> 4) & 0xf];
        bb = TypeIV[(depart_left[4*j + 0] >> 0) & 0xf][(depart_right[4*j + 0] >> 0) & 0xf];
        state[4*j + 0] = (aa << 4) | bb;
        aa = TypeIV[(depart_left[4*j + 1] >> 4) & 0xf][(depart_right[4*j + 1] >> 4) & 0xf];
        bb = TypeIV[(depart_left[4*j + 1] >> 0) & 0xf][(depart_right[4*j + 1] >> 0) & 0xf];
        state[4*j + 1] = (aa << 4) | bb;
        aa = TypeIV[(depart_left[4*j + 2] >> 4) & 0xf][(depart_right[4*j + 2] >> 4) & 0xf];
        bb = TypeIV[(depart_left[4*j + 2] >> 0) & 0xf][(depart_right[4*j + 2] >> 0) & 0xf];
        state[4*j + 2] = (aa << 4) | bb;
        aa = TypeIV[(depart_left[4*j + 3] >> 4) & 0xf][(depart_right[4*j + 3] >> 4) & 0xf];
        bb = TypeIV[(depart_left[4*j + 3] >> 0) & 0xf][(depart_right[4*j + 3] >> 0) & 0xf];
        state[4*j + 3] = (aa << 4) | bb;
    }

    //Round 9 Type III
    for(int j = 0; j < 4; j++)
    {
        a = TypeIII_final[4*j + 0][state[4*j + 0]];
        b = TypeIII_final[4*j + 1][state[4*j + 1]];
        c = TypeIII_final[4*j + 2][state[4*j + 2]];
        d = TypeIII_final[4*j + 3][state[4*j + 3]];

        aa = TypeIV[(a >> 28) & 0xf][(b >> 28) & 0xf];
        bb = TypeIV[(c >> 28) & 0xf][(d >> 28) & 0xf];
        cc = TypeIV[(a >> 24) & 0xf][(b >> 24) & 0xf];
        dd = TypeIV[(c >> 24) & 0xf][(d >> 24) & 0xf];
        state[4*j + 0] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

        aa = TypeIV[(a >> 20) & 0xf][(b >> 20) & 0xf];
        bb = TypeIV[(c >> 20) & 0xf][(d >> 20) & 0xf];
        cc = TypeIV[(a >> 16) & 0xf][(b >> 16) & 0xf];
        dd = TypeIV[(c >> 16) & 0xf][(d >> 16) & 0xf];
        state[4*j + 1] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

        aa = TypeIV[(a >> 12) & 0xf][(b >> 12) & 0xf];
        bb = TypeIV[(c >> 12) & 0xf][(d >> 12) & 0xf];
        cc = TypeIV[(a >>  8) & 0xf][(b >>  8) & 0xf];
        dd = TypeIV[(c >>  8) & 0xf][(d >>  8) & 0xf];
        state[4*j + 2] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];

        aa = TypeIV[(a >>  4) & 0xf][(b >>  4) & 0xf];
        bb = TypeIV[(c >>  4) & 0xf][(d >>  4) & 0xf];
        cc = TypeIV[(a >>  0) & 0xf][(b >>  0) & 0xf];
        dd = TypeIV[(c >>  0) & 0xf][(d >>  0) & 0xf];
        state[4*j + 3] = (TypeIV[aa][bb] << 4) | TypeIV[cc][dd];
    }

    //Round 10
    shiftRows(state);
    for (int j = 0; j < 16; j++) 
    {
        state[j] = TypeII_final[j][state[j]];
    }

    for (int i = 0; i < 16; i++)
    {
        output[i] = state[i];
    }
}