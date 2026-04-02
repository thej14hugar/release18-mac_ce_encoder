## Overview
The program reads input from a text file named input.txt.
 
This file defines:
->Total MAC PDU size
->Number of MAC Control Elements (CEs) to encode
->CE blocks with parameters
The program parses the file line-by-line and encodes only the specified number of CEs
 
## File Structure
The input file consists of:
1. "Global parameters"
2. "Number of Control Elements"
3. Repeated CE blocks"
 
## 1. Global Parameters
These must appear at the top of the file.
 
### "Total pdu_size"
Defines the total size of the MAC PDU (in bytes)
-------------------
---text---
Total pdu_size <integer>
-------------------
 
### "num_ce"
Number of Control Elements defined in the file
----------------
---text---
num_ce <integer>
----------------
 
## 2. Control Element Blocks
Each Control Element is defined using the following structure:
--------------
---text---
 
<ce_name>
parameter=value
parameter=value
 
--------------
 
## 3. Supported Control Elements
Below are the supported CE types and their required parameters.
 
### SHORT_BSR
 
---text---
 
<short_bsr>
lcgid=<integer>
buffer=<integer>
 
----------------------------------------------------------------------
 
### PHR (Power Headroom Report)
 
---text---
 
<phr>
ph=<integer>
pcmax=<integer>
 
----------------------------------------------------------------------
 
### ENH_PHR (Enhanced PHR)
 
---text---
 
<enh_phr>
ph=<integer>
pcmax=<integer>
 
---------------------------------------------------------------------
 
### ENH_BFR (Enhanced Beam Failure Recovery)
 
---text---
 
<enhanced_bfr>
ci=<integer>
s=<integer>
ac=<integer>
id=<integer>
candidate_id=<integer>
 
---------------------------------------------------------------------
 
### CRNTI
 
---text---
 
<crnti>
value=<integer>
 
---------------------------------------------------------------------
 
### REC_BIT_RATE
 
---text---
 
<rec_bit_rate>
lcid=<integer>
bit_rate=<integer>
ui_dl_=<0 or 1>
 
---------------------------------------------------------------------
 
### EXT_BSR
 
---text---
 
<extended_bsr>
lcg<integer>
buffer<integer>
 
-----------------------------------------------------------------------
 
### SL_LBT
 
---text---
 
<sl_lbt>
value=<integer>
 
-----------------------------------------------------------------------
 
### DSR
 
---text---
 
<dsr>
lcg=<integer>
rt=<integer>
buffer=<integer>
 
-----------------------------------------------------------------------
 
## Example
 
---text---
Total pdu_size 12
num_ce 2
 
<short_bsr>
lcgid=2
buffer=12
 
<phr>
ph=23
pcmax=5
 
-----------------------------------------------------------------------
 
## Parsing Rules
 
* CE must start with < >
* Parameters use =
* Order of parameters must match expected format
* Only num_ce CEs are processed
 
------------------------------------------------------------------------
 
## Error Handling
The program may produce errors if:
 
* Invalid PDU size
* Invalid num_ce
* Unknown CE name
* Missing parameters
* Extra parameters
* *n-integer values
* Values out of range
 
-------------------------------------------------------------------------
 
 
 
 
 