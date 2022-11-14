/* modiSBML.h

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

   Header file for main parsing routines.
*/

#ifndef MODISBML_H_DEFINED

/* ----------------------------------------------------------------------------
   Inclusions
*/

#include "config.h"


/* ----------------------------------------------------------------------------
   Public Definitions
*/

#define MAX_ARGS 25


/* ----------------------------------------------------------------------------
   Public Constants
*/

/* SBML Context Types, to define SBML sections */

#define CN_SBML          1
#define CN_APPLY         1

/* ---------------------------------------------------------------------------
   Public Typedefs */


/* ---------------------------------------------------------------------------
   Prototypes */

int  GetSBMLKeywordCode (PSTR szKeyword);
void ReadApply (PINPUTBUF pibIn, PINT bInited, PSTR toto);
void ReadSBMLModels (PINPUTBUF pibIn);
void ReadPKTemplate (PINPUTBUF pibIn);

#define MODISBML_H_DEFINED
#endif

/* End */
