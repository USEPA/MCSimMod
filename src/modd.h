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

__attribute__((warn_unused_result)) int AddEquation(PVMMAPSTRCT *ppvm, PSTR szName, PSTR szEqn, HANDLE hType);
HANDLE CalculateVarHandle(PVMMAPSTRCT pvm, PSTR sz);
__attribute__((warn_unused_result)) int CopyString(PSTR szOrg, PSTR *szBuf);
__attribute__((warn_unused_result)) int DeclareModelVar(PINPUTBUF pibIn, PSTR szName, int iKWCode);
__attribute__((warn_unused_result)) int DefineCalcOutEqn(PINPUTBUF pibIn, PSTR szName, PSTR szEqn, HANDLE hType);
__attribute__((warn_unused_result)) int DefineDynamicsEqn(PINPUTBUF pibIn, PSTR szName, PSTR szEqn, HANDLE hType);
__attribute__((warn_unused_result)) int DefineGlobalVar(PINPUTBUF pibIn, PVMMAPSTRCT pvm, PSTR szName, PSTR szEqn,
                                                        HANDLE hType);
__attribute__((warn_unused_result)) int DefineJacobEqn(PINPUTBUF pibIn, PSTR szName, PSTR szEqn, HANDLE hType);
__attribute__((warn_unused_result)) int DefineScaleEqn(PINPUTBUF pibIn, PSTR szName, PSTR szEqn, HANDLE hType);
__attribute__((warn_unused_result)) int DefineVariable(PINPUTBUF pibIn, PSTR szName, PSTR szEqn, int iKWCode);
PVMMAPSTRCT GetVarPTR(PVMMAPSTRCT pvm, PSTR szName);
int GetVarType(PVMMAPSTRCT pvm, PSTR szName);
BOOL IsMathFunc(PSTR sz);
__attribute__((warn_unused_result)) int SetEquation(PVMMAPSTRCT pvm, PSTR szEqn);
void SetVarType(PVMMAPSTRCT pvm, PSTR szName, HANDLE hType);
__attribute__((warn_unused_result)) BOOL VerifyEqn(PINPUTBUF pibIn, PSTR szEqn);

#define MODD_H_DEFINED
#endif

/* End */
