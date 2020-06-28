#include "genTables.h"
#ifdef __GNUC__
#include <x86intrin.h>
#endif
#ifdef _MSC_VER
#include <intrin.h>
#endif
#pragma intrinsic(__rdtsc)

//Repeat test times and calculate on average for accuracy
#define TEST 100

//CPU cycles set start;
uint64_t start_rdtsc()
{
    return __rdtsc();
}

//CPU cycles set end;
uint64_t end_rdtsc()
{
    return __rdtsc();
}

char ascii2hex(char in){
    char out;

    if (('0' <= in) && (in <= '9'))
        out = in - '0';

    if (('A' <= in) && (in <= 'F'))
        out = in - 'A' + 10;

    if (('a' <= in) && (in <= 'f'))
        out = in - 'a' + 10;

    return out;
}

void asciiStr2hex (char * in, char * out, int len){
    int j = 0;
    for (int i = 0; i < len; i += 2)
        out[j++]  = (ascii2hex(in[i ]) << 4) +  ascii2hex(in[i+1]);
}

int main(int argc, char * argv[])
{
        unsigned char OUT[16];
        unsigned char OOUT[16];
        //unsigned char IN[32];
        int i;
        u8 expandedKey[176];
        u8 key[16] = {0};
        uint64_t begin;
        uint64_t end;
        uint64_t ans = 0;
        
        //printf("Generating tables...\n");
        expandKey (key, expandedKey);

        //printf("WBAES-WBMatrix performance test:\n");
        //begin = start_rdtsc();
        //for (i = 0; i < TEST; i++)
        {
            computeTables(expandedKey);
        }
        //end = end_rdtsc();
        //ans = (end - begin);
        //printf("WBAES generate tables cost %llu CPU cycles\n", ans/TEST);

        //asciiStr2hex(argv[1], (char *)IN, 32);
        
        unsigned char IN[16] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
        //unsigned char IN[16] = {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf};
        //printstate(IN);
        aes_128_table_encrypt(IN, OUT);
        printstate(OUT);
        
        unsigned char IIN[16] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
        //unsigned char IIN[16] = {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf};
        aes_128_encrypt(IIN, OOUT);
        printstate(OOUT);

        return 0;
}
