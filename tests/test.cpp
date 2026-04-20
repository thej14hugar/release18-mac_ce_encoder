#include <gtest/gtest.h>
#include <string>
#include <stdio.h>

extern "C"
{
#include "encoder.h"
}

/******************************************
Common fixture
*******************************************/
class MACTest : public ::testing::Test
{
protected:
    uint8_t pdu[10];
    int offset;
    Flags flags;
    EncoderState state;

    void SetUp() override
    {
        memset(pdu, 0, sizeof(pdu));
        flags = {1, 0, 1, 0, 0, 0};
        offset = 0;
        state = {0};
    }
};
class ParseTest : public ::testing::Test
{
};

/******************************************
 * TEST CASES FOR SHORT _BSR
 *******************************************/
// Valid case
TEST_F(MACTest, ShortBSR_ValidInput)
{
    uint8_t temp_pdu[10] = {0};
    int lcgid = 3;
    int buffer = 10;

    temp_pdu[0] = LCID_SHORT_BSR;
    temp_pdu[1] = (lcgid << 5) | buffer;

    int result = short_bsr(pdu, &offset, 2, lcgid, buffer, 2);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[0], temp_pdu[0]);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
}

// Missing buffer
TEST_F(MACTest, ShortBSR_MissingBuffer)
{
    int result = short_bsr(pdu, &offset, 1, 3, -1, 2);
    EXPECT_EQ(result, FAILURE);
}

// Missing LCGID
TEST_F(MACTest, ShortBSR_MissingLCGID)
{
    // SetUp();
    int lcgid = 0;
    int buffsize = 0;
    uint8_t temp_pdu[10] = {0};
    temp_pdu[0] = 0x3d;
    for (int i = 0; i < 255; i++)
    {

        int result = short_bsr(pdu, &offset, 2, lcgid, buffsize, 2);
        // uint8_t temp_pdu[]={0x3d, 0x00, 0x3d, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        temp_pdu[1] = (lcgid << 5) | buffsize;
        EXPECT_EQ(result, SUCCESS);
        EXPECT_EQ(pdu[0], temp_pdu[0]);
        EXPECT_EQ(pdu[1], temp_pdu[1]);
        buffsize++;
        if (buffsize > 31)
        {
            buffsize = 0;
            lcgid++;
        }
        memset(pdu, 0, sizeof(pdu));
        offset = 0;
    }
}

// Both parameters missing
TEST_F(MACTest, ShortBSR_BothMissing)
{
    int result = short_bsr(pdu, &offset, 0, -1, -1, 2);
    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0); // buffer unchanged
    EXPECT_EQ(pdu[1], 0);
}

// Extra parameters
TEST_F(MACTest, ShortBSR_ExtraParameters)
{
    int result = short_bsr(pdu, &offset, 3, 3, 10, 2);

    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0); //  buffer unchanged
    EXPECT_EQ(pdu[1], 0);
}

// Invalid LCGID (out of range)
TEST_F(MACTest, ShortBSR_InvalidLCGIDRange)
{
    int result = short_bsr(pdu, &offset, 2, 10, 10, 2);

    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0); //  buffer unchanged
    EXPECT_EQ(pdu[1], 0);
}

// Invalid buffer (out of range)
TEST_F(MACTest, ShortBSR_InvalidBufferRange)
{
    int result = short_bsr(pdu, &offset, 2, 3, 40, 2);

    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0); //  buffer unchanged
    EXPECT_EQ(pdu[1], 0);
}

//  Negative values
TEST_F(MACTest, ShortBSR_NegativeValues)
{
    int result = short_bsr(pdu, &offset, 2, 1, -5, 2);

    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0); //  buffer unchanged
    EXPECT_EQ(pdu[1], 0);
}

// LCGID value missing (lcgid=)
TEST_F(MACTest, ShortBSR_LCGID_ValueMissing)
{
    int result = short_bsr(pdu, &offset, 1, -1, 10, 2);

    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0); //  buffer unchanged
    EXPECT_EQ(pdu[1], 0);
}

//  Boundary max
TEST_F(MACTest, ShortBSR_MaxBoundaryValues)
{
    uint8_t temp_pdu[10] = {0};
    int lcgid = 7;
    int buffer = 31;

    temp_pdu[0] = LCID_SHORT_BSR;
    temp_pdu[1] = (lcgid << 5) | buffer;

    int result = short_bsr(pdu, &offset, 2, lcgid, buffer, 2);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[0], temp_pdu[0]);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
}

//  Boundary min
TEST_F(MACTest, ShortBSR_MinBoundaryValues)
{
    uint8_t temp_pdu[10] = {0};
    int lcgid = 0;
    int buffer = 0;

    temp_pdu[0] = LCID_SHORT_BSR;
    temp_pdu[1] = (lcgid << 5) | buffer;

    int result = short_bsr(pdu, &offset, 2, lcgid, buffer, 2);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[0], temp_pdu[0]);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
}

TEST_F(ParseTest, ShortBSR_LCGID_ValueMissing)
{
    FILE *fp = fopen("test_input.txt", "w");

    fprintf(fp,
            "Total pdu_size 30\n"
            "num_ce 1\n"
            "<short_bsr>\n"
            "lcgid=1\n"
            "buffer=\n");

    fclose(fp);

    uint8_t pdu[100] = {0};
    int size = 0;

    int result = parse_and_encode("test_input.txt", pdu, &size);
    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0);
}

/******************************************
 * TEST CASES FOR PHR
 *******************************************/
// Valid case
TEST_F(MACTest, PHR_ValidInput)
{
    uint8_t temp_pdu[10] = {0};
    int ph = 20;
    int pcmax = 15;

    // Expected encoding
    temp_pdu[0] = LCID_PHR;
    temp_pdu[1] = (flags.P << 7) | (flags.R << 6) | ph;
    temp_pdu[2] = (flags.MPE << 7) | (flags.R2 << 6) | pcmax;

    int result = phr(pdu, &offset, 2, ph, pcmax, flags, 3);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[0], temp_pdu[0]);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
    EXPECT_EQ(pdu[2], temp_pdu[2]);
}

// Missing PH
TEST_F(MACTest, PHR_MissingPH)
{
    int result = phr(pdu, &offset, 1, -1, 10, flags, 3);
    EXPECT_EQ(result, FAILURE);
}

// Missing PCMAX
TEST_F(MACTest, PHR_MissingPCMAX)
{
    int result = phr(pdu, &offset, 1, 20, -1, flags, 3);
    EXPECT_EQ(result, FAILURE);
}

// Both missing
TEST_F(MACTest, PHR_BothMissing)
{
    int result = phr(pdu, &offset, 0, -1, -1, flags, 3);
    EXPECT_EQ(result, FAILURE);
}

// Extra parameters
TEST_F(MACTest, PHR_ExtraParameters)
{
    int result = phr(pdu, &offset, 3, 20, 15, flags, 3);
    EXPECT_EQ(result, FAILURE);
}

// Invalid PH range
TEST_F(MACTest, PHR_InvalidPHRange)
{
    int result = phr(pdu, &offset, 2, 100, 10, flags, 3);
    EXPECT_EQ(result, FAILURE);
}

// Invalid PCMAX range
TEST_F(MACTest, PHR_InvalidPCMAXRange)
{
    int result = phr(pdu, &offset, 2, 20, 100, flags, 3);
    EXPECT_EQ(result, FAILURE);
}

// Negative values
TEST_F(MACTest, PHR_NegativeValues)
{
    int result = phr(pdu, &offset, 2, -5, -10, flags, 3);
    EXPECT_EQ(result, FAILURE);
}

// PH value missing (ph=)
TEST_F(MACTest, PHR_PH_ValueMissing)
{
    int result = phr(pdu, &offset, 1, -1, 10, flags, 3);
    EXPECT_EQ(result, FAILURE);
}

// PCMAX value missing (pcmax=)
TEST_F(MACTest, PHR_PCMAX_ValueMissing)
{

    int result = phr(pdu, &offset, 1, 20, -1, flags, 3);

    EXPECT_EQ(result, FAILURE);
}

// Boundary max
TEST_F(MACTest, PHR_MaxBoundaryValues)
{
    uint8_t temp_pdu[10] = {0};
    int ph = 63;
    int pcmax = 63;

    temp_pdu[0] = LCID_PHR;
    temp_pdu[1] = (flags.P << 7) | (flags.R << 6) | ph;
    temp_pdu[2] = (flags.MPE << 7) | (flags.R2 << 6) | pcmax;

    int result = phr(pdu, &offset, 2, ph, pcmax, flags, 3);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[0], temp_pdu[0]);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
    EXPECT_EQ(pdu[2], temp_pdu[2]);
}

// Boundary min
TEST_F(MACTest, PHR_MinBoundaryValues)
{
    uint8_t temp_pdu[10] = {0};
    int ph = 0;
    int pcmax = 0;

    temp_pdu[0] = LCID_PHR;
    temp_pdu[1] = (flags.P << 7) | (flags.R << 6) | ph;
    temp_pdu[2] = (flags.MPE << 7) | (flags.R2 << 6) | pcmax;

    int result = phr(pdu, &offset, 2, ph, pcmax, flags, 3);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[0], temp_pdu[0]);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
    EXPECT_EQ(pdu[2], temp_pdu[2]);
}

TEST_F(ParseTest, PHR_PH_ValueMissing)
{
    uint8_t pdu[100] = {0};
    int size = 0;
    FILE *fp = fopen("test_input.txt", "w");

    fprintf(fp,
            "Total pdu_size 30\n"
            "num_ce 1\n"
            "<phr>\n"
            "ph=1\n"
            "pcmax=\n");

    fclose(fp);

    int result = parse_and_encode("test_input.txt", pdu, &size);
    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0);
}

/******************************************
 * TEST CASES FOR CRNTI
 *******************************************/
// Valid case
TEST_F(MACTest, CRNTI_ValidInput)
{
    uint8_t temp_pdu[10] = {0};
    int value = 500;

    temp_pdu[0] = LCID_CRNTI;
    temp_pdu[1] = (value >> 8) & 0xFF;
    temp_pdu[2] = value & 0xFF;

    int result = crnti(pdu, &offset, 1, 500, 3);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[0], temp_pdu[0]);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
    EXPECT_EQ(pdu[2], temp_pdu[2]);
}

// Missing CRNTI
TEST_F(MACTest, CRNTI_MissingValue)
{
    int result = crnti(pdu, &offset, 0, -1, 3);

    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0);
    EXPECT_EQ(pdu[1], 0);
}

// Extra parameters
TEST_F(MACTest, CRNTI_ExtraParameters)
{
    int result = crnti(pdu, &offset, 2, 500, 3);

    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0);
}

// Invalid range (too large)
TEST_F(MACTest, CRNTI_InvalidRange)
{
    int result = crnti(pdu, &offset, 1, 70000, 3);

    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0);
}

// Negative value
TEST_F(MACTest, CRNTI_NegativeValue)
{
    int result = crnti(pdu, &offset, 1, -5, 3);

    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0);
}

// Boundary max
TEST_F(MACTest, CRNTI_MaxBoundary)
{
    uint8_t temp_pdu[10] = {0};
    int value = 65535;
    temp_pdu[0] = LCID_CRNTI;
    temp_pdu[1] = 0xFF;
    temp_pdu[2] = 0xFF;

    int result = crnti(pdu, &offset, 1, value, 3);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
    EXPECT_EQ(pdu[2], temp_pdu[2]);
}

// Boundary min
TEST_F(MACTest, CRNTI_MinBoundary)
{
    uint8_t temp_pdu[10] = {0};
    int value = 0;
    temp_pdu[0] = LCID_CRNTI;
    temp_pdu[1] = 0x00;
    temp_pdu[2] = 0x00;

    int result = crnti(pdu, &offset, 1, value, 3);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
    EXPECT_EQ(pdu[2], temp_pdu[2]);
}

TEST_F(ParseTest, CRNTI_ValueMissing)
{
    FILE *fp = fopen("test_input.txt", "w");

    fprintf(fp,
            "Total pdu_size 30\n"
            "num_ce 1\n"
            "<crnti>\n"
            "crnti=\n");

    fclose(fp);

    uint8_t pdu[100] = {0};
    int size = 0;

    int result = parse_and_encode("test_input.txt", pdu, &size);

    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0);
}

/******************************************
 * TEST CASES FOR SL-LBT
 *******************************************/
// Valid case
TEST_F(MACTest, SLLBT_ValidInput)
{
    uint8_t temp_pdu[10] = {0};
    int value = 10;

    temp_pdu[0] = LCID_EXT_1BYTE;
    temp_pdu[1] = ELCID_SL_LBT;
    temp_pdu[2] = value & 0x1F;

    int result = sl_lbt(pdu, &offset, 1, value, 3);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[0], temp_pdu[0]);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
    EXPECT_EQ(pdu[2], temp_pdu[2]);
}

// Missing value
TEST_F(MACTest, SLLBT_MissingValue)
{
    int result = sl_lbt(pdu, &offset, 0, -1, 3);

    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0);
}

// Extra parameters
TEST_F(MACTest, SLLBT_ExtraParameters)
{
    int result = sl_lbt(pdu, &offset, 2, 10, 3);

    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0);
}

// Invalid range (>255)
TEST_F(MACTest, SLLBT_InvalidRange)
{
    int result = sl_lbt(pdu, &offset, 1, 40, 3);

    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0);
}

// Negative value
TEST_F(MACTest, SLLBT_NegativeValue)
{
    int result = sl_lbt(pdu, &offset, 1, -5, 3);

    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0);
}

// Boundary max
TEST_F(MACTest, SLLBT_MaxBoundary)
{
    uint8_t temp_pdu[10] = {0};
    int value = 31;

    temp_pdu[0] = LCID_EXT_1BYTE;
    temp_pdu[1] = ELCID_SL_LBT;
    temp_pdu[2] = 31;

    int result = sl_lbt(pdu, &offset, 1, value, 3);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[0], temp_pdu[0]);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
    EXPECT_EQ(pdu[2], temp_pdu[2]);
}

// Boundary min
TEST_F(MACTest, SLLBT_MinBoundary)
{
    uint8_t temp_pdu[10] = {0};
    int value = 0;
    temp_pdu[0] = LCID_EXT_1BYTE;
    temp_pdu[1] = ELCID_SL_LBT;
    temp_pdu[2] = 0;

    int result = sl_lbt(pdu, &offset, 1, value, 3);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[0], temp_pdu[0]);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
    EXPECT_EQ(pdu[2], temp_pdu[2]);
}
TEST_F(ParseTest, SLLBT_ValueMissing)
{
    FILE *fp = fopen("test_input.txt", "w");

    fprintf(fp,
            "Total pdu_size 30\n"
            "num_ce 1\n"
            "<sl_lbt>\n"
            "value=\n");

    fclose(fp);

    uint8_t pdu[100] = {0};
    int size = 0;

    int result = parse_and_encode("test_input.txt", pdu, &size);

    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0);
}

/******************************************
 * TEST CASES FOR REC_BIT_RATE
 *******************************************/
// Valid case
TEST_F(MACTest, RecBitRate_ValidInput)
{
    uint8_t temp_pdu[10] = {0};
    int lcid = 3;
    int rate = 20;
    int ul_dl = 1;

    temp_pdu[0] = LCID_REC_BIT_RATE;
    temp_pdu[1] = lcid << 2 | ul_dl << 1;
    temp_pdu[2] = rate << 2;

    int result = rec_bit_rate(pdu, &offset, 3, lcid, rate, ul_dl, flags, 3);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[0], temp_pdu[0]);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
    EXPECT_EQ(pdu[2], temp_pdu[2]);
}

// Missing rate
TEST_F(MACTest, RecBitRate_MissingRate)
{
    int result = rec_bit_rate(pdu, &offset, 2, 3, -1, 1, flags, 3);
    EXPECT_EQ(result, FAILURE);
}

// Missing UL/DL
TEST_F(MACTest, RecBitRate_MissingULDL)
{
    int result = rec_bit_rate(pdu, &offset, 2, 3, 20, -1, flags, 3);
    EXPECT_EQ(result, FAILURE);
}

// Missing LCID
TEST_F(MACTest, RecBitRate_MissingLCID)
{
    int result = rec_bit_rate(pdu, &offset, 2, -1, 20, 1, flags, 3);
    EXPECT_EQ(result, FAILURE);
}

// All missing
TEST_F(MACTest, RecBitRate_AllMissing)
{
    int result = rec_bit_rate(pdu, &offset, 0, -1, -1, -1, flags, 3);
    EXPECT_EQ(result, FAILURE);
}
// extra parameter
TEST_F(MACTest, RecBitRate_ExtraParameters)
{
    int result = rec_bit_rate(pdu, &offset, 4, 3, 20, 1, flags, 3);

    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0);
}
// Invalid LCID
TEST_F(MACTest, RecBitRate_InvalidLCIDRange)
{
    int result = rec_bit_rate(pdu, &offset, 3, 80, 20, 1, flags, 3);

    EXPECT_EQ(result, FAILURE);
}

// Invalid rate
TEST_F(MACTest, RecBitRate_InvalidRateRange)
{
    int result = rec_bit_rate(pdu, &offset, 3, 3, 100, 1, flags, 3);

    EXPECT_EQ(result, FAILURE);
}

// Invalid UL/DL
TEST_F(MACTest, RecBitRate_InvalidULDL)
{
    int result = rec_bit_rate(pdu, &offset, 3, 3, 20, 5, flags, 3);

    EXPECT_EQ(result, FAILURE);
}
// negative value
TEST_F(MACTest, RecBitRate_NegativeValues)
{
    int result = rec_bit_rate(pdu, &offset, 3, -2, -10, -1, flags, 3);

    EXPECT_EQ(result, FAILURE);
}

// Max boundary
TEST_F(MACTest, RecBitRate_MaxBoundaryValues)
{
    uint8_t temp_pdu[10] = {0};
    int lcid = 7;
    int rate = 63;
    int ul_dl = 1;

    temp_pdu[0] = LCID_REC_BIT_RATE;
    temp_pdu[1] = lcid << 2 | ul_dl << 1;
    temp_pdu[2] = rate << 2;

    int result = rec_bit_rate(pdu, &offset, 3, lcid, rate, ul_dl, flags, 3);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[0], temp_pdu[0]);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
    EXPECT_EQ(pdu[2], temp_pdu[2]);
}

// Min boundary
TEST_F(MACTest, RecBitRate_MinBoundaryValues)
{
    uint8_t temp_pdu[10] = {0};
    int lcid = 0;
    int rate = 0;
    int ul_dl = 0;

    temp_pdu[0] = LCID_REC_BIT_RATE;
    temp_pdu[1] = lcid << 2 | ul_dl << 1;
    temp_pdu[2] = rate << 2;

    int result = rec_bit_rate(pdu, &offset, 3, lcid, rate, ul_dl, flags, 3);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[0], temp_pdu[0]);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
    EXPECT_EQ(pdu[2], temp_pdu[2]);
}

TEST_F(ParseTest, RecBitRate_Rate_ValueMissing)
{
    uint8_t pdu[100] = {0};
    int size = 0;

    FILE *fp = fopen("test_input.txt", "w");

    fprintf(fp,
            "Total pdu_size 30\n"
            "num_ce 1\n"
            "<rec_bit_rate>\n"
            "lcid=3\n"
            "rate=\n"
            "ul_dl=1\n");

    fclose(fp);

    int result = parse_and_encode("test_input.txt", pdu, &size);

    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0);
}

/******************************************
 * TEST CASES FOR DSR
 *******************************************/
// Valid case
TEST_F(MACTest, DSR_ValidInput)
{
    uint8_t temp_pdu[10] = {0};
    int lcg = 3;
    int rt = 4;
    int buffer = 20;

    temp_pdu[0] = LCID_EXT_1BYTE;
    temp_pdu[1] = ELCID_DSR;
    temp_pdu[2] = (1 << lcg);
    temp_pdu[3] = (rt & 0x3F);
    temp_pdu[4] = buffer;

    int result = dsr(pdu, &offset, 3, lcg, rt, buffer, flags, state, 5);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[0], temp_pdu[0]);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
    EXPECT_EQ(pdu[2], temp_pdu[2]);
}

// Missing buffer
TEST_F(MACTest, DSR_MissingBuffer)
{
    int result = dsr(pdu, &offset, 2, 3, 4, -1, flags, state, 5);
    EXPECT_EQ(result, FAILURE);
}

// Missing RT
TEST_F(MACTest, DSR_MissingRT)
{
    int result = dsr(pdu, &offset, 2, 3, -1, 20, flags, state, 5);
    EXPECT_EQ(result, FAILURE);
}

// Missing LCG
TEST_F(MACTest, DSR_MissingLCG)
{
    int result = dsr(pdu, &offset, 2, -1, 4, 20, flags, state, 5);
    EXPECT_EQ(result, FAILURE);
}

// All missing
TEST_F(MACTest, DSR_AllMissing)
{
    int result = dsr(pdu, &offset, 0, -1, -1, -1, flags, state, 5);
    EXPECT_EQ(result, FAILURE);
}
TEST_F(MACTest, DSR_ExtraParameters)
{
    int result = dsr(pdu, &offset, 4, 3, 4, 20, flags, state, 5);

    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0);
}

// Invalid LCG
TEST_F(MACTest, DSR_InvalidLCGRange)
{
    int result = dsr(pdu, &offset, 3, 10, 4, 20, flags, state, 5);
    EXPECT_EQ(result, FAILURE);
}

// Invalid RT
TEST_F(MACTest, DSR_InvalidRTRange)
{
    int result = dsr(pdu, &offset, 3, 3, 78, 20, flags, state, 5);
    EXPECT_EQ(result, FAILURE);
}

// Invalid buffer
TEST_F(MACTest, DSR_InvalidBufferRange)
{
    int result = dsr(pdu, &offset, 3, 3, 4, 300, flags, state, 5);
    EXPECT_EQ(result, FAILURE);
}

// Max boundary
TEST_F(MACTest, DSR_MaxBoundaryValues)
{
    uint8_t temp_pdu[10] = {0};
    int lcg = 7;
    int rt = 7;
    int buffer = 63;

    temp_pdu[0] = LCID_EXT_1BYTE;
    temp_pdu[1] = ELCID_DSR;
    temp_pdu[2] = (1 << lcg);
    temp_pdu[3] = (rt & 0x3F);
    temp_pdu[4] = buffer;

    int result = dsr(pdu, &offset, 3, lcg, rt, buffer, flags, state, 5);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[0], temp_pdu[0]);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
    EXPECT_EQ(pdu[2], temp_pdu[2]);
}

// Min boundary
TEST_F(MACTest, DSR_MinBoundaryValues)
{
    uint8_t temp_pdu[10] = {0};
    int lcg = 0;
    int rt = 0;
    int buffer = 0;

    temp_pdu[0] = LCID_EXT_1BYTE;
    temp_pdu[1] = ELCID_DSR;
    temp_pdu[2] = (1 << lcg);
    temp_pdu[3] = (rt & 0x3F);
    temp_pdu[4] = buffer;

    int result = dsr(pdu, &offset, 3, lcg, rt, buffer, flags, state, 5);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[0], temp_pdu[0]);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
    EXPECT_EQ(pdu[2], temp_pdu[2]);
}

TEST_F(ParseTest, DSR_Buffer_ValueMissing)
{
    uint8_t pdu[100] = {0};
    int size = 0;

    FILE *fp = fopen("test_input.txt", "w");

    fprintf(fp,
            "Total pdu_size 30\n"
            "num_ce 1\n"
            "<dsr>\n"
            "lcg=3\n"
            "rt=\n"
            "buffer=4\n");

    fclose(fp);

    int result = parse_and_encode("test_input.txt", pdu, &size);

    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0);
}

/******************************************
 * TEST CASES FOR EXTENDED BSR
 *******************************************/
// Valid case
TEST_F(MACTest, ExtendedBSR_ValidInput)
{
    uint8_t temp_pdu[10] = {0};
    int lcg = 3;
    int buffer = 100;

    temp_pdu[0] = LCID_EXT_1BYTE;
    temp_pdu[1] = ELCID_EXT_BSR;
    temp_pdu[2] = lcg & 0x07;
    temp_pdu[3] = buffer & 0xFF;

    int result = extended_bsr(pdu, &offset, 2, lcg, buffer, 4);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[0], temp_pdu[0]);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
    EXPECT_EQ(pdu[2], temp_pdu[2]);
    EXPECT_EQ(pdu[3], temp_pdu[3]);
}

// Missing BUFFER
TEST_F(MACTest, ExtendedBSR_MissingBuffer)
{
    int result = extended_bsr(pdu, &offset, 1, 3, -1, 4);
    EXPECT_EQ(result, FAILURE);
}

// Missing LCG
TEST_F(MACTest, ExtendedBSR_MissingLCG)
{
    int result = extended_bsr(pdu, &offset, 1, -1, 50, 4);
    EXPECT_EQ(result, FAILURE);
}

// Both parameters missing
TEST_F(MACTest, ExtendedBSR_BothMissing)
{
    int result = extended_bsr(pdu, &offset, 0, -1, -1, 4);
    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0);
}

// Extra parameters
TEST_F(MACTest, ExtendedBSR_ExtraParameters)
{
    int result = extended_bsr(pdu, &offset, 3, 3, 50, 4);
    EXPECT_EQ(result, FAILURE);
    EXPECT_EQ(pdu[0], 0);
}

// Negative LCG
TEST_F(MACTest, ExtendedBSR_NegativeLCG)
{
    int result = extended_bsr(pdu, &offset, 2, -5, 50, 4);
    EXPECT_EQ(result, FAILURE);
}

// Negative BUFFER
TEST_F(MACTest, ExtendedBSR_NegativeBuffer)
{
    int result = extended_bsr(pdu, &offset, 2, 3, -10, 4);
    EXPECT_EQ(result, FAILURE);
}

// Invalid LCG range (>255)
TEST_F(MACTest, ExtendedBSR_InvalidLCGRange)
{
    int result = extended_bsr(pdu, &offset, 2, 300, 50, 4);
    EXPECT_EQ(result, FAILURE);
}

// Invalid BUFFER range (>255)
TEST_F(MACTest, ExtendedBSR_InvalidBufferRange)
{
    int result = extended_bsr(pdu, &offset, 2, 3, 300, 4);
    EXPECT_EQ(result, FAILURE);
}

// Boundary MAX values
TEST_F(MACTest, ExtendedBSR_MaxBoundary)
{
    uint8_t temp_pdu[10] = {0};

    int lcg = 7; // effectively used
    int buffer = 255;

    temp_pdu[0] = LCID_EXT_1BYTE;
    temp_pdu[1] = ELCID_EXT_BSR;
    temp_pdu[2] = lcg & 0x07;
    temp_pdu[3] = buffer & 0xFF;

    int result = extended_bsr(pdu, &offset, 2, lcg, buffer, 4);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[2], temp_pdu[2]);
    EXPECT_EQ(pdu[3], temp_pdu[3]);
}

// Boundary MIN values
TEST_F(MACTest, ExtendedBSR_MinBoundary)
{
    uint8_t temp_pdu[10] = {0};

    int lcg = 0;
    int buffer = 0;

    temp_pdu[0] = LCID_EXT_1BYTE;
    temp_pdu[1] = ELCID_EXT_BSR;
    temp_pdu[2] = 0;
    temp_pdu[3] = 0;

    int result = extended_bsr(pdu, &offset, 2, lcg, buffer, 4);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[2], 0);
    EXPECT_EQ(pdu[3], 0);
}

TEST_F(ParseTest, ExtendedBSR_MissingBuffer)
{
    FILE *fp = fopen("test_input.txt", "w");

    fprintf(fp,
            "Total pdu_size 30\n"
            "num_ce 1\n"
            "<extended_bsr>\n"
            "lcgid=3\n"); // buffer missing

    fclose(fp);

    uint8_t pdu[100] = {0};
    int size = 0;

    int result = parse_and_encode("test_input.txt", pdu, &size);

    EXPECT_EQ(result, FAILURE);
}

/******************************************
 * TEST CASES FOR enhanced phr
 *******************************************/
TEST_F(MACTest, EnhancedPHR_ValidInput)
{
    uint8_t temp_pdu[10] = {0};
    int params[3] = {20, 25, 15}; // PH1, PH2, PCMAX

    temp_pdu[0] = LCID_EXT_1BYTE;
    temp_pdu[1] = ELCID_ENH_PHR;
    temp_pdu[2] = (20 << 1);
    temp_pdu[3] = (25 << 1);
    temp_pdu[4] = (15 << 1);

    int result = enhanced_phr(pdu, &offset, 3, params, 5);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[0], temp_pdu[0]);
    EXPECT_EQ(pdu[1], temp_pdu[1]);
    EXPECT_EQ(pdu[2], temp_pdu[2]);
    EXPECT_EQ(pdu[3], temp_pdu[3]);
    EXPECT_EQ(pdu[4], temp_pdu[4]);
}

// Missing PH1
TEST_F(MACTest, EnhancedPHR_MissingPH1)
{
    int params[3] = {-1, 20, 10};

    int result = enhanced_phr(pdu, &offset, 3, params, 5);
    EXPECT_EQ(result, FAILURE);
}

// Missing PH2
TEST_F(MACTest, EnhancedPHR_MissingPH2)
{
    int params[3] = {10, -1, 10};

    int result = enhanced_phr(pdu, &offset, 3, params, 5);
    EXPECT_EQ(result, FAILURE);
}

// Missing PCMAX
TEST_F(MACTest, EnhancedPHR_MissingPCMAX)
{
    int params[3] = {10, 20, -1};

    int result = enhanced_phr(pdu, &offset, 3, params, 5);
    EXPECT_EQ(result, FAILURE);
}

// All missing
TEST_F(MACTest, EnhancedPHR_AllMissing)
{
    int params[3] = {-1, -1, -1};

    int result = enhanced_phr(pdu, &offset, 3, params, 5);
    EXPECT_EQ(result, FAILURE);
}

// PH1 out of range
TEST_F(MACTest, EnhancedPHR_InvalidPH1)
{
    int params[3] = {100, 20, 10};

    int result = enhanced_phr(pdu, &offset, 3, params, 5);
    EXPECT_EQ(result, FAILURE);
}

// PH2 out of range
TEST_F(MACTest, EnhancedPHR_InvalidPH2)
{
    int params[3] = {10, 100, 10};

    int result = enhanced_phr(pdu, &offset, 3, params, 5);
    EXPECT_EQ(result, FAILURE);
}

// PCMAX out of range
TEST_F(MACTest, EnhancedPHR_InvalidPCMAX)
{
    int params[3] = {10, 20, 100};

    int result = enhanced_phr(pdu, &offset, 3, params, 5);
    EXPECT_EQ(result, FAILURE);
}
TEST_F(MACTest, EnhancedPHR_NegativeValues)
{
    int params[3] = {-5, -10, -1};

    int result = enhanced_phr(pdu, &offset, 3, params, 5);
    EXPECT_EQ(result, FAILURE);
}

// Max boundary
TEST_F(MACTest, EnhancedPHR_MaxBoundary)
{
    uint8_t temp_pdu[10] = {0};

    int params[3] = {63, 63, 63};

    temp_pdu[0] = LCID_EXT_1BYTE;
    temp_pdu[1] = ELCID_ENH_PHR;
    temp_pdu[2] = (63 << 1);
    temp_pdu[3] = (63 << 1);
    temp_pdu[4] = (63 << 1);

    int result = enhanced_phr(pdu, &offset, 3, params, 5);

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(pdu[2], temp_pdu[2]);
}

// Min boundary
TEST_F(MACTest, EnhancedPHR_MinBoundary)
{
    int params[3] = {0, 0, 0};

    int result = enhanced_phr(pdu, &offset, 3, params, 5);
    EXPECT_EQ(result, SUCCESS);
}
TEST_F(ParseTest, EnhancedPHR_PCMAX_Missing)
{
    FILE *fp = fopen("test_input.txt", "w");

    fprintf(fp,
            "Total pdu_size 30\n"
            "num_ce 1\n"
            "<enhanced_phr>\n"
            "ph1=10\n"
            "ph2=20\n");

    fclose(fp);

    uint8_t pdu[100] = {0};
    int size = 0;

    int result = parse_and_encode("test_input.txt", pdu, &size);

    EXPECT_EQ(result, FAILURE);
}