## Overview
The program reads input from a text file named `input.txt`.
This file defines the MAC PDU size and a list of MAC Control Elements (CEs) along with their parameters.
 
## File Structure
The input file consists of:
1. "Global configuration"
2. "Number of Control Elements"
3. Repeated CE blocks"
 
## 1. Global Parameters
These must appear at the top of the file.
 
### "pdu_size"
Defines the total size of the MAC PDU (in bytes)
-------------------
---text---
pdu_size=<integer>
-------------------
 
### "num_ce"
Number of Control Elements defined in the file
----------------
---text---
num_ce=<integer>
----------------
 
## 2. Control Element Blocks
Each Control Element is defined using the following structure:
--------------
---text---
ce_type
<ce_name>
<parameter_1>
<parameter_2>
 
--------------
 
## 3. Supported Control Elements
Below are the supported CE types and their required parameters.
 
### SHORT_BSR
 
---text---
ce_type
<short_bsr>
lcgid=<integer>
buffer_size=<integer>
 
----------------------------------------------------------------------
 
### PHR (Power Headroom Report)
 
---text---
ce_type
<phr>
ph=<integer>
pcmax=<integer>
 
----------------------------------------------------------------------
 
### ENH_PHR (Enhanced PHR)
 
---text---
ce_type
<enh_phr>
ph1=<integer>
ph2=<integer>
pcmax=<integer>
 
---------------------------------------------------------------------
 
### ENH_BFR (Enhanced Beam Failure Recovery)
 
---text---
ce_type
<enh_bfr>
c bits=<integer>
sp=<integer>
s bits=<integer>
num_candidates=<integer>
AC=<integer>
ID=<integer>
rs_id=<integer>
 
---------------------------------------------------------------------
 
### CRNTI
 
---text---
ce_type
<crnti>
crnti=<integer>
 
---------------------------------------------------------------------
 
### REC_BIT_RATE
 
---text---
ce_type
<rec_bit_rate>
bit_rate=<integer>
 
---------------------------------------------------------------------
 
### EXT_BSR
 
---text---
ce_type
<ext_bsr>
lcg_id=<integer>
buffer_size=<integer>
 
-----------------------------------------------------------------------
 
### SL_LBT
 
---text---
ce_type
<sl_lbt>
lbt_info=<integer>
 
-----------------------------------------------------------------------
 
### DSR
 
---text---
ce_type
<dsr>
dsr=<integer>
 
-----------------------------------------------------------------------
 
## Example
 
---text---
pdu_size=12
num_ce=2
 
ce_type
<short_bsr>
lcgid=2
buffer_size=12
 
ce_type
<phr>
ph=23
pcmax=5
 
-----------------------------------------------------------------------
 
## Parsing Rules
 
* The file is parsed "line by line"
* Keys and values are separated using `=`
* CE names must be enclosed in `< >`
* Order of parameters must match the expected format for each CE
* Empty lines are not ignored
 
------------------------------------------------------------------------
 
## Error Handling
The program may produce errors if:
 
* `ce_type` is missing before a CE block
* Unknown CE name is used
* Required parameters are missing
* `num_ce` does not match actual CE blocks
 
-------------------------------------------------------------------------
 
## Notes
* All values are expected to be integers unless specified
* Parameter names are case-sensitive
* Extra spaces or lines may cause parsing failures
 
--------------------------------------------------------------------------
 
## Design Rationale
This format was chosen because:
 
*  Easy to read and edit manually
*  Extensible for adding new CE types
*  Structured for deterministic parsing
 
---------------------------------------------------------------------------
 