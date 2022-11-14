/* modi.h

   Copyright (c) 1993-2017. Free Software Foundation, Inc.

   This file is part of GNU MCSim.

   GNU MCSim  is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 3
   of the License, or (at your option) any later version.

   GNU MCSim is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GNU MCSim; if not, see <http://www.gnu.org/licenses/>

   Header file for main parsing routines.
*/

#ifndef MODI_H_DEFINED

/* ---------------------------------------------------------------------------
   Public Typedefs */

typedef struct tagKM {
  PSTR szKeyword;
  int  iKWCode;   /* Enumeration code of Keyword KM_* */
  WORD  fContext; /* Bit flags of valid context for KW */
} KM, *PKM; /* Keyword Map */


/* ---------------------------------------------------------------------------
   Public Prototypes */

int  GetKeywordCode (PSTR szKeyword, PINT pfContext);
PSTR GetKeyword (int iCode);
void GetVarList (PINPUTBUF pibIn, PSTR szLex, int iKWCode);
void ProcessDTStatement (PINPUTBUF pibIn, PSTR szLex, PSTR szEqn, int iKWCode);
void ProcessIdentifier (PINPUTBUF pibIn, PSTR szLex, PSTR szEqn, int iKWCode);
void ProcessInlineStatement (PINPUTBUF pibIn, PSTR szLex, PSTR szEqn, 
                             int iKWCode);
void ProcessWord (PINPUTBUF pibIn, PSTR szLex, PSTR szEqn);
void ReadModel (PINPUTINFO pinfo, PINPUTINFO ptempinfo, PSTR szFileIn);

#define MODI_H_DEFINED
#endif

/* End */


