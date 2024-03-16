/* mod.h

   Copyright (c) 1993-2018. Free Software Foundation, Inc.

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
*/

#ifndef MOD_DEFINED

/* ----- Inclusions  */

#include "lex.h"
#include "hungtype.h"
#include "config.h"


/* ----- Constants  */

/* Version and copyright */
#define VSZ_VERSION "v6.1.0"
#define VSZ_COPYRIGHT "Copyright (c) 1993-2019 Free Software Foundation, Inc."

/* The time variable accessible to the user */
#define VSZ_TIME "t"
#define VSZ_TIME_SBML "time"

/* Maximum length of an input or output  file name */
#define MAX_FILENAMESIZE      256   /* Arbitrary */


/* Keyword Map Constants */

#define KM_NULL         0
#define KM_STATES       1
#define KM_INPUTS       2
#define KM_OUTPUTS      3
#define KM_DYNAMICS     4
#define KM_SCALE        5
#define KM_JACOB        6
#define KM_CALCOUTPUTS  7
#define KM_EVENTS       8
#define KM_ROOTS        9
#define KM_DXDT         20
#define KM_INLINE       30
#define KM_SBMLMODELS   40
#define KM_PKTEMPLATE   41
#define KM_COMPARTMENTS 42
#define KM_FUNCTION     43
#define KM_END          100

/* Context Types bit flags */
/*
   These are used to keep track of what context the parser is in,
   i.e. defining global vars or defining dynamics.
   
   They are also used as bit flags in the keyword list to indicate
   a valid context for keywords, var names.
*/

#define CN_GLOBAL            0x0001
#define CN_DYNAMICS          0x0002
#define CN_SCALE             0x0003
#define CN_JACOB             0x0004
#define CN_CALCOUTPUTS       0x0005
#define CN_EVENTS            0x0006
#define CN_ROOTS             0x0007
#define CN_INPUTDEF          0x0100
#define CN_TEMPLATE_DEFINED  0x0200
#define CN_END               0x4000

#define CN_ALL               0xFFFF  /* All Contexts */


/* Identifier Types -- Stored as upper nybble so that the variable
   can be indexed in modo.c to create a handle to the variable
*/

#define ID_TYPEMASK     0xF0000 /* Allow up to 15 variable types */
#define ID_SPACEFLAG    0x0F000 /* To flag for formatting eqns */
#define ID_INDEXMASK    0x07FFF /* Index for symbol table */
#define MAX_VARS        (ID_SPACEFLAG) /* Max number of vars of a type */

#define ID_NULL         0x00000
#define ID_STATE        0x10000 /* Model state variables -- dynamics */
#define ID_INPUT        0x20000 /* Model input -- type IFN */
#define ID_OUTPUT       0x30000 /* Model output -- for observation only */
#define ID_PARM         0x40000 /* Global parameters */
#define ID_LOCALDYN     0x50000 /* Local variables in Dynamics */
#define ID_LOCALSCALE   0x60000 /* Local variables in Scale    */
#define ID_LOCALJACOB   0x70000 /* Local variables in Jacobian */
#define ID_LOCALEVENT   0x70300 /* Local variables in Events */
#define ID_LOCALROOT    0x70600 /* Local variables in Roots */
#define ID_LOCALCALCOUT 0x80000 /* Local variables in CalcOutputs */
#define ID_DERIV        0x90000 /* Derivative eqn in CalcDeriv */
#define ID_INLINE       0xA0000 /* Inline statement */
#define ID_COMPARTMENT  0xB0000 /* Model compartment (for SBML processing) */
#define ID_FUNCTION     0xC0000 /* Function definition (for SBML processing) */

/* ---------------------------------------------------------------------------
   Public Typedefs */

/* The VMMAPSTRCT, Variable Map structure, used for maintaining a
   list of variables and of dynamics equations. */

typedef HANDLE HVAR;

typedef struct tagVM {
  PSTR szName;    /* Identifier */
  PSTR szEqn;     /* Def'ing eqn to be created and copied */
  HANDLE hType;   /* ID_type of identifier */

  struct tagVM *pvmNextVar; /* Var list is a stack */

} VMMAPSTRCT, *PVMMAPSTRCT; /* Variable Map */

typedef struct tagINPUTINFO {
  WORD wContext;
  BOOL bDelays;
  BOOL bforR;
  BOOL bTemplateInUse;
  PSTR szInputFilename;
  PSTR szModGenName;

  PVMMAPSTRCT  pvmGloVars;
  PVMMAPSTRCT  pvmDynEqns;
  PVMMAPSTRCT  pvmScaleEqns;
  PVMMAPSTRCT  pvmJacobEqns;
  PVMMAPSTRCT  pvmCalcOutEqns;
  PVMMAPSTRCT  pvmEventEqns;
  PVMMAPSTRCT  pvmRootEqns;

  PVMMAPSTRCT  pvmCpts;
  PVMMAPSTRCT  pvmLocalCpts;

} INPUTINFO, *PINPUTINFO; /* tagINPUTINFO */
 

/* ----- Macros */

#define TYPE(pvm) (pvm ? (pvm)->hType & ID_TYPEMASK : ID_NULL)
#define INDEX(pvm) (pvm ? (pvm)->hType & ID_INDEXMASK : ID_NULL)

#define KM_TO_CN(kmCode) ((kmCode) == KM_CALCOUTPUTS ? CN_CALCOUTPUTS \
     : (kmCode) == KM_JACOB    ? CN_JACOB \
     : (kmCode) == KM_SCALE    ? CN_SCALE \
     : (kmCode) == KM_DYNAMICS ? CN_DYNAMICS \
     : (kmCode) == KM_EVENTS   ? CN_EVENTS \
     : (kmCode) == KM_ROOTS    ? CN_ROOTS \
     : 0)


/* ---------------------------------------------------------------------------
   Public Prototypes */

void InitInfo (PINPUTINFO pinfo, PSTR szModGenName);


#define MOD_DEFINED
#endif


