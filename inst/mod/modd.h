/* modd.h

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

   Header file for modd.c.
*/

#ifndef MODD_H_DEFINED


/* ---------------------------------------------------------------------------
   Prototypes */

void AddEquation (PVMMAPSTRCT *ppvm, PSTR szName, PSTR szEqn, HANDLE hType);
HANDLE CalculateVarHandle (PVMMAPSTRCT pvm, PSTR sz);
PSTR CopyString (PSTR szOrg);
void DeclareModelVar (PINPUTBUF pibIn, PSTR szName, int iKWCode);
void DefineCalcOutEqn (PINPUTBUF pibIn, PSTR szName, PSTR szEqn, HANDLE hType);
void DefineDynamicsEqn (PINPUTBUF pibIn, PSTR szName, PSTR szEqn, 
                        HANDLE hType);
void DefineGlobalVar (PINPUTBUF pibIn, PVMMAPSTRCT pvm, PSTR szName, 
                      PSTR szEqn, HANDLE hType);
void DefineJacobEqn (PINPUTBUF pibIn, PSTR szName, PSTR szEqn, HANDLE hType);
void DefineScaleEqn (PINPUTBUF pibIn, PSTR szName, PSTR szEqn, HANDLE hType);
void DefineVariable (PINPUTBUF pibIn, PSTR szName, PSTR szEqn, int iKWCode);
PVMMAPSTRCT GetVarPTR (PVMMAPSTRCT pvm, PSTR szName);
int  GetVarType (PVMMAPSTRCT pvm, PSTR szName);
BOOL IsMathFunc (PSTR sz);
void SetEquation (PVMMAPSTRCT pvm, PSTR szEqn);
void SetVarType (PVMMAPSTRCT pvm, PSTR szName, HANDLE hType);
BOOL VerifyEqn (PINPUTBUF pibIn, PSTR szEqn);

#define MODD_H_DEFINED
#endif

/* End */


