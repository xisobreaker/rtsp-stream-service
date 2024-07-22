#pragma once

#include <cstdint>

int          U(uint8_t *pBuf, uint32_t &nStartBit, uint32_t nBitSize);
unsigned int Ue(uint8_t *pBuf, uint32_t nLen, uint32_t &nStartBit);
int          Se(uint8_t *pBuf, uint32_t nLen, uint32_t &nStartBit);
void         de_emulation_prevention(uint8_t *pBuf, uint32_t &nLen);
