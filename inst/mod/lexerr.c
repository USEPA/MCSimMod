/* lexerr.c

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

   Reports errors and exits program if fatal.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lexerr.h"


/* ---------------------------------------------------------------------------
   ReportError

   Reports error iCode to terminal (one of RE_) and optional
   szMessage. If iSeverity is set to RE_FATAL, exits program.
*/

void ReportError (PINPUTBUF pibIn, WORD wCode, PSTR szMsg, PSTR szAltMsg)
{
  char cNull = '\0';
  BOOL bFatal   = wCode & RE_FATAL;
  BOOL bWarning = wCode & RE_WARNING;

  wCode &= ~(RE_FATAL | RE_WARNING);

  if (!szMsg)
    szMsg = &cNull;

  if (wCode) {
    if (bWarning)
      printf("*** Warning: ");
    else {
      printf("*** Error: ");
      bFatal |= (pibIn && (pibIn->cErrors++ > MAX_ERRORS));
    } /* else */
  } /* if */

  if (pibIn) {
    if (pibIn->pfileIn || pibIn->iLNPrev) { /* Line number is valid */
      printf("line %d: ", pibIn->iLineNum);
    }
    else {
      if (wCode != RE_FILENOTFOUND) { /* Dummy pibIn, show buffer */
        PSTRLEX szTmp;
        szTmp[MAX_LEX-1] = '\0';
        printf("'%s'...\n  ", strncpy (szTmp, pibIn->pbufOrg, MAX_LEX-1));
      } /* if */
    }
  }

  switch (wCode) {

  case 0:
    break;

  default:
    printf("Unknown error code %x: %s", wCode, szMsg);

  case RE_INIT:
    printf("Initialization error.");
    break;

  case RE_FILENOTFOUND:
    printf("File not found \"%s\".", szMsg);
    break;

  case RE_CANNOTOPEN:
    printf("Cannot open file \"%s\".", szMsg);
    break;

  case RE_UNEXPECTED:
    printf("Unexpected character '%c' in input file.", *szMsg);
    break;

  case RE_UNEXPESCAPE:
    printf("Unexpected escape sequence '%s' in input file.", szMsg);
    break;
    
  case RE_UNEXPNUMBER:
    printf("Unexpected number %s in input file.", szMsg);
    break;

  case RE_EXPECTED:
    printf("Expected '%c' before '%c'.", szMsg[1], szMsg[0]);
    break;

  case RE_LEXEXPECTED:
    printf("Expected <%s>", szMsg);
    if (szAltMsg)
      printf(" before '%s'", szAltMsg);
    break;

  /* USER error handling -- Add user error reporting below */

  /* Model generator errors */

  case RE_BADCONTEXT:
    printf("'%s' used in invalid context.", szMsg);
    break;

  case RE_DUPDECL:
    printf("Duplicate declaration of model variable '%s'.", szMsg);
    break;

  case RE_DUPSECT:
    printf("Only one '%s' section is allowed.", szMsg);
    break;

  case RE_OUTOFMEM:
    printf("Out of memory in %s() !", szMsg);
    break;

  case RE_REDEF:
    printf("'%s' redefined.", szMsg);
    break;

  case RE_EQNTOOLONG:
    printf("Equation is too long.  Possibly missing terminator.");
    break;

  case RE_BADSTATE:
    printf("Invalid state identifier '%s'.", szMsg);
    break;

  case RE_UNDEFINED:
    printf("Undefined identifier '%s'.", szMsg);
    break;

  case RE_NOINPDEF:
    printf("Input '%s' is not initialized.", szMsg);
    break;

  case RE_NODYNEQN:
    printf("State variable '%s' has no dynamics.", szMsg);
    break;

  case RE_NOOUTPUTEQN:
    printf("Output variable '%s' is not computed anywhere.", szMsg);
    break;

  case RE_TOOMANYVARS:
    printf("Too many %s declarations. Limit is %d.", szMsg, 
            *(PINT)szAltMsg);
    break;

  case RE_POSITIVE:
    printf("Positive number expected.");
    break;

  case RE_NAMETOOLONG:
    printf("Name %s exceed %d characters.", szMsg, MAX_NAME);
    break;

  case RE_UNBALPAR:
    printf("Unbalanced () or equation too long at this line or above.");
    break;

  case RE_NOEND:
    printf("End keyword is missing in file %s.", szMsg);
    break;

  } /* switch */

  printf("\n");
  if (szAltMsg && wCode != RE_LEXEXPECTED)
    printf("%s\n", szAltMsg);

  if (bFatal) {
    printf("One or more fatal errors: Exiting...\n\n");
    exit (wCode);
  } /* if */

} /* ReportError */

