#include "rtp_utils.h"
#include <cmath>

int U(uint8_t *pBuf, uint32_t &nStartBit, uint32_t nBitSize)
{
    int nValue = 0;
    for (int i = 0; i < nBitSize; i++) {
        nValue = (nValue << 1) + ((pBuf[nStartBit / 8] & (0x80 >> (nStartBit % 8))) ? 1 : 0);
        nStartBit++;
    }
    return nValue;
}

unsigned int Ue(uint8_t *pBuf, uint32_t nLen, uint32_t &nStartBit)
{
    unsigned int nZeroNum = 0;
    for (int i = 0; i < nLen * 8; i++) {
        if (pBuf[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
            break;
        nZeroNum++;
        nStartBit++;
    }

    unsigned int nValue = 0;
    for (int i = 0; i < nZeroNum + 1; i++) {
        nValue = (nValue << 1) + ((pBuf[nStartBit / 8] >> (7 - nStartBit % 8)) & 0x01);
        nStartBit++;
    }
    return nValue - 1;
}

int Se(uint8_t *pBuf, uint32_t nLen, uint32_t &nStartBit)
{
    int    UeVal  = Ue(pBuf, nLen, nStartBit);
    double k      = UeVal;
    int    nValue = ceil(k / 2);
    if (UeVal % 2 == 0)
        nValue = -nValue;
    return nValue;
}

void de_emulation_prevention(uint8_t *pBuf, uint32_t &nLen)
{
    int s = 0, e = 0;
    for (int i = 0; i < nLen - 2; i++) {
        if (0 == (pBuf[i] ^ 0x00) + (pBuf[i + 1] ^ 0x00) + (pBuf[i + 2] ^ 0x03)) {
            for (int j = i + 2; j < nLen - 1; j++) {
                pBuf[j] = pBuf[j + 1];
            }
            nLen -= 1;
        }
    }
}