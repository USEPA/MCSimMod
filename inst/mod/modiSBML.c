/* modiSBML.c

   Copyright (c) 2007-2017. Free Software Foundation, Inc.

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

   Handles input parsing of the SBML Model Definition Files and of the
   template model (if used).
   Primitive, but does not require libSBML.
*/

#include "config.h"

/* if config.h defines HAVE_LIBSBML this file is NOT compiled */

#ifndef HAVE_LIBSBML

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "lexerr.h"
#include "mod.h"
#include "modi.h"
#include "modiSBML.h"
#include "modd.h"
#include "modo.h"
#include "strutil.h"


/* ----------------------------------------------------------------------------
   Private Constants
*/

/* SBML Keyword Map Constants */

#define KM_MODEL         1
#define KM_CPTLIST       2
#define KM_CPT           3
#define KM_SPECIESLIST   4
#define KM_SPECIE        5
#define KM_SPECIES       6
#define KM_PARAMS        7
#define KM_PARAM         8
#define KM_REACTIONS     9
#define KM_SBML          10
#define KM_RULESLIST     15
#define KM_RATERULE      16
#define KM_FUNCLIST      17
#define KM_FUNC          18
#define KM_REACTION      20
#define KM_REACTANTS     30
#define KM_PRODUCTS      40
#define KM_SPECIES_REF   50
#define KM_STOICHIO      51
#define KM_LAW           60
#define KM_MATH          61
#define KM_APPLY         62
#define KM_CI            63
#define KM_PLUS          64
#define KM_MINUS         65
#define KM_TIMES         66
#define KM_DIVIDE        67
#define KM_POWER         68


/* ---------------------------------------------------------------------------
   Private Enums */

typedef enum {
  product,
  reactant,
  parameter
} VARTYPES;

typedef enum {
  plus,
  minus
} OPSIGNS;


/* ---------------------------------------------------------------------------
   Private Typedefs */

/* pack what is needed for SetVar */

typedef struct tagFORSV {
  PINPUTBUF pibIn;
  PSTR szName;
  PSTR szVal;
  PVMMAPSTRCT pTarget;
} FORSV, *PFORSV;


/* ---------------------------------------------------------------------------
   Private Predefined Structures */

/* SBML Keyword Map Structure */

KM vrgSBMLKW[] = {
  {"sbml",                KM_SBML,       CN_SBML},
  {"model",               KM_MODEL,      CN_GLOBAL},

   {"listOfFunctionDefinitions", KM_FUNCLIST,    CN_GLOBAL},
    {"functionDefinition",       KM_FUNC,        CN_GLOBAL},

   {"listOfCompartments", KM_CPTLIST,    CN_GLOBAL},
    {"compartment",       KM_CPT,        CN_GLOBAL},

   {"listOfSpecies",      KM_SPECIESLIST,CN_GLOBAL},
    {"specie",            KM_SPECIES,    CN_GLOBAL},
    {"species",           KM_SPECIES,    CN_GLOBAL},

   {"listOfParameters",   KM_PARAMS,     CN_GLOBAL},
    {"parameter",         KM_PARAM,      CN_GLOBAL},

   {"listOfRules",        KM_RULESLIST,  CN_GLOBAL},
    {"rateRule",          KM_RATERULE,   CN_GLOBAL},

   {"listOfReactions",    KM_REACTIONS,  CN_GLOBAL},
    {"reaction",          KM_REACTION,   CN_GLOBAL},
     {"listOfReactants",  KM_REACTANTS,  CN_GLOBAL},
     {"listOfProducts",   KM_PRODUCTS,   CN_GLOBAL},
      {"specieReference", KM_SPECIES_REF,CN_GLOBAL},
      {"speciesReference",KM_SPECIES_REF,CN_GLOBAL},
      {"stoichiometry",   KM_STOICHIO,   CN_GLOBAL},
     {"kineticLaw",       KM_LAW,        CN_GLOBAL},
      {"math",            KM_MATH,       CN_GLOBAL},
       {"apply",          KM_APPLY,      CN_GLOBAL},
        {"plus",          KM_PLUS,       CN_APPLY},
        {"minus",         KM_MINUS,      CN_APPLY},
        {"times",         KM_TIMES,      CN_APPLY},
        {"divide",        KM_DIVIDE,     CN_APPLY},
        {"power",         KM_POWER,      CN_APPLY},
        {"ci",            KM_CI,         CN_APPLY},

  {"", 0, CN_ALL} /* This marks the end of map */
  
}; /* vrgSBMLKW[] */


/* ----------------------------------------------------------------------------
   AugmentEquation
   
   Augment the equation field of PVMMAPSTRCT with the equation szEqn by
   simple concatenation.
*/
void AugmentEquation (PVMMAPSTRCT pvm, PSTR szEqn, PSTR szStoi, OPSIGNS sign)
{
  PSTR szBuf;
  PSTRLEX szSymbol;
  
  if (!pvm || !szEqn || !szStoi)
    return;

    if (sign == plus)
      sprintf (szSymbol, " + ");

    if (sign == minus)
      sprintf (szSymbol, " - ");

    if ((szBuf = (PSTR) malloc (strlen(pvm->szEqn) + strlen(szSymbol) + 
                                strlen(szStoi) + strlen(szEqn) + 5)))
      if (!strcmp(szStoi, "1"))
        sprintf (szBuf, "%s%s%s", pvm->szEqn, szSymbol, szEqn);
      else
        sprintf (szBuf, "%s%s %s * %s", pvm->szEqn, szSymbol, szStoi, szEqn);

    else
      ReportError (NULL, RE_OUTOFMEM | RE_FATAL, szEqn,
                   "* .. defining equation in AugmentEquation");

  if (pvm->szEqn)
    free (pvm->szEqn);

  pvm->szEqn = szBuf;
  
} /* AugmentEquation */


/* ----------------------------------------------------------------------------
   ConstructEqn

   Contruct a differential equation for each species (reactant or product) 
   of a given reaction. Reactions are supposed to have been pre-defined as
   local variables. Products should call this routine with "product"
   as last argument; Reactants should use "reactant".
*/
void ConstructEqn (PINPUTBUF pibIn, PSTR szRName, VARTYPES eType)
{
  int iLexType;
  HANDLE  hType;
  PSTRLEX szSName;
  PSTRLEX szStoichio;
  PVMMAPSTRCT pvm;
  PINPUTINFO pinfo = (PINPUTINFO) pibIn->pInfo;

  /* get species name */
  while (*pibIn->pbufCur++ != '"'); /* this assumes that name comes fist */
  GetIdentifier (pibIn, szSName);

  /* get the stoichiometry if it is there, otherwise assume 1 */
  *pibIn->pbufCur = *pibIn->pbufCur + 1; /* pass closing '"' of name */
  while ((*pibIn->pbufCur != '"') && (*pibIn->pbufCur != '>')) {
    *pibIn->pbufCur++;
  }
  if (*pibIn->pbufCur == '"') {
    *pibIn->pbufCur++;
    GetNumber (pibIn, szStoichio, &iLexType);
  }
  else sprintf (szStoichio, "1");
  printf ("%s stoichio: %s\n", szSName, szStoichio);

  /* reactions are supposed to happen in the one compartment defined:
     pad species name with that compartment name */
  if (!GetVarPTR (pinfo->pvmGloVars, szSName))
    sprintf (szSName, "%s_%s", szSName, pinfo->pvmLocalCpts->szName);

  /* padded species should have been declared State variables or 
     parameters if they were set to boundary conditions */
  hType = GetVarType (pinfo->pvmGloVars, szSName);
  if (hType == ID_STATE) { /* deal with differential */
    /* construct equation */
    pvm = GetVarPTR (pinfo->pvmDynEqns, szSName);
    if (!pvm) {
      /* no dynamic equation yet defined for szSName, create one */
      DefineVariable (pibIn, szSName, "", KM_DXDT);
      /* pvmDynEqns has been initialized, refresh */
      pvm = GetVarPTR (pinfo->pvmDynEqns, szSName); 
    }
    AugmentEquation (pvm, szRName, szStoichio, 
                     (eType == reactant ? minus : plus));
  }
  else {
    if (hType != ID_PARM)
      ReportError (NULL, RE_BADSTATE | RE_FATAL, szSName, NULL);
  }
} /* ConstructEqn */


/* ----------------------------------------------------------------------------
   CountLines

   Count the actual lines of a stream (which must be already open) and reset
   the stream. 
*/
long CountLines (PFILE pFileIn)
{
  int  nLines = 0;
  char szDummy[2];

  /* skip the first line of comments in pFileIn */
  fscanf (pFileIn,  "%*[^\n]"); getc(pFileIn);

  /* keep reading lines as long as we have not reached eof */
  while ( !(feof(pFileIn))) {
    if (fscanf (pFileIn, "%1s", szDummy) > 0)
      nLines++;
    fscanf (pFileIn, "%*[^\n]"); getc(pFileIn); /* throw away rest of line */
  }

  rewind (pFileIn);

  return (nLines);

} /* CountLines */


/* ----------------------------------------------------------------------------
   GetSBMLKeywordCode
   
   Returns the code of the szKeyword given. If the string is not
   a valid keyword or abbreviation, returns 0.
*/
int GetSBMLKeywordCode (PSTR szKeyword)
{
  PKM pkm = &vrgSBMLKW[0];
  
  while (*pkm->szKeyword && strcmp (szKeyword, pkm->szKeyword))
    pkm++;
  
  return (pkm->iKWCode); /* Return Keyword Code or 0 */

} /* GetSBMLKeywordCode */


/* ---------------------------------------------------------------------------
   GetSBMLLex

   Parse a SBML buffer from the current position on, looking for a keyword 
   with given code. 
   Return with the buffer pointer set at the caracter following 
   the first keyword found. Never pass the </X> tag where X is determined by 
   iEnding. 
   Return 1 if the keyword is found, 0 if not found.
*/
int GetSBMLLex (PINPUTBUF pibIn, int iEnding, int iKWCode)
{
  PSTRLEX szLex;
  char c;
  BOOL bDone = FALSE;
  BOOL bFound = FALSE;

  if (!pibIn)
    return (0);

  if (!pibIn->pbufCur)
    return (0);

  while (!bDone) {
    c = *pibIn->pbufCur;
    if (c == '<') { /* found an opening bracket */
      szLex[0] = *pibIn->pbufCur++;
      c = *pibIn->pbufCur; /* get character after '<' */
      if ((c == '!') || (c == '?') || (c == '/')) {
        if (c == '/') { /* ending tag, do not go beyond </iEnding> */
	  szLex[0] = *pibIn->pbufCur++;
          GetIdentifier (pibIn, szLex);
          bDone = (GetSBMLKeywordCode (szLex) == iEnding);
        }
        else /* '<' followed by '!' or '?' */
          while (*pibIn->pbufCur++ != '>'); /* eat comment or section */
      }
      else { /* opening bracket not followed by '!', '?' or '/' */
        GetIdentifier (pibIn, szLex);
        bFound = (iKWCode == GetSBMLKeywordCode (szLex));
        bDone = bFound;
      } /* end else */
    } /* end if c == '<' */
    else { /* c is not '<' */
      if (c == 0)
        bDone = TRUE;
      else
        pibIn->pbufCur = pibIn->pbufCur + 1;
    }
  } /* end while */

  return (bFound);

} /* GetSBMLLex */


/* ----------------------------------------------------------------------------
   SetVar
   
   Declare a global variable and link its value to it. 
*/
void SetVar (PINPUTBUF pibIn, PSTR szName, PSTR szVal, HANDLE hType)
{
  PVMMAPSTRCT pvm;
  PINPUTINFO pinfo = (PINPUTINFO) pibIn->pInfo;
  int iKWCode;

  if (!(GetVarPTR (pinfo->pvmGloVars, szName))) { /* New id */

    iKWCode = ((hType ==    ID_STATE  ? KM_STATES /* Translate to KW_ */
                : (hType == ID_INPUT  ? KM_INPUTS
                : (hType == ID_OUTPUT ? KM_OUTPUTS
                                     : KM_NULL))));

    if ((hType == ID_PARM) || 
        (hType == (ID_LOCALDYN     | ID_SPACEFLAG)) ||
        (hType == (ID_LOCALCALCOUT | ID_SPACEFLAG)) ||
        (hType == (ID_LOCALSCALE   | ID_SPACEFLAG))) {
      AddEquation (&pinfo->pvmGloVars, szName, szVal, hType);
      if (hType == ID_PARM)
        printf ("param.   %s = %s\n", szName, szVal);
    }
    else {
      DeclareModelVar (pibIn, szName, iKWCode);

      /* link value to symbol */
      pvm = GetVarPTR (pinfo->pvmGloVars, szName);
      DefineGlobalVar (pibIn, pvm, szName, szVal, hType);

      if (hType == ID_STATE)
        printf ("species  %s = %s\n", szName, szVal);

      if (hType == ID_INPUT)
        printf ("input    %s = %s\n", szName, szVal);

      if (hType == ID_OUTPUT)
        printf ("output   %s = %s\n", szName, szVal);
    }
  }

} /* SetVar */


/* ----------------------------------------------------------------------------
   Create1Var
  
   Get the name of the PK template variable stored in pvm. If it starts
   with '_' (underscore) write the name of the SBML species passed in pInfo 
   at the beginning of the name. Then define the variable.
   This is a callback function for ForAllVar().
*/
int Create1Var (PFILE pfile, PVMMAPSTRCT pvm, PVOID pInfo)
{
  PFORSV     pV = (PFORSV) pInfo;
  PSTRLEX    szTmp;

  if (pvm->szName[0] == '_') {
    /* extend the variable name with the compartment name */
    sprintf (szTmp, "%s%s", pV->szName, pvm->szName);
    if (pvm->hType == (ID_LOCALDYN | ID_SPACEFLAG))
      SetVar (pV->pibIn, szTmp, pvm->szEqn, pvm->hType);
    else
      SetVar (pV->pibIn, szTmp, pV->szVal, pvm->hType);
  }
  else /* copy the PK template variable as is */
    SetVar (pV->pibIn, pvm->szName, pvm->szEqn, pvm->hType);

  return (1);

} /* Create1Var */


/* ----------------------------------------------------------------------------
   Transcribe1AlgEqn

   Copy with eventual modification an equation from the temporary info 
   structure of mod (filled in by ReadPKTemplate) to the primary info 
   structure of mod.
   Skip equations of type ID_DERIV.
   Get the name of the PK template variable stored in pvm. If it starts
   with '_' (underscore) write the name of the SBML species passed in pInfo 
   at the beginning of the name. Do the same on all terms of the equation.
   Then register the equation in the primary info structure of mod.
   This is a callback function for ForAllVar().
*/
int Transcribe1AlgEqn (PFILE pfile, PVMMAPSTRCT pvm, PVOID pInfo)
{
  PFORSV      pV = (PFORSV) pInfo;
  PSTRLEX     szTmpName = "";
  PSTREQN     szTmpEq = "";
  INPUTBUF    ibDummy;
  PSTRLEX     szLex;
  int         iType;

  if (pvm->szName[0] == '_') {
    /* extend the variable name with the compartment name */
    sprintf (szTmpName, "%s%s", pV->szName, pvm->szName);
  }
  else sprintf (szTmpName, "%s", pvm->szName); /* simple copy */

  /* deal with the equation */
  MakeStringBuffer (NULL, &ibDummy, pvm->szEqn);
  
  while (!EOB(&ibDummy)) {

    NextLex (&ibDummy, szLex, &iType); /* ...all errors reported */

    if ((iType == LX_IDENTIFIER ) && !(IsMathFunc (szLex)) && 
        (szLex[0] == '_'))
      sprintf (szTmpEq, "%s%s%s", szTmpEq, pV->szName, szLex); /* extend */
    else
      sprintf (szTmpEq, "%s%s", szTmpEq, szLex);
  
  } /* while */

  if (!(GetVarPTR (pV->pTarget, szTmpName))) { /* New id */
    if (pvm->hType < ID_DERIV) {
      DefineVariable (pV->pibIn, szTmpName, szTmpEq, KM_NULL);
      printf ("local v. %s = %s\n", szTmpName, szTmpEq);
    }
    else {
      if (pvm->hType == ID_INLINE) {
        DefineVariable (pV->pibIn, szTmpName, szTmpEq, KM_INLINE);
        printf ("inline   %s\n", szTmpEq);
      }
    }
  }

  return (1);

} /* Transcribe1AlgEqn */


/* ----------------------------------------------------------------------------
   Transcribe1DiffEqn

   Copy with eventual modification a differential equation from the temporary
   info structure of mod (filled in by ReadPKTemplate) to the primary info 
   structure of mod.
   Process only equations of type ID_DERIV.
   Get the name of the PK template variable stored in pvm. If it starts
   with '_' (underscore) write the name of the SBML species passed in pInfo 
   at the beginning of the name. Do the same on all terms of the equation.
   Then register the equation in the primary info structure of mod.
   This is a callback function for ForAllVar().
*/
int Transcribe1DiffEqn (PFILE pfile, PVMMAPSTRCT pvm, PVOID pInfo)
{
  PFORSV      pV = (PFORSV) pInfo;
  PSTRLEX     szTmpName = "";
  PSTREQN     szTmpEq = "";
  INPUTBUF    ibDummy;
  PSTRLEX     szLex;
  int         iType;

  if ((pvm->hType & ID_TYPEMASK) != ID_DERIV)
    return (0);

  if (pvm->szName[0] == '_') {
    /* extend the variable name with the compartment name */
    sprintf (szTmpName, "%s%s", pV->szName, pvm->szName);
  }
  else sprintf (szTmpName, "%s", pvm->szName); /* simple copy */

  /* deal with the equation */
  MakeStringBuffer (NULL, &ibDummy, pvm->szEqn);

  while (!EOB(&ibDummy)) {

    NextLex (&ibDummy, szLex, &iType); /* ...all errors reported */

    if ((iType == LX_IDENTIFIER ) && !(IsMathFunc (szLex)) && 
        (szLex[0] == '_'))
      sprintf (szTmpEq, "%s%s%s", szTmpEq, pV->szName, szLex); /* extend */
    else
      sprintf (szTmpEq, "%s%s", szTmpEq, szLex);
  
  } /* while */

  if (!(GetVarPTR (pV->pTarget, szTmpName))) { /* New id */
      DefineVariable (pV->pibIn, szTmpName, szTmpEq, KM_DXDT);
      printf ("template ODE term for %s = %s\n", szTmpName, szTmpEq);
  }

  return (1);

} /* Transcribe1DiffEqn */


/* ----------------------------------------------------------------------------
   ReadCpt
   
   Read an SBML compartment tag content. Skip the automatic external 
   compartment named 'compartment'. Print the name and value of the compartment
   if bTell is TRUE.
*/
void ReadCpt (PINPUTBUF pibIn, BOOL bTell)
{
  PSTRLEX szName;
  PSTREQN szEqn;
  int iLexType;
  PINPUTINFO pinfo = (PINPUTINFO) pibIn->pInfo;

  pinfo->wContext = CN_GLOBAL;

  while (*pibIn->pbufCur++ != '"'); /* this assumes that name comes fist */

  GetIdentifier (pibIn, szName);

  if (!strcmp (szName, "compartment")) /* do not register the external cpt */ 
    return;

  if (!(GetVarPTR (pinfo->pvmLocalCpts, szName))) {

    /* New compartment */

    *pibIn->pbufCur = *pibIn->pbufCur + 1; /* pass closing '"' of name */
    while (*pibIn->pbufCur++ != '"'); /* this assumes that value comes next */

    GetNumber (pibIn, szEqn, &iLexType);
    if (!iLexType) /* no value, assign 0 by default */
      sprintf (szEqn, "0.0");

    /* link value to symbol */
    AddEquation (&pinfo->pvmLocalCpts, szName, szEqn, ID_COMPARTMENT);

    if (bTell)
      printf ("compart. %s = %s\n", szName, szEqn);

  } /* end if */

  while (*pibIn->pbufCur++ != '>'); /* go to end of tag */

} /* ReadCpt */


/* ----------------------------------------------------------------------------
   ReadCpts
   
   Read an SBML list of compartements section.
*/
void ReadCpts (PINPUTBUF pibIn, BOOL bTell)
{
  PINPUTINFO pinfo = (PINPUTINFO) pibIn->pInfo;

  /* reset the compartment list! */
  pinfo->pvmLocalCpts = NULL;

  while (GetSBMLLex (pibIn, KM_CPTLIST, KM_CPT))
    ReadCpt (pibIn, bTell);

} /* ReadCpts */


/* ----------------------------------------------------------------------------
   ReadFunction
   
   Read a function definition section in the given SBML level 2 buffer.
*/
void ReadFunction (PINPUTBUF pibIn)
{
  PSTRLEX szRName;
  PSTREQN szEqn;
  int bInited = FALSE;
  PINPUTINFO pinfo = (PINPUTINFO) pibIn->pInfo;

  /* set context to Dynamics section */
  pinfo->wContext = CN_DYNAMICS;

  while (*pibIn->pbufCur++ != '"'); /* this assumes that name comes fist */

  GetIdentifier (pibIn, szRName);

  while (*pibIn->pbufCur++ != '>'); /* go to end of tag */

  /* actually we should get the bvar setup the head and then get the 
     apply to define the body */

  /* get an "apply" tag */
  GetSBMLLex (pibIn, KM_SBML, KM_APPLY);

  ReadApply (pibIn, &bInited, szEqn);

  printf ("rate for %s = %s\n", szRName, szEqn);

  /* define reaction name as Derivative spec in the Dynamics section */ 
  DefineVariable (pibIn, szRName, szEqn, KM_DXDT);

  while (*pibIn->pbufCur++ != '>'); /* go to end of tag */

} /* ReadFunction */


/* ----------------------------------------------------------------------------
   ReadFunctions
   
   Read a list of function definitions section.
*/
void ReadFunctions (PINPUTBUF pibIn, int iSBML_level)
{

  if (iSBML_level == 1)
    printf ("mod: ignoring function definitions in level 1...\n");
  else
    while (GetSBMLLex (pibIn, KM_FUNCLIST, KM_FUNC))
      ReadFunction (pibIn);

} /* ReadFunctions */


/* ----------------------------------------------------------------------------
   ReadDifferentials
   
   Construct a differential equation for each species involved in the 
   reactions of the SBML model stored in the given buffer.
   
*/
void ReadDifferentials (PINPUTBUF pibIn)
{
  PSTRLEX szRName;
  PINPUTINFO pinfo = (PINPUTINFO) pibIn->pInfo;

  /* set context to Dynamics section */
  pinfo->wContext = CN_DYNAMICS;

  while (GetSBMLLex (pibIn, KM_SBML, KM_REACTION)) {

    /* get reaction name */
    while (*pibIn->pbufCur++ != '"'); /* this assumes that name comes fist */
    GetIdentifier (pibIn, szRName);
    while (*pibIn->pbufCur++ != '>'); /* go to end of tag */

    /* go to the reactants' list */
    GetSBMLLex (pibIn, KM_REACTION, KM_REACTANTS);

    while (GetSBMLLex (pibIn, KM_REACTANTS, KM_SPECIES_REF)) {
      /* construct the differential for that reactant */
      ConstructEqn (pibIn, szRName, reactant);
    }

    /* go to the products' list */
    GetSBMLLex (pibIn, KM_REACTION, KM_PRODUCTS);

    while (GetSBMLLex (pibIn, KM_PRODUCTS, KM_SPECIES_REF)) {
      /* construct the differential for that product */
      ConstructEqn (pibIn, szRName, product);
    }

  } /* end while */

} /* ReadDifferentials */


/* ----------------------------------------------------------------------------
   ReadParameter
   
   Read an SBML global parameter tag content and set it up as global variable.
*/
void ReadParameter (PINPUTBUF pibIn)
{
  PSTRLEX szName;
  PSTREQN szEqn;
  int iLexType;
  PVMMAPSTRCT pvm;
  HANDLE hType;
  PINPUTINFO pinfo = (PINPUTINFO) pibIn->pInfo;

  pinfo->wContext = CN_GLOBAL;

  while (*pibIn->pbufCur++ != '"'); /* this assumes that name comes fist */

  GetIdentifier (pibIn, szName);

  if (!(hType = GetVarType (pinfo->pvmGloVars, szName))) { /* New id */

    *pibIn->pbufCur = *pibIn->pbufCur + 1; /* pass closing '"' of name */
    while (*pibIn->pbufCur++ != '"'); /* this assumes that value comes next */

    GetNumber (pibIn, szEqn, &iLexType);
    if (!iLexType) { /* no value, assign 0 by default */
      sprintf (szEqn, "0.0");
    }

    /* link value to symbol */
    DefineGlobalVar (pibIn, pvm, szName, szEqn, hType);

    printf ("param.   %s = %s\n", szName, szEqn);

  } /* end if */

  else { /* the parameter was already defined, this is confusing, exit */
    printf ("***Error: redeclaration of parameter %s\n", szName);
    printf ("Exiting...\n\n");
    exit (0);
  }

  while (*pibIn->pbufCur++ != '>'); /* go to end of tag */

} /* ReadParameter */


/* ----------------------------------------------------------------------------
   ReadParameters
   
   Read an SBML list of parameters section.
*/
void ReadParameters (PINPUTBUF pibIn)
{
  while (GetSBMLLex (pibIn, KM_PARAMS, KM_PARAM))
    ReadParameter (pibIn);

} /* ReadParameters */


/* ----------------------------------------------------------------------------
   ReadReaction_L1
   
   Read an SBML reaction tag in the given SBML level 1 buffer.
*/
void ReadReaction_L1 (PINPUTBUF pibIn)
{
  PSTRLEX szRName;
  PSTREQN szEqn;
  PINPUTINFO pinfo = (PINPUTINFO) pibIn->pInfo;

  /* set context to Dynamics section */
  pinfo->wContext = CN_DYNAMICS;

  while (*pibIn->pbufCur++ != '"'); /* this assumes that name comes fist */

  GetIdentifier (pibIn, szRName);

  while (*pibIn->pbufCur++ != '>'); /* go to end of tag */

  /* get kinetic equation */
  GetSBMLLex (pibIn, KM_SBML, KM_LAW);

  /* assume that formula is next */
  while (*pibIn->pbufCur++ != '"');
 
  /* go back one char */
  pibIn->pbufCur = pibIn->pbufCur - 1;

  GetaString (pibIn, szEqn);

  printf ("reaction %s = %s\n", szRName, szEqn);

  /* define reaction name as a local variable in the Dynamics section */ 
  DefineVariable (pibIn, szRName, szEqn, 0);

  while (*pibIn->pbufCur++ != '>'); /* go to end of tag */

} /* ReadReaction_L1 */


/* ----------------------------------------------------------------------------
   TranscribeOpSymbol
   
   Transcribe the C language symbol of an SMBL reaction name.
*/
void TranscribeOpSymbol (PSTR szOp)
{  
  switch (GetSBMLKeywordCode (szOp)) {

  case KM_PLUS:
    sprintf (szOp, "%s", "+");
    break;

  case KM_MINUS:
    sprintf (szOp, "%s", "-");
    break;

  case KM_TIMES:
    sprintf (szOp, "%s", "*");
    break;

  case KM_DIVIDE:
    sprintf (szOp, "%s", "/");
    break;

  case KM_POWER:
    sprintf (szOp, "%s", "pow");
    break;

  default:
    printf ("***Error: unknown mathXML operation '%s' - exiting...\n\n", szOp);
    exit (0);
  }

} /* TranscribeOpSymbol */


/* ----------------------------------------------------------------------------
   ReadApply
   
   Recursive. Get the content of an <apply> section of MathXML and write it to 
   szEqn.
   The opening <apply> tag is supposed to have been read.
   Note : this rather a hack.
*/
void ReadApply (PINPUTBUF pibIn, PINT bInited, PSTR szEqn)
{
  PSTRLEX szOp;
  PSTRLEX szLex;
  int iKw;
  int ithTerm = 0;
  BOOL bDone = FALSE;
  char c;
  PINPUTINFO pinfo = (PINPUTINFO) pibIn->pInfo;

  /* write an opening '(' to szEqn */
  if (*bInited) /* we are somewhere in an "apply" section: concatenate */
    sprintf (szEqn, "%s(", szEqn);
  else {
    sprintf (szEqn, "("); /* initiate */
    *bInited = TRUE;
  }

  /* get the operation */
  while (*pibIn->pbufCur++ != '<');
  c = *pibIn->pbufCur; /* get character after '<' */
  if (c == '!') /* comment, skip it */
    while (*pibIn->pbufCur++ != '<');

  GetIdentifier (pibIn, szOp);
  TranscribeOpSymbol (szOp);

  do {
    /* try to get the next lex, stop if it's an /apply token */
    while (*pibIn->pbufCur++ != '<');
    c = *pibIn->pbufCur; /* get character after '<' */
    if (c == '/') { /* ending tag, do not go beyond </apply> */
      pibIn->pbufCur++;
      GetIdentifier (pibIn, szLex);
      iKw = GetSBMLKeywordCode (szLex);
      if ((iKw == KM_APPLY) || (iKw == KM_MATH)) {
        sprintf (szEqn, "%s)", szEqn);
        return;
      }
    }
    else /* 'c' is not '/', read item */
      GetIdentifier (pibIn, szLex);

    if (!strcmp (szLex, "apply")) {
      ithTerm++;

      if  (!strcmp(szOp, "pow")) {
        if (ithTerm > 1)
          sprintf (szEqn, "%s)", szEqn);
        else
          sprintf (szEqn, "%s%s(,", szEqn, szOp);
      }
      else {
        if (ithTerm > 1)
        sprintf (szEqn, "%s%s", szEqn, szOp);
      }
      ReadApply (pibIn, bInited, szEqn); /* found "apply", get lower level */
    }
    else { /* szLex == "ci" (hopefully!), get the atoms of the expression */
      do { 
        /* go one char, beyond '>' */
        pibIn->pbufCur = pibIn->pbufCur + 1;
        SkipWhitespace (pibIn);
        GetIdentifier (pibIn, szLex); /* this should be a valid formula atom */

        /* if PK template is used, reactions are supposed to happen in the 
           one compartment defined: pad species name with compartment name */
        if ((pinfo->bTemplateInUse) && (!GetVarPTR (pinfo->pvmGloVars, szLex)))
          sprintf (szLex, "%s_%s", szLex, pinfo->pvmLocalCpts->szName);

        ithTerm++;

        if (!strcmp(szOp, "pow")) {
          if (ithTerm > 1)
            sprintf (szEqn, "%s%s)", szEqn, szLex);
          else
            sprintf (szEqn, "%s%s(%s,", szEqn, szOp, szLex);
        }
        else {
          if (ithTerm > 1)
            sprintf (szEqn, "%s%s%s", szEqn, szOp, szLex);
          else
            sprintf (szEqn, "%s%s", szEqn, szLex);
        }
      }
      while (GetSBMLLex (pibIn, KM_APPLY, KM_CI));
      sprintf (szEqn, "%s)", szEqn);
      return;
    } /* end else */
  } /* end do */
  while (!bDone);

} /* ReadApply */


/* ----------------------------------------------------------------------------
   ReadReaction_L2
   
   Read a reaction tag in the given SBML level 2 buffer.
*/
void ReadReaction_L2 (PINPUTBUF pibIn)
{
  PSTRLEX szRName;
  PSTREQN szEqn;
  int bInited = FALSE;
  PINPUTINFO pinfo = (PINPUTINFO) pibIn->pInfo;

  /* set context to Dynamics section */
  pinfo->wContext = CN_DYNAMICS;

  while (*pibIn->pbufCur++ != '"'); /* this assumes that name comes fist */

  GetIdentifier (pibIn, szRName);

  while (*pibIn->pbufCur++ != '>'); /* go to end of tag */

  /* get an "apply" tag */
  GetSBMLLex (pibIn, KM_SBML, KM_APPLY);

  ReadApply (pibIn, &bInited, szEqn);

  printf ("reaction %s = %s\n", szRName, szEqn);

  /* define reaction name as a local variable in the Dynamics section */ 
  DefineVariable (pibIn, szRName, szEqn, 0);

  while (*pibIn->pbufCur++ != '>'); /* go to end of tag */

} /* ReadReaction_L2 */


/* ----------------------------------------------------------------------------
   ReadReactions
   
   Read a list of reactions sections.
*/
void ReadReactions (PINPUTBUF pibIn, int iSBML_level)
{
  while (GetSBMLLex (pibIn, KM_REACTIONS, KM_REACTION)) {
    if (iSBML_level == 1)
      ReadReaction_L1 (pibIn);
    else
      ReadReaction_L2 (pibIn);
  }

} /* ReadReactions */


/* ----------------------------------------------------------------------------
   ReadRule
   
   Read a rate rule section in the given SBML level 2 buffer.
*/
void ReadRule (PINPUTBUF pibIn)
{
  PSTRLEX szRName;
  PSTREQN szEqn;
  int bInited = FALSE;
  PINPUTINFO pinfo = (PINPUTINFO) pibIn->pInfo;

  /* set context to Dynamics section */
  pinfo->wContext = CN_DYNAMICS;

  while (*pibIn->pbufCur++ != '"'); /* this assumes that name comes fist */

  GetIdentifier (pibIn, szRName);

  while (*pibIn->pbufCur++ != '>'); /* go to end of tag */

  /* get an "apply" tag */
  GetSBMLLex (pibIn, KM_SBML, KM_APPLY);

  ReadApply (pibIn, &bInited, szEqn);

  printf ("rate for %s = %s\n", szRName, szEqn);

  /* define reaction name as Derivative spec in the Dynamics section */ 
  DefineVariable (pibIn, szRName, szEqn, KM_DXDT);

  while (*pibIn->pbufCur++ != '>'); /* go to end of tag */

} /* ReadRule */


/* ----------------------------------------------------------------------------
   ReadRules
   
   Read a list of rate rules.
*/
void ReadRules (PINPUTBUF pibIn, int iSBML_level)
{

  if (iSBML_level == 1)
    printf ("mod: ignoring rate rules definitions in level 1...\n");
  else
    while (GetSBMLLex (pibIn, KM_RULESLIST, KM_RATERULE))
      ReadRule (pibIn);

} /* ReadRules */


/* ----------------------------------------------------------------------------
   ReadSBMLLevel
   
   Read a sbml tag content and get the level.
*/
int ReadSBMLLevel (PINPUTBUF pibIn)
{
  PSTREQN szEqn;
  int iLexType;
  
  /* assumes that level comes as second spec */
  while (*pibIn->pbufCur++ != '"'); 
  while (*pibIn->pbufCur++ != '"');
  while (*pibIn->pbufCur++ != '"');

  GetNumber (pibIn, szEqn, &iLexType);

  if (!iLexType) { /* no value, assign 0 by default */
    printf ("***Error: cannot read the sbml level - exiting...\n\n");
    exit (0);
  }

  while (*pibIn->pbufCur++ != '>'); /* go to end of tag */

  switch (szEqn[0]) {

  case '1':
    printf ("sbml level 1\n");
    return (1);

  case '2':
    printf ("sbml level 2\n");
    return (2);

  default:
    printf ("***Error: unknown sbml level %s - exiting...\n\n", szEqn);
    exit (0);
  }

} /* ReadSBMLLevel */


/* ----------------------------------------------------------------------------
   Read1Species
   
   Read a species tag content. Set it up as state variables.
   If a PK template is used, process also all the automatic PK variables to
   be created from that species. 
   If bProcessPK_ODEs is TRUE, the derivatives specified by the PK template
   are processed and copied to the main info.
*/
void Read1Species (PINPUTBUF pibIn, BOOL bProcessPK_ODEs)
{
  PSTRLEX     szName;
  PSTRLEX     szBoundary;
  PSTRLEX     szCpt;
  PSTREQN     szEqn;
  int         iLexType;
  BOOL        bBoundary;
  FORSV       sVar;
  HANDLE      hType;
  PINPUTINFO  pinfo = (PINPUTINFO) pibIn->pInfo;
  PINPUTINFO  ptempinfo = (PINPUTINFO) pibIn->pTempInfo;
  PVMMAPSTRCT pvm;

  pinfo->wContext = CN_GLOBAL;

  /* get the species' name */
  while (*pibIn->pbufCur++ != '"'); /* this assumes that name comes fist */
  GetIdentifier (pibIn, szName);
  pibIn->pbufCur = pibIn->pbufCur + 1; /* pass closing '"' of name */

  while (*pibIn->pbufCur++ != '"'); /* get to boundary condition flag */
  GetIdentifier (pibIn, szBoundary);
  pibIn->pbufCur = pibIn->pbufCur + 1; /* pass closing '"' of boundary cond */
  bBoundary = !strcmp (szBoundary, "true");

  /* get the species' value */
  while (*pibIn->pbufCur++ != '"'); /* this assumes that value comes next */
  GetNumber (pibIn, szEqn, &iLexType);

  if (pinfo->bTemplateInUse) {
    /* reset the species' value, to avoid confusion in case of redefinition */
    sprintf (szEqn, "0");
    /* get the species compartment */
    pibIn->pbufCur = pibIn->pbufCur + 1; /* pass closing '"' of value */
    while (*pibIn->pbufCur++ != '"'); /* get to compartment flag */
    GetIdentifier (pibIn, szCpt);

    if (strcmp (szCpt, "compartment")) { /* species is in a meaningful cpt */ 
      if (!(GetVarPTR (ptempinfo->pvmCpts, szCpt))) {
        /* compartment not defined by the template: error */
        printf ("***Error: template did not defined");
        printf (" compartment '%s' - exiting...\n\n", szCpt);
        exit (0);
      }
      else /* extend the variable name with the compartment name */
        sprintf (szName, "%s_%s", szName, szCpt);

      if (bBoundary) { 
        /* species assigned boundary conditions are defined as parameters */
        if (!(hType = GetVarType (pinfo->pvmGloVars, szName))) { /* New id */
          /* link value to symbol */
          DefineGlobalVar (pibIn, pvm, szName, szEqn, hType);
          printf ("param.   %s = %s  (was boundary species)\n", szName, szEqn);
        } /* end if */
      } /* end if bBoundary */
      else /* not boundary, create a state variable */
        SetVar (pibIn, szName, szEqn, ID_STATE);
    }
    else { /* species is outside of a meaningful compartment */

      /* first: species set to boundary conditions (i.e. invariant) are
         not allowed to circulate. If found outside of a meaningful
         compartment: exit with error message */
      if (bBoundary) {
        printf ("***Error: Species %s is set to boundary;\n", szName);
        printf ("          It has to be inside a meaningful compartment -");
        printf ("exiting.\n\n");
        exit (0);
      } 

      /* species in external compartment, store its info to pass through
         ForAllVar list scanning routine */
      sVar.pibIn  = pibIn;
      sVar.szName = szName;
      sVar.szVal  = szEqn;

      /* create state variables, input, outputs and parameters, adding the 
         species name at the beginning of each state variable of the 
         template that starts with '_' */
      ForAllVar (NULL, ptempinfo->pvmGloVars, 
                 &Create1Var, ID_NULL, (PVOID) &sVar);

      /* same for the PK variables local to Dynamics */
      pinfo->wContext = CN_DYNAMICS;
      sVar.pTarget = pinfo->pvmDynEqns;
      ForAllVar (NULL, ptempinfo->pvmGloVars, 
                 &Create1Var, ID_LOCALDYN, (PVOID) &sVar);
      /* transcribe PK dynamic equations, except derivatives */
      ForAllVar (NULL, ptempinfo->pvmDynEqns,
                 &Transcribe1AlgEqn, ID_NULL, (PVOID) &sVar);
      /* transcribe derivatives only if requested */
      if (bProcessPK_ODEs)
        ForAllVar (NULL, ptempinfo->pvmDynEqns,
                   &Transcribe1DiffEqn, ID_NULL, (PVOID) &sVar);

      /* same for Scale */
      pinfo->wContext = CN_SCALE;
      sVar.pTarget = pinfo->pvmScaleEqns;
      ForAllVar (NULL, ptempinfo->pvmGloVars, 
                 &Create1Var, ID_LOCALSCALE, (PVOID) &sVar);
      ForAllVar (NULL, ptempinfo->pvmScaleEqns, 
                 &Transcribe1AlgEqn, ID_NULL, (PVOID) &sVar);

      /* same CalcOutputs */
      pinfo->wContext = CN_CALCOUTPUTS;
      sVar.pTarget = pinfo->pvmCalcOutEqns;
      ForAllVar (NULL, ptempinfo->pvmGloVars, 
                 &Create1Var, ID_LOCALCALCOUT, (PVOID) &sVar);
      ForAllVar (NULL, ptempinfo->pvmCalcOutEqns, 
                 &Transcribe1AlgEqn, ID_NULL, (PVOID) &sVar);
    }
  } /* end if (pinfo->bTemplateInUse) */

  else { /* no PK template, process the variable, ignoring compartments */
    if (!iLexType)
      sprintf (szEqn, "0.0"); /* no value, assign 0 by default */
    if (bBoundary) { 
      /* species assigned boundary conditions are defined as parameters */
      if (!(hType = GetVarType (pinfo->pvmGloVars, szName))) { /* New id */
        /* link value to symbol */
        DefineGlobalVar (pibIn, pvm, szName, szEqn, hType);
        printf ("param.   %s = %s  (was boundary species)\n", szName, szEqn);
      } /* end if */
    } /* end if bBoundary */
    else /* not boundary, create a state variable */
      SetVar (pibIn, szName, szEqn, ID_STATE);
  }

  while (*pibIn->pbufCur++ != '>'); /* go to end of tag */

} /* Read1Species */


/* ----------------------------------------------------------------------------
   ReadSpecies
   
   Read a list of species section.
*/
void ReadSpecies (PINPUTBUF pibIn, int iSBML_level, BOOL bProcessPK_ODEs)
{
  while (GetSBMLLex (pibIn, KM_SPECIESLIST, KM_SPECIES))
    Read1Species (pibIn, bProcessPK_ODEs);

} /* ReadSpecies */


/* ----------------------------------------------------------------------------
   ReadFileNames

   Reads strings found at the beginning of each line (except the first one) of
   a file. 
*/
void ReadFileNames (PINPUTBUF pibIn, PLONG nFiles, PSTR **pszNames)
{
  long i;
  int iLexType, iErr = 0;
  char szLex[MAX_FILENAMESIZE];
  PSTRLEX szPunct;
  PSTR pbufStore;

  /* store current buffer position */
  pbufStore = pibIn->pbufCur;

  do { /* Get number of model filenames in list */
    GetaString (pibIn, szLex);
    *nFiles = *nFiles + 1;
    NextLex (pibIn, szPunct, &iLexType);
    SkipWhitespace (pibIn);

    if (!(iLexType & LX_IDENTIFIER)) {
      /* not an identifier, should be ',' or '}' */
      if ((szPunct[0] != ',') && (szPunct[0] != CH_RBRACE))
        iErr = szPunct[1] = CH_RBRACE;
    }              
  } while ((szPunct[0] != CH_RBRACE) && (!iErr));

  if (!(*pszNames = (PSTR *) malloc (*nFiles * sizeof(PSTR))))
    ReportError (NULL, RE_OUTOFMEM | RE_FATAL, "ReadJModels", NULL);

  /* Get actual model filenames */
  pibIn->pbufCur = pbufStore;
  for (i = 0; i < *nFiles; i++) {
    GetaString (pibIn, szLex);
    NextLex (pibIn, szPunct, &iLexType);
    SkipWhitespace (pibIn);

    if ( !((*pszNames)[i] = (PSTR) malloc (strlen (szLex) + 1)))
      ReportError (NULL, RE_OUTOFMEM | RE_FATAL, "ReadFileNames", NULL);
    else
      strcpy ((*pszNames)[i], szLex);
  } /* for i */

} /* ReadFileNames */


/* ----------------------------------------------------------------------------
   ReadSBMLModels
   
   Read the list of SBML model definition file given in an SBMLModels list.
*/
void ReadSBMLModels (PINPUTBUF pibIn)
{
  long i, nFiles = 0;
  PSTR *pszFileNames = NULL;
  INPUTBUF ibInLocal;
  int iSBML_level;
  PINPUTINFO pinfo = (PINPUTINFO) pibIn->pInfo;

  /* read the SBML model file names from current buffer */
  ReadFileNames (pibIn, &nFiles, &pszFileNames);

  /* in each file, get the functions, the compartements, variables and 
     rate rules  or reactions (to be set up as local variables) */
  for (i = 0; i < nFiles; i++) { 

    printf ("\nreading model %s\n", pszFileNames[i]);

    /* init buffer and read in the input file. */
    /* buffer size -1 will create a buffer of the size of the input file */ 
    if (!InitBuffer (&ibInLocal, -1, pszFileNames[i]))
      ReportError (&ibInLocal, RE_INIT | RE_FATAL, "ReadJModels", NULL);

    /* attach info records to input buffer */
    ibInLocal.pInfo = pibIn->pInfo;
    ibInLocal.pTempInfo = pibIn->pTempInfo;

    /* read the SBML level */
    if (GetSBMLLex (&ibInLocal, KM_SBML, KM_SBML))
      iSBML_level = ReadSBMLLevel (&ibInLocal);

    /* PK template requires level 2 SBML, issue an error otherwise */
    if ((pinfo->bTemplateInUse) && (iSBML_level < 2)) {
      printf ("***Error: use of a PK template requires ");
      printf ("SBML level 2 - exiting.\n\n");
      exit (0);
    }

    /* read compartments, do not presume order, reset buffer */
    if (pinfo->bTemplateInUse) {
      ibInLocal.pbufCur = ibInLocal.pbufOrg;
      if (GetSBMLLex (&ibInLocal, KM_SBML, KM_CPTLIST))
        ReadCpts (&ibInLocal, TRUE); /* TRUE -> print the cpt name etc. */
    }
    else { /* ignore the compartments of SBML models if no template */
      printf ("no PK template given: ignoring SBML compartments\n");
    }

    /* read function definitions, reset buffer */
    ibInLocal.pbufCur = ibInLocal.pbufOrg;
    if (GetSBMLLex (&ibInLocal, KM_SBML, KM_FUNCLIST))
      ReadFunctions (&ibInLocal, iSBML_level);

    /* read SBML parameters, reset buffer */
    ibInLocal.pbufCur = ibInLocal.pbufOrg;
    while (GetSBMLLex (&ibInLocal, KM_SBML, KM_PARAMS))
      ReadParameters (&ibInLocal);

    /* read SBML species, reset buffer */
    ibInLocal.pbufCur = ibInLocal.pbufOrg;
    if (GetSBMLLex (&ibInLocal, KM_SBML, KM_SPECIESLIST))
      ReadSpecies (&ibInLocal, iSBML_level, FALSE); /* don't bProcessPK_ODEs */

    /* read SBML rate rules, reset buffer */
    ibInLocal.pbufCur = ibInLocal.pbufOrg;
    if (GetSBMLLex (&ibInLocal, KM_SBML, KM_RULESLIST))
      ReadRules (&ibInLocal, iSBML_level);

    /* read SBML reactions, reset buffer */
    ibInLocal.pbufCur = ibInLocal.pbufOrg;
    if (GetSBMLLex (&ibInLocal, KM_SBML, KM_REACTIONS))
      ReadReactions (&ibInLocal, iSBML_level);

  } /* for model index i*/

  /* now scan again to read the differential equations */
  for (i = 0; i < nFiles; i++) { 

    /* init buffer and read in the input file. */
    if (!InitBuffer (&ibInLocal, -1, pszFileNames[i]))
      ReportError (&ibInLocal, RE_INIT | RE_FATAL, "ReadJModels", NULL);

    ibInLocal.pInfo = pibIn->pInfo;
    ibInLocal.pTempInfo = pibIn->pTempInfo;

    /* re-read compartments, no printing */
    /* ignore the compartments of SBML models if no template */
    if (pinfo->bTemplateInUse) {
      if (GetSBMLLex (&ibInLocal, KM_SBML, KM_CPTLIST))
        ReadCpts (&ibInLocal, FALSE);
    }

    printf ("\nmod: reading differentials in model %s\n", pszFileNames[i]); 

    /* re-read SBML species, reset buffer */
    ibInLocal.pbufCur = ibInLocal.pbufOrg;
    if (GetSBMLLex (&ibInLocal, KM_SBML, KM_SPECIESLIST))
      ReadSpecies (&ibInLocal, iSBML_level, TRUE); /* TRUE: bProcessPK_ODEs */

    ReadDifferentials (&ibInLocal);

  } /* for model index i*/

  printf ("\n");

  /* cleanup */
  for (i = 0; i < nFiles; i++)
    free (pszFileNames[i]);

  free (pszFileNames);

  pinfo->wContext = CN_END;

} /* ReadSBMLModels */


/* ----------------------------------------------------------------------------
   ReadPKTemplate
   
   Read the template pharmacokinetic model definition in the file given 
   by the next lexical element of the buffer argument. Information is 
   stored in the pTmpInfo structure of the pibIn buffer argument.

*/
void ReadPKTemplate (PINPUTBUF pibIn)
{
  INPUTBUF ibInLocal;
  PSTRLEX szLex; /* Lex elem of MAX_LEX length */
  PSTREQN szEqn; /* Equation buffer of MAX_EQN length */
  int iLexType;
  long nFiles = 0;
  PSTR *pszFileNames;
  PINPUTINFO pinfo;

  /* exchange info and tempInfo pointers to store data in fact in tempInfo */
  pinfo = (PINPUTINFO) pibIn->pTempInfo;

  /* reset context */
  pinfo->wContext = CN_GLOBAL;

  /* read the template model file name from current buffer */
  ReadFileNames (pibIn, &nFiles, &pszFileNames);

  if (nFiles > 1)
    printf ("mod: cannot use more that one template - using only the 1st\n\n");

  /* give the template name used */
  printf ("%s\n", pszFileNames[0]);

  if (!InitBuffer (&ibInLocal, BUFFER_SIZE, pszFileNames[0]))
    ReportError (&ibInLocal, RE_INIT | RE_FATAL, "ReadModel", NULL);

  ibInLocal.pInfo = (PVOID) pinfo; /* Attach info to local input buffer */

  do { /* State machine for parsing syntax */
    NextLex (&ibInLocal, szLex, &iLexType);
    switch (iLexType) {
      case LX_NULL:
        pinfo->wContext = CN_END;
        break;

      case LX_IDENTIFIER:
        ProcessWord (&ibInLocal, szLex, szEqn);
        break;
 
      case LX_PUNCT: case LX_EQNPUNCT:
        if (szLex[0] == CH_STMTTERM) {
          break;
        }
        else {
          if (szLex[0] == CH_RBRACE && 
             (pinfo->wContext & (CN_DYNAMICS | CN_JACOB | CN_SCALE))) {
            pinfo->wContext = CN_GLOBAL;
            break;
          }
          else {
            if (szLex[0] == CH_COMMENT) {
              SkipComment (&ibInLocal);
              break;
            }
            /* else: fall through! */
          }
        }

      default:
        ReportError (&ibInLocal, RE_UNEXPECTED, szLex, "* Ignoring");
        break;
 
      case LX_INTEGER:
      case LX_FLOAT:
        ReportError (&ibInLocal, RE_UNEXPNUMBER, szLex, "* Ignoring");
        break; 
 
    } /* switch */

  } while (pinfo->wContext != CN_END && 
           (*ibInLocal.pbufCur || 
            FillBuffer (&ibInLocal, BUFFER_SIZE) != EOF));

  fclose (ibInLocal.pfileIn);

  ReversePointers (&pinfo->pvmGloVars);
  ReversePointers (&pinfo->pvmDynEqns);
  ReversePointers (&pinfo->pvmScaleEqns);
  ReversePointers (&pinfo->pvmCalcOutEqns);
  ReversePointers (&pinfo->pvmJacobEqns);

  /* reset pinfo pointers */
  pinfo = (PINPUTINFO) pibIn->pInfo;

  /* set context */
  pinfo->wContext = CN_TEMPLATE_DEFINED;
  pinfo->bTemplateInUse = TRUE;
 
} /* ReadPKTemplate */

#endif /* HAVE_LIBSBML */
