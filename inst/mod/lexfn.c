/* lexfn.c

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

   Handles lexical parsing for functions.
   
   This file contains the GetInputFn() routine and the definition of
   input functions which are shared between the model code generator
   facility and the simulation input routines.
   
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "lex.h"
#include "lexfn.h"
#include "strutil.h"
#include "lexerr.h"

#include "mod.h"


/* Macros */

#define IsIdentifier(sz)  ((sz) ? isalpha(*(sz)) || *(sz) == '_' : FALSE)

HANDLE CalculateVarHandle (PVMMAPSTRCT pvm, PSTR sz);
static PVMMAPSTRCT vpvmGlo; /* Global symbol table (yech) */

#define GetParmHandle(sz)  (CalculateVarHandle(vpvmGlo, (sz)))


/* Keyword Map Structure */

typedef struct tagINPUTFUNCTIONMAP {

  PSTR szName;
  int  iIFNType; /* Input function type */

} IFM, *PIFM; /* Input function map */


IFM vrgifmMap[] = { /* Input function map */

  {"PerDose", IFN_PERDOSE},  /* Predef'd Input functions */
  {"PerRate", IFN_PERRATE},
  {"PerExp",  IFN_PEREXP},
  {"NDoses",  IFN_NDOSES},
  {"", IFN_NULL}  /* End flag */

};  /* vrgifnMap[] = */


/* ----------------------------------------------------------------------------
 GetFnType
   
   Returns the code of the szKeyword given.  If the string is not
   a valid keyword or abbreviation, returns 0.
*/

int GetFnType (PSTR szName)
{
  PIFM pifm = &vrgifmMap[0];
  
  while (*pifm->szName && MyStrcmp (szName, pifm->szName))
    pifm++;
  
  return (pifm->iIFNType); /* Return Keyword Code or 0 */

} /* GetFnType */



/* ----------------------------------------------------------------------------
   InitIFN
   
   Initialize an input function.
*/
void InitIFN (PIFN pifn)
{
    pifn->dTStartPeriod = 0.0;
    pifn->bOn = FALSE;

    pifn->dMag   = 0.0; /* Initialize parameters */
    pifn->dTper  = 0.0;
    pifn->dT0  = 0.0;
    pifn->dTexp  = 0.0;
    pifn->dDecay = 0.0;
    pifn->dVal   = 0.0;
    pifn->nDoses = 0;

    pifn->hMag   = 0; /* Initialize handles to parameters */
    pifn->hTper  = 0;
    pifn->hT0    = 0;
    pifn->hTexp  = 0;
    pifn->hDecay = 0;
    
    pifn->nDoses = pifn->iDoseCur = 0;
    pifn->rgT0s = pifn->rgTexps = pifn->rgMags = NULL;

}  /* InitIFN */
    

/* ----------------------------------------------------------------------------
   DefDepParm
   
   Defines a parameter or a handle to a model parameter on which the
   parameter is dependent.  Returns TRUE on success.

   NOTE: The call to GetParmHandle() is actually a macro.
   
   If it is the simulation, GetParmHandle() should inquire
   into the symbol table for a handle to the dependent variable.
   If it is the model generator, the symbol table has not yet
   been created and so we actually have to calculate what the
   handle will be.
*/
BOOL DefDepParm (PSTR szLex, PDOUBLE pdValue, HANDLE *phvar)
{
  BOOL bReturn = TRUE;
  
  if (IsIdentifier(szLex)) { /* Define handle to model parameter */

    if (!(*phvar = (HANDLE) GetParmHandle(szLex))) {
      bReturn = FALSE;
      ReportError (NULL, RE_UNDEFINED, szLex, NULL);
    } /* if */
  } /* if */
  else
    *pdValue = atof(szLex); /* Define actual parm from number */

  return (bReturn);

} /* DefDepParm */


/* ----------------------------------------------------------------------------
   GetInputsArgs
   
   Reads the arguments to the input function pifn.
   
   * If the argument is a number, the parameter is defined.

   * If the argument is an identifier which is a valid model
     parameter, the input function parameter is defined as
     being dependent on this model parameter.  The input parameter
     will be defined in the first UpdateInputs() call after model
     initialization.
*/
BOOL GetInputArgs (PINPUTBUF pibIn, PIFN pifn)
{
  PSTRLEX rgszLex[4];
  int rgiTypes[4], i;
  long rgiLowerB[4], rgiUpperB[4];
  BOOL bReturn = FALSE;

  for (i = 0; i < 4; i++)
    rgiTypes[i] = LX_INTEGER | LX_FLOAT | LX_IDENTIFIER;

  if (GetFuncArgs (pibIn, 4, rgiTypes, rgszLex[0], rgiLowerB, rgiUpperB)) {

    for (i = 0; i < 4; i++)
      if ((rgiLowerB[i] != -1) || (rgiUpperB[i] != -1))
        ReportError (pibIn, RE_BADCONTEXT | RE_FATAL, "array bounds", 
                     "Arrays cannot be used as input function parameters");
    
    /* Try to get each parm to show all errors */
    bReturn = TRUE;
    bReturn &= DefDepParm (rgszLex[0], &pifn->dMag, &pifn->hMag);
    bReturn &= DefDepParm (rgszLex[1], &pifn->dTper, &pifn->hTper);
    bReturn &= DefDepParm (rgszLex[2], &pifn->dT0, &pifn->hT0);

    if (pifn->iType == IFN_PEREXP)
      bReturn &= DefDepParm (rgszLex[3], &pifn->dDecay, &pifn->hDecay);
    else
      bReturn &= DefDepParm (rgszLex[3], &pifn->dTexp, &pifn->hTexp);
    
    if (!bReturn)
      ReportError (pibIn, RE_EXPECTED, "input-spec", NULL);
  } /* if */

  return (bReturn);

} /* GetInputArgs */


/* ----------------------------------------------------------------------------
   GetNNumbers
   
   Tries to read nNumbers from pibIn.  Returns TRUE on error.
*/
BOOL GetNNumbers (PINPUTBUF pibIn, PSTR szLex, int nNumbers, PDOUBLE rgd)
{
  BOOL bErr = FALSE;
  int i;
  
  for (i = 0; i < nNumbers  && !bErr; i++) {
    if (i)
      GetOptPunct (pibIn, szLex, ',');
    if (!(bErr = ENextLex (pibIn, szLex, LX_NUMBER)))
      rgd[i] = atof(szLex);
  } /* for */

  return bErr;

} /* GetNNumbers */


/* ----------------------------------------------------------------------------
   GetNDoses
   
   Reads the arguments for the NDoses() input type.  Return TRUE if
   the structure is defined, FALSE on error.
*/
BOOL GetNDoses (PINPUTBUF pibIn, PSTR szLex, PIFN pifn)
{
  BOOL bErr = FALSE; /* Return value flags error condition */

  if ((bErr = EGetPunct (pibIn, szLex, CH_LPAREN)))
    goto Exit_GetNDoses;

  /* Get positive integer number of doses */
  if ((bErr = ENextLex (pibIn, szLex, LX_INTEGER)))
    goto Exit_GetNDoses;

  pifn->nDoses = atoi(szLex);

  if ((bErr = (pifn->nDoses <= 0))) {
    ReportError (pibIn, RE_LEXEXPECTED, "positive-integer", szLex);
    goto Exit_GetNDoses;
  } /* if */

  if (!(pifn->rgT0s = (PDOUBLE) malloc (pifn->nDoses * sizeof(double))) || 
      !(pifn->rgTexps = (PDOUBLE) malloc (pifn->nDoses * sizeof(double))) ||
      !(pifn->rgMags = (PDOUBLE) malloc (pifn->nDoses * sizeof(double))))
    ReportError (pibIn, RE_OUTOFMEM | RE_FATAL, "GetNDoses", NULL);
  
  /* Try to get doses list: n Mag's, n T0's, n Texp's */
  GetOptPunct (pibIn, szLex, ',');
  bErr = GetNNumbers (pibIn, szLex, pifn->nDoses, pifn->rgMags);

  GetOptPunct (pibIn, szLex, ',');
  if (!bErr)
    bErr = GetNNumbers (pibIn, szLex, pifn->nDoses, pifn->rgT0s);

  GetOptPunct (pibIn, szLex, ',');
  if (!bErr)
    bErr = GetNNumbers (pibIn, szLex, pifn->nDoses, pifn->rgTexps);

  if (!bErr)
    bErr = EGetPunct (pibIn, szLex, CH_RPAREN);

Exit_GetNDoses:
  
  if (bErr)
    fprintf (stderr, "Syntax: GetNDoses (nDoses, <n Magnitudes>, "
                     "<n T0's>, <n Texposure's>)\n");

  return (!bErr);

} /* GetNDosesArgs */


/* ----------------------------------------------------------------------------
   GetInputFn
   
   Attempts to define an IFN structure pifn according to the
   input spec in sz if sz is non-NULL, or pibIn if sz is NULL.
   Returns TRUE if the structure is defined.
*/
BOOL GetInputFn (PINPUTBUF pibIn, PSTR sz, PIFN pifn)
{
  INPUTBUF ibDummy;
  PINPUTBUF pibDum = &ibDummy;
  PSTRLEX szLex;
  int iType;
  BOOL bReturn = FALSE;
  
  if (!pibIn || !pifn)
    return (FALSE);
  
  /* Define global variable map (yech) */
  {
    PINPUTINFO pinfo = (PINPUTINFO) pibIn->pInfo;
    
    vpvmGlo = pinfo->pvmGloVars;
  } /* block */
  
  if (sz)
    MakeStringBuffer (pibIn, pibDum, sz);
  else
    pibDum = pibIn;

  NextLex (pibDum, szLex, &iType);
  switch (iType) {
    default:
    case LX_NULL:
    case LX_PUNCT:
      ReportError (pibIn, RE_LEXEXPECTED, "input-spec", NULL);
      break;
  
    case LX_FLOAT:
    case LX_INTEGER:
    case LX_IDENTIFIER:
      InitIFN (pifn);

      if (iType == LX_IDENTIFIER) {
        pifn->iType = GetFnType (szLex);
        switch (pifn->iType) {
          case IFN_NDOSES:
            bReturn = GetNDoses (pibDum, szLex, pifn);
            break;
 
          default:
            pifn->iType = IFN_NULL;
            ReportError (pibIn, RE_LEXEXPECTED, "input-spec", szLex);
            break;

          case IFN_PERDOSE:
          case IFN_PERRATE:
          case IFN_PEREXP:
            bReturn = GetInputArgs (pibDum, pifn);
            break;
 
        } /* switch pifn-> iType */
      } /* if identifier */

      else {
        pifn->iType = IFN_CONSTANT;
        pifn->dMag = pifn->dVal = atof (szLex);
        pifn->bOn = TRUE;
        bReturn = TRUE;
      } /* else */
    break;
    
  } /* switch iType */

  return (bReturn);

}  /* GetInputFn */
