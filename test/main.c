#include "genTables.h"

int main(int argc, char * argv[])
{
    unsigned char OUT[16];
    unsigned char OOUT[16];
    int i;
    u8 expandedKey[176];
    u8 key[16] = {0};
    uint64_t begin;
    uint64_t end;
    uint64_t ans = 0;
    
    expandKey (key, expandedKey);
    computeTables(expandedKey);
    
    unsigned char IN[16] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
    aes_128_table_encrypt(IN, OUT);
    printstate(OUT);
    
    unsigned char IIN[16] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
    aes_128_encrypt(IIN, key, OOUT);
    printstate(OOUT);

    return 0;
}
