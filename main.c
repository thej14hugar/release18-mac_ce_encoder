#include <stdio.h>
#include <stdint.h>
#include "encoder.h"

int main()
{
    uint8_t pdu[MAX_MAC_CE_SIZE] = {0};
    int pdu_size = 0;
    parse_and_encode("input.txt", pdu, &pdu_size);
    return 0;
}