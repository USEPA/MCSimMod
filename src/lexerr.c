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
#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexerr.h"

/* ---------------------------------------------------------------------------
ReportError

   Reports error iCode to terminal (one of RE_) and optional
   szMessage. If iSeverity is set to RE_FATAL, exits program.
*/

int ReportError(PINPUTBUF pibIn, WORD wCode, PSTR szMsg, PSTR szAltMsg) {
  char cNull = '\0';
  BOOL bFatal = wCode & RE_FATAL;
  BOOL bWarning = wCode & RE_WARNING;

  wCode &= ~(RE_FATAL | RE_WARNING);

  if (!szMsg) {
    szMsg = &cNull;
  }

  if (wCode) {
    if (bWarning) {
      Rprintf("*** Warning: ");
    } else {
      Rprintf("*** Error: ");
      bFatal |= (pibIn && (pibIn->cErrors++ > MAX_ERRORS));
    } /* else */
  } /* if */

  if (pibIn) {
    if (pibIn->pfileIn || pibIn->iLNPrev) { /* Line number is valid */
      Rprintf("line %d: ", pibIn->iLineNum);
    } else {
      if (wCode != RE_FILENOTFOUND) { /* Dummy pibIn, show buffer */
        PSTRLEX szTmp;
        szTmp[MAX_LEX - 1] = '\0';
        Rprintf("'%s'...\n  ", strncpy(szTmp, pibIn->pbufOrg, MAX_LEX - 1));
      } /* if */
    }
  }

  switch (wCode) {

  case 0:
    break;

  default:
    Rprintf("Unknown error code %x: %s", wCode, szMsg);

  case RE_INIT:
    Rprintf("Initialization error.");
    break;

  case RE_FILENOTFOUND:
    Rprintf("File not found \"%s\".", szMsg);
    break;

  case RE_CANNOTOPEN:
    Rprintf("Cannot open file \"%s\".", szMsg);
    break;

  case RE_UNEXPECTED:
    Rprintf("Unexpected character '%c' in input file.", *szMsg);
    break;

  case RE_UNEXPESCAPE:
    Rprintf("Unexpected escape sequence '%s' in input file.", szMsg);
    break;

  case RE_UNEXPNUMBER:
    Rprintf("Unexpected number %s in input file.", szMsg);
    break;

  case RE_EXPECTED:
    Rprintf("Expected '%c' before '%c'.", szMsg[1], szMsg[0]);
    break;

  case RE_LEXEXPECTED:
    Rprintf("Expected <%s>", szMsg);
    if (szAltMsg) {
      Rprintf(" before '%s'", szAltMsg);
    }
    break;

    /* USER error handling -- Add user error reporting below */

    /* Model generator errors */

  case RE_BADCONTEXT:
    Rprintf("'%s' used in invalid context.", szMsg);
    break;

  case RE_DUPDECL:
    Rprintf("Duplicate declaration of model variable '%s'.", szMsg);
    break;

  case RE_DUPSECT:
    Rprintf("Only one '%s' section is allowed.", szMsg);
    break;

  case RE_OUTOFMEM:
    Rprintf("Out of memory in %s() !", szMsg);
    break;

  case RE_REDEF:
    Rprintf("'%s' redefined.", szMsg);
    break;

  case RE_EQNTOOLONG:
    Rprintf("Equation is too long.  Possibly missing terminator.");
    break;

  case RE_BADSTATE:
    Rprintf("Invalid state identifier '%s'.", szMsg);
    break;

  case RE_UNDEFINED:
    Rprintf("Undefined identifier '%s'.", szMsg);
    break;

  case RE_NOINPDEF:
    Rprintf("Input '%s' is not initialized.", szMsg);
    break;

  case RE_NODYNEQN:
    Rprintf("State variable '%s' has no dynamics.", szMsg);
    break;

  case RE_NOOUTPUTEQN:
    Rprintf("Output variable '%s' is not computed anywhere.", szMsg);
    break;

  case RE_TOOMANYVARS:
    Rprintf("Too many %s declarations. Limit is %d.", szMsg, *(PINT)szAltMsg);
    break;

  case RE_POSITIVE:
    Rprintf("Positive number expected.");
    break;

  case RE_NAMETOOLONG:
    Rprintf("Name %s exceed %d characters.", szMsg, MAX_NAME);
    break;

  case RE_UNBALPAR:
    Rprintf("Unbalanced () or equation too long at this line or above.");
    break;

  case RE_NOEND:
    Rprintf("End keyword is missing in file %s.", szMsg);
    break;

  } /* switch */

  Rprintf("\n");
  if (szAltMsg && wCode != RE_LEXEXPECTED) {
    Rprintf("%s\n", szAltMsg);
  }

  if (bFatal) {
    Rprintf("One or more fatal errors: Exiting...\n\n");
    return EXIT_ERROR;
  } /* if */
  return 0;
} /* ReportError */
