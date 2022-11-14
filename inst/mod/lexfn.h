/* lexfn.h

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

   Header file for input definitions.
*/

#ifndef LEXFN_DEFINED

/* ----- Inclusions  */

#include "lex.h"

/* ----- Constants  */

#define N_TAU_EXPOSE	40	/* Number of Tau's to expose exp() inputs */

/* Input Function constants */

#define IFN_NULL	0
#define IFN_CONSTANT	1
#define IFN_PERDOSE	2
#define IFN_PERRATE	3
#define IFN_PEREXP	4
#define IFN_NDOSES	5

/* ----- Enumerations  */

/* ----- Typedefs  */

/* The following structure is used for predefined period input
   functions.
*/

typedef struct tagIFN {		/* Input Function struct */
  int iType;	/* IFN_ */

  BOOL bOn; 	/* Flag to indicate exposure is On */

  double dMag;	/* Magnitude of input */
  double dTper;	/* Duration of Period */
  double dT0;	/* Starting time of exposure */
  double dTexp;	/* Exposure time, input is 0.0 after T0 + Texp */

  double dDecay;/* For exponential inputs, the exponential decay rate
		   Exposure lasts for N_TAU_EXPOSE Tau periods. (tau=1/Decay)
		   After this, input is considered to be neglible. */

  double dVal;	/* Current value as calculated by CalcInputs */

  double dTStartPeriod;	/* Start of current period - for NextTransitionTime */

  HANDLE hMag;	  /* Handle to magnitude */
  HANDLE hTper;	  /* Handle to period */
  HANDLE hT0;	  /* Handle to starting time */
  HANDLE hTexp;	  /* Handle to exposure time */
  HANDLE hDecay;  /* Handle to exponential decay rate */

  int nDoses;	/* Number of dose/transition pairs for IFN_NDOSES */
  int iDoseCur;	/* Current Dose */
  PDOUBLE rgT0s;
  PDOUBLE rgTexps;
  PDOUBLE rgMags;
  
} IFN, *PIFN;	/* struct tagIFN */


/* ----- Macros  */

/* ----- Globals/Externals  */

/* ----- Prototypes  */

int  GetFnType (PSTR szName);
void InitIFN (PIFN pifn);
BOOL DefDepParm (PSTR szLex, PDOUBLE pdValue, HANDLE *phvar);
BOOL GetInputArgs (PINPUTBUF pibIn, PIFN pifn);
BOOL GetNNumbers (PINPUTBUF pibIn, PSTR szLex, int nNumbers, PDOUBLE rgd);
BOOL GetNDoses (PINPUTBUF pibIn, PSTR szLex, PIFN pifn);
BOOL GetInputFn (PINPUTBUF pibIn, PSTR sz, PIFN pifn);

#define LEXFN_DEFINED
#endif
