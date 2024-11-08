
#include <stdint.h>

// https://www.embeddedrelated.com/showarticle/1065.php

typedef struct {
    int32_t state;
    int32_t taps;
    int32_t ymask;
} LFSR_T;

LFSR_T lfsr;

void lfsr_init(LFSR_T* plfsr)
{
    plfsr->state = 1;
    plfsr->taps = 0x0005; // a0 = 1, a2 = 1
    plfsr->ymask = 0x0010; // bit 4 is the highest significant state bit
}

/* Galois implementation of LFSR */
bool lfsr_step(LFSR_T* plfsr)
{
    bool out = (plfsr->state & plfsr->ymask) != 0;
    plfsr->state <<= 1;    // shift left
    if (out)
    {
        plfsr->state ^= plfsr->taps;
        /* flip bits connected to the taps if the output is 1.
           Note that the state bits beyond bit 4 are ignored */
    }
    return out;
}
