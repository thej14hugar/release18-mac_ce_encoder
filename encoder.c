#include <stdio.h>
#include <string.h>
#include "input_validation.h"
#include "encoder.h"
 
#define MAX_LINE 100
 
    uint8_t MPE = 1;
    uint8_t R2  = 0;
    uint8_t P = 1;
    uint8_t R = 0;
 
//----------------------------------
// VALIDATION
//----------------------------------
int check_range(int val, int min, int max, const char *name)
{
    if (val < min || val > max)
    {
        printf("ERROR: %s out of range (%d-%d)\n", name, min, max);
        return FAILURE;
    }
    return SUCCESS;
}
 
//----------------------------------
// SHORT BSR
//----------------------------------
  // MAC subPDU with:
    // - 8-bit MAC subheader (LCID = 6 bits , R =2 bits)
    // - MAC CE payload (1 octet)
    //
    // MAC CE FORMAT:
    // | LCG ID (2 bits) | Buffer Size (6 bits) |
int short_bsr(uint8_t *pdu, int *offset, int argc, int lcg, int buffer)
{
    if (argc < 2)
    {
        printf("ERROR: short_bsr missing parameters (LCG BUFFER)\n");
        return FAILURE;
    }
    if (argc > 2)
    {
        printf("ERROR: short_bsr extra parameters detected\n");
        return FAILURE;
    }
 
    if (lcg < 0 || buffer < 0)
    {
        printf("ERROR: Invalid values\n");
        return FAILURE;
    }
 
    if (check_range(lcg, 0, 7, "LCG")) return FAILURE;
    if (check_range(buffer, 0, 31, "BUFFER")) return FAILURE;
     // Octet:
    // Bits [7:6] → LCG ID
    // Bits [5:0] → Buffer Size
 
    pdu[(*offset)++] = LCID_SHORT_BSR;
    pdu[(*offset)++] = (lcg << 6) | buffer;
 
    return SUCCESS;
}
 
//----------------------------------
// PHR
//----------------------------------
int phr(uint8_t *pdu, int *offset, int argc, int ph, int pcmax)
{
    // ✅ PARAM COUNT CHECK
    if (argc == 0)
    {
        printf("ERROR: phr missing parameters (PH PCMAX)\n");
        return FAILURE;
    }

    if (argc == 1)
    {
        printf("ERROR: phr missing parameter (PCMAX)\n");
        return FAILURE;
    }

    if (argc > 2)
    {
        printf("ERROR: phr extra parameters detected\n");
        return FAILURE;
    }

    if (ph == -1)
    {
        printf("ERROR: PH not provided \n");
        return FAILURE;
    }

    if (pcmax == -1)
    {
        printf("ERROR: PCMAX not provided \n");
        return FAILURE;
    }

   
    if (ph < 0)
    {
        printf("ERROR: PH cannot be negative\n");
        return FAILURE;
    }

    if (pcmax < 0)
    {
        printf("ERROR: PCMAX cannot be negative\n");
        return FAILURE;
    }

   
    if (check_range(ph, 0, 63, "PH")) return FAILURE;
    if (check_range(pcmax, 0, 63, "PCMAX")) return FAILURE;

    
    pdu[(*offset)++] = LCID_PHR;

    pdu[(*offset)++] = (P << 7) | (R << 6) | (ph & 0x3F);

    pdu[(*offset)++] = (MPE << 7) | (R2 << 6) | (pcmax & 0x3F);

    return SUCCESS;
}
//----------------------------------
// CRNTI
//----------------------------------
int crnti(uint8_t *pdu, int *offset, int argc, int value)
{
    
    if (argc == 0)
    {
        printf("ERROR: crnti missing parameter (CRNTI value)\n");
        return FAILURE;
    }

    if (argc > 1)
    {
        printf("ERROR: crnti extra parameters detected\n");
        return FAILURE;
    }

    
    if (value == -1)
    {
        printf("ERROR: CRNTI value not provided \n");
        return FAILURE;
    }

    
    if (value < 0)
    {
        printf("ERROR: CRNTI cannot be negative\n");
        return FAILURE;
    }

   
    if (value > 65535)
    {
        printf("ERROR: CRNTI out of range (0-65535)\n");
        return FAILURE;
    }

    // ENCODING
    pdu[(*offset)++] = LCID_CRNTI;
    pdu[(*offset)++] = (value >> 8) & 0xFF;
    pdu[(*offset)++] = value & 0xFF;

    return SUCCESS;
}
 
//----------------------------------
// DSR
//----------------------------------
int dsr(uint8_t *pdu, int *offset, int argc, int *params)
{
   
   // MAC subPDU with:
// - 16-bit MAC subheader (Extended LCID)
// - MAC CE with DSR
//
// Subheader:
// Octet 1 → R | R | LCID = 0x3F (LCID = 63 → extended)
// Octet 2 → eLCID = 228 (DSR)
    // FORMAT:
    // Octet 1 → LCG bitmap (which LCGs are present)
    // Then for each entry:
    //   Octet → | BT (1 bit) | R (1 bit) | Remaining Time (6 bits) |
    //   Octet → Buffer Size (8 bits)
    if (argc < 3)
    {
        printf("ERROR: dsr missing parameters (LCG RT BUFFER)\n");
        return FAILURE;
    }
 
    if (argc % 3 != 0)
    {
        printf("ERROR: Extra parameters detected\n");
        return FAILURE;
    }
 
    int entries = argc / 3;
 
    // -------- BUILD LCG BITMAP --------
    uint8_t lcg_bitmap = 0;
 
    for (int i = 0; i < entries; i++)
    {
        int lcg = params[i*3];
 
        if (lcg < 0)
        {
            printf("ERROR: LCG cannot be negative\n");
            return FAILURE;
        }
 
        if (check_range(lcg, 0, 7, "LCG")) return FAILURE;
 
        lcg_bitmap |= (1 << lcg);
    }
 
    // -------- ENCODING --------
 
    // LCID
    pdu[(*offset)++] = LCID_DSR;
 
    // LCG bitmap
    pdu[(*offset)++] = lcg_bitmap;
 
    // -------- EACH ENTRY --------
    for (int i = 0; i < entries; i++)
    {
        int lcg    = params[i*3];
        int rt     = params[i*3 + 1];
        int buffer = params[i*3 + 2];
 
        // validation
        if (rt < 0 || buffer < 0)
        {
            printf("ERROR: Negative values not allowed\n");
            return FAILURE;
        }
 
        if (check_range(rt, 0, 63, "RT")) return FAILURE;
        if (check_range(buffer, 0, 255, "BUFFER")) return FAILURE;
 
        // Octet → BT | R | RT
        uint8_t BT = 0;
        uint8_t R  = 0;
 
        pdu[(*offset)++] = (BT << 7) | (R << 6) | (rt & 0x3F);
 
        // Buffer size
        pdu[(*offset)++] = buffer;
    }
 
    return SUCCESS;
}

//----------------------------------
// RECOMMENDED BIT RATE
//----------------------------------
int rec_bit_rate(uint8_t *pdu, int *offset, int argc, int lcid, int rate, int ul_dl)
{
    // -------- PARAM COUNT --------
    if (argc < 3)
    {
        printf("ERROR: rec_bit_rate missing parameters (LCID BIT_RATE UL/DL)\n");
        return FAILURE;
    }

    if (argc > 3)
    {
        printf("ERROR: rec_bit_rate extra parameters detected\n");
        return FAILURE;
    }

    // -------- INVALID CHECK --------
    if (lcid == -1)
    {
        printf("ERROR: logical channel LCID not provided\n");
        return FAILURE;
    }

    if (rate == -1)
    {
        printf("ERROR: bit_rate not provided\n");
        return FAILURE;
    }

    if (ul_dl == -1)
    {
        printf("ERROR: UL/DL not provided\n");
        return FAILURE;
    }

    // -------- NEGATIVE CHECK --------
    if (lcid < 0 || rate < 0 || ul_dl < 0)
    {
        printf("ERROR: negative values not allowed\n");
        return FAILURE;
    }

    // -------- RANGE CHECK --------
    if (check_range(lcid, 0, 63, "LCID")) return FAILURE;
    if (check_range(rate, 0, 63, "BIT_RATE")) return FAILURE;
    if (check_range(ul_dl, 0, 1, "UL/DL")) return FAILURE;

    // -------- ENCODING --------

    // Subheader LCID (MAC CE type)
    pdu[(*offset)++] = LCID_REC_BIT_RATE;

    // Octet 1: LCID + UL/DL + R
    uint8_t R = 0;
    pdu[(*offset)++] = (lcid << 2) | (ul_dl << 1) | R;

    // Octet 2: Bit Rate + X + R
    uint8_t X = 0;  // multiplier bit
    uint8_t R2 = 0;
    pdu[(*offset)++] = (rate << 2) | (X << 1) | R2;

    return SUCCESS;
}
int enhanced_phr(uint8_t *pdu, int *offset, int ph1, int ph2, int pcmax)
{
    pdu[(*offset)++] = LCID_ENH_PHR;

    // Octet 1: P | V | PH1
    pdu[(*offset)++] = (1 << 7) | (0 << 6) | (ph1 & 0x3F);

    // Octet 2: R | V | PH2
    pdu[(*offset)++] = (0 << 7) | (0 << 6) | (ph2 & 0x3F);

    // Octet 3: MPE | R | PCMAX
    pdu[(*offset)++] = (1 << 7) | (0 << 6) | (pcmax & 0x3F);

    return SUCCESS;
}

int sl_lbt(uint8_t *pdu, int *offset, int value)
{
    pdu[(*offset)++] = LCID_SL_LBT;

    // | R R R R4 R3 R2 R1 R0 |
    pdu[(*offset)++] = value & 0x1F;

    return SUCCESS;
}

int enhanced_bfr(uint8_t *pdu, int *offset, int argc, int *params)
{
    if (argc < 5)
    {
        printf("ERROR: enhanced_bfr requires ci, s and entries\n");
        return FAILURE;
    }

    if ((argc - 2) % 3 != 0)
    {
        printf("ERROR: enhanced_bfr entries must be (ac id candidate_id)\n");
        return FAILURE;
    }

    int entries = (argc - 2) / 3;

    int ci = params[0];
    int s  = params[1];

    if (ci < 0 || s < 0)
    {
        printf("ERROR: ci/s cannot be negative\n");
        return FAILURE;
    }

    if (check_range(ci, 0, 255, "CI")) return FAILURE;
    if (check_range(s, 0, 255, "S")) return FAILURE;

    // LCID
    pdu[(*offset)++] = LCID_ENH_BFR;

    // CI & S
    pdu[(*offset)++] = ci;
    pdu[(*offset)++] = s;

    for (int i = 0; i < entries; i++)
    {
        int ac  = params[2 + i*3];
        int id  = params[2 + i*3 + 1];
        int cid = params[2 + i*3 + 2];

        if (ac < 0 || id < 0 || cid < 0)
        {
            printf("ERROR: Negative values not allowed\n");
            return FAILURE;
        }

        if (check_range(ac, 0, 1, "AC")) return FAILURE;
        if (check_range(id, 0, 1, "ID")) return FAILURE;
        if (check_range(cid, 0, 63, "Candidate ID")) return FAILURE;

        pdu[(*offset)++] = (ac << 7) | (id << 6) | (cid & 0x3F);
    }

    return SUCCESS;
}
int extended_bsr(uint8_t *pdu, int *offset, int lcg, int buffer)
{
    pdu[(*offset)++] = LCID_EXT_BSR;

    // Octet 1 → LCG
    pdu[(*offset)++] = lcg & 0x07;

    // Octet 2 → Buffer
    pdu[(*offset)++] = buffer & 0xFF;

    return SUCCESS;
}
 
//----------------------------------
// PRINT HEX
//----------------------------------
void print_hex(uint8_t *data, int len)
{
    for (int i = 0; i < len; i++)
        printf("%02X ", data[i]);
}
 
//----------------------------------
// PRINT BITS
//----------------------------------
void print_bits(uint8_t *data, int len)
{
    for (int i = 0; i < len; i++)
    {
        for (int j = 7; j >= 0; j--)
            printf("%d", (data[i] >> j) & 1);
        printf(" ");
    }
}
 
//----------------------------------
// PADDING
//----------------------------------
void add_padding(uint8_t *buffer, int *offset, int remaining)
{
    for (int i = 0; i < remaining; i++)
    {
        buffer[(*offset)++] = 0x00;
    }
}
 
//----------------------------------
// TYPE → ID
//----------------------------------
int get_ce_id(char *type)
{
    if (strcmp(type,"short_bsr")==0) return 1;
    if (strcmp(type,"phr")==0) return 2;
    if (strcmp(type,"crnti")==0) return 3;
    if (strcmp(type,"rec_bit_rate")==0) return 4;
    if (strcmp(type,"dsr")==0) return 5;
    if (strcmp(type,"enhanced_phr")==0) return 6;
if (strcmp(type,"sl_lbt")==0) return 7;
if (strcmp(type,"enhanced_bfr")==0) return 8;
if (strcmp(type,"extended_bsr")==0) return 9;
    return -1;
}
 
//----------------------------------
// PARSER (FINAL)
//----------------------------------
int parse_and_encode(const char *filename, uint8_t *pdu, int *pdu_size)
{
    if (validate_input_file(filename) == FAILURE)
        return FAILURE;
 
    FILE *fp = fopen(filename, "r");
 
    char line[MAX_LINE];
    int offset = 0;
 
    fgets(line, sizeof(line), fp);
    sscanf(line, "Total pdu_size %d", pdu_size);
 
    printf(" TOTAL PDU SIZE : %d\n\n", *pdu_size);
 
    while (fgets(line, sizeof(line), fp))
    {
        //detect <ce_type>
        if (line[0] == '<')
        {
        char type[20];
        sscanf(line, "<%[^>]>",type);
        int a=-1, b=-1, c=-1;
        int before = offset;
        int ret = FAILURE;
 
        printf("MAC CE : %s\n", type);
        int id = get_ce_id(type);
        if (id == -1)
        {
            printf("ERROR: Unknown CE %s\n\n", type);
            continue;
        }
        switch(id)
        {
         
           case 1:
{
    int param_count = 0;

    // Read LCG
    fgets(line, sizeof(line), fp);
    sscanf(strchr(line,'=')+1,"%d",&a);
    param_count++;

    // Read BUFFER
    fgets(line, sizeof(line), fp);
    sscanf(strchr(line,'=')+1,"%d",&b);
    param_count++;

    // Check for extra parameter
    long pos = ftell(fp);
    if (fgets(line, sizeof(line), fp))
    {
        if (strchr(line, '<') == NULL && strchr(line, '=') != NULL)
        {
            param_count++;   // extra param detected
        }
        fseek(fp, pos, SEEK_SET);
    }

    ret = short_bsr(pdu, &offset, param_count, a, b);
    break;   
}
             
            case 2: // MAC subPDU:
                // - 8-bit MAC subheader (LCID = PHR)
                // - MAC CE payload (2 octets)
              {
        fgets(line, sizeof(line), fp);
       sscanf(strchr(line, '=') + 1, "%d", &a);
 
        fgets(line, sizeof(line), fp);
       sscanf(strchr(line, '=') + 1, "%d", &b);
 
        ret = phr(pdu, &offset, 2, a, b);
        break;
    }
             
            case 3: // MAC subPDU:
                // - 8-bit MAC subheader (LCID = CRNTI)
                // - MAC CE payload (2 octets)
             {
        fgets(line, sizeof(line), fp);
       sscanf(strchr(line, '=') + 1, "%d", &a);
 
        ret = crnti(pdu, &offset, 1, a);
        break;
    }
 
           
           case 4:
{
    int param_count = 0;

    // LCID
    fgets(line, sizeof(line), fp);
    sscanf(strchr(line,'=')+1,"%d",&a);
    param_count++;

    // BIT RATE
    fgets(line, sizeof(line), fp);
    sscanf(strchr(line,'=')+1,"%d",&b);
    param_count++;

    // UL/DL
    fgets(line, sizeof(line), fp);
    sscanf(strchr(line,'=')+1,"%d",&c);
    param_count++;

    // check extra param
    long pos = ftell(fp);
    if (fgets(line, sizeof(line), fp))
    {
        if (strchr(line, '<') == NULL && strchr(line, '=') != NULL)
        {
            param_count++;
        }
        fseek(fp, pos, SEEK_SET);
    }

    ret = rec_bit_rate(pdu, &offset, param_count, a, b, c);
    break;
}
 
            
   case 5:
{
    int params[100];
    int count = 0;

    while (fgets(line, sizeof(line), fp))
    {
        if (strchr(line, '<') != NULL)
        {
            fseek(fp, -strlen(line), SEEK_CUR);
            break;
        }

        if (strchr(line, '=') == NULL) continue;

       char *ptr = strchr(line,'=');

int val = -1;

if (ptr != NULL && *(ptr+1) != '\n' && *(ptr+1) != '\0' && sscanf(ptr+1,"%d",&val) == 1)
{
    params[count++] = val;
}
else
{
    printf("ERROR:missing value in DSR input\n\n");
    return FAILURE;
}
    }

    ret = dsr(pdu, &offset, count, params);
    break;
}

    case 6:
{
    fgets(line, sizeof(line), fp);
    sscanf(strchr(line,'=')+1,"%d",&a);

    fgets(line, sizeof(line), fp);
    sscanf(strchr(line,'=')+1,"%d",&b);

    fgets(line, sizeof(line), fp);
    sscanf(strchr(line,'=')+1,"%d",&c);

    ret = enhanced_phr(pdu, &offset, a, b, c);
    break;
}

case 7:
{
    fgets(line, sizeof(line), fp);
    sscanf(strchr(line,'=')+1,"%d",&a);

    ret = sl_lbt(pdu, &offset, a);
    break;
}

case 8:
{
    int params[100];
    int count = 0;

    while (fgets(line, sizeof(line), fp))
    {
        if (strchr(line, '<') != NULL)
        {
            fseek(fp, -strlen(line), SEEK_CUR);
            break;
        }

        if (strchr(line, '=') == NULL) continue;

        char *ptr = strchr(line,'=');
        int val = -1;

        if (ptr && sscanf(ptr+1,"%d",&val) == 1)
        {
            params[count++] = val;
        }
        else
        {
            printf("ERROR: invalid enhanced_bfr input\n");
            return FAILURE;
        }
    }

    ret = enhanced_bfr(pdu, &offset, count, params);
    break;
}

case 9:
{
    fgets(line, sizeof(line), fp);
    sscanf(strchr(line,'=')+1,"%d",&a);

    fgets(line, sizeof(line), fp);
    sscanf(strchr(line,'=')+1,"%d",&b);

    ret = extended_bsr(pdu, &offset, a, b);
    break;
}
}
if (ret == FAILURE)
        {
            printf("\n");
            continue;
        }
 
       int ce_len = offset - before;
 
// SUBHEADER SIZE (DSR = extended)
int is_extended_ce = (id == 5);  // only DSR
int subheader_size = is_extended_ce ? 2 : 1;
        
int payload_size = ce_len - subheader_size;
int total_size = ce_len;
 
printf("Subheader Size : %d byte\n", subheader_size);
printf("Payload Size   : %d bytes\n", payload_size);
printf("Total CE Size  : %d bytes (Subheader + Payload)\n", total_size);
 
// PRINT BITS
printf("Encoded Bits : ");
print_bits(&pdu[before], ce_len);
printf("\n");
 
// PRINT HEX
printf("Encoded Hex  : ");
print_hex(&pdu[before], ce_len);
printf("\n");
 
// SUCCESS MESSAGE
printf("[SUCCESS] %s Encoded\n\n", type);
    }
}
 // ================= FINAL OUTPUT =================
 
int total_used_before = offset;
int remaining = *pdu_size - offset;
 
printf("\n===== FINAL SUMMARY =====\n");
printf("Total PDU Size   : %d bytes\n", *pdu_size);
printf("Total Used Bytes : %d bytes\n", total_used_before);
printf("Remaining Bytes  : %d bytes\n", remaining);
 
// PADDING
add_padding(pdu, &offset, remaining);
 
printf("\nRemaining bytes filled with 00.\n");
 
// FINAL BUFFER
printf("\nFinal MAC Buffer:\n");
print_hex(pdu, *pdu_size);
printf("\n");
    fclose(fp);
    return SUCCESS;
}
 
 