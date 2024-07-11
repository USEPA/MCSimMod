/* modo.h

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

   Header file for outputing routines.
*/

#ifndef MODO_H_DEFINED

/* ---------------------------------------------------------------------------
   Constants  */

#define ALL_VARS (0)

/* ---------------------------------------------------------------------------
   Typedefs */

typedef int (*PFI_CALLBACK)(PFILE, PVMMAPSTRCT, PVOID);

/* ---------------------------------------------------------------------------
   Macros */

#define WriteIndexName(pfile, pvm) (fprintf((pfile), "ID_%s", (pvm)->szName))

/* ---------------------------------------------------------------------------
   Prototypes */

__attribute__((warn_unused_result)) int AdjustOneVar(PFILE pfile, PVMMAPSTRCT pvm, PVOID pInfo);
__attribute__((warn_unused_result)) int AdjustVarHandles(PVMMAPSTRCT pvmGlo);
__attribute__((warn_unused_result)) int AssertExistsEqn(PFILE pfile, PVMMAPSTRCT pvm, PVOID pInfo);
__attribute__((warn_unused_result)) int CountOneDecl(PFILE pfile, PVMMAPSTRCT pvm, PVOID pInfo);
__attribute__((warn_unused_result)) int ForAllVar(PFILE pfile, PVMMAPSTRCT pvm, PFI_CALLBACK pfiFunc, HANDLE hType,
                                                  PVOID pinfo);
__attribute__((warn_unused_result)) int ForAllVarwSep(PFILE pfile, PVMMAPSTRCT pvm, PFI_CALLBACK pfiFunc, HANDLE htype,
                                                      PVOID pinfo);
PSTR GetName(PVMMAPSTRCT pvm, PSTR szModelVarName, PSTR szDerivName, HANDLE hType);
__attribute__((warn_unused_result)) int IndexOneVar(PFILE pfile, PVMMAPSTRCT pvm, PVOID pInfo);
__attribute__((warn_unused_result)) int IndexVariables(PVMMAPSTRCT pvmGlo);
void ReversePointers(PVMMAPSTRCT *ppvm);
__attribute__((warn_unused_result)) int TranslateEquation(PFILE pfile, PSTR szEqn, long iEqType);
__attribute__((warn_unused_result)) int TranslateID(PINPUTBUF pibDum, PFILE pfile, PSTR szLex, int iEqType);
__attribute__((warn_unused_result)) int VerifyEqns(PVMMAPSTRCT pvmGlo, PVMMAPSTRCT pvmDyn);
__attribute__((warn_unused_result)) int WriteCalcDeriv(PFILE pfile, PVMMAPSTRCT pvmGlo, PVMMAPSTRCT pvmDyn);
__attribute__((warn_unused_result)) int WriteCalcJacob(PFILE pfile, PVMMAPSTRCT pvmGlo, PVMMAPSTRCT pvmJacob);
__attribute__((warn_unused_result)) int WriteCalcOutputs(PFILE pfile, PVMMAPSTRCT pvmGlo, PVMMAPSTRCT pvmCalcOut);
__attribute__((warn_unused_result)) int WriteDecls(PFILE pfile, PVMMAPSTRCT pvmGlo);
__attribute__((warn_unused_result)) int WriteHeader(PFILE pfile, PSTR szName, PVMMAPSTRCT pvmGlo);
void WriteIncludes(PFILE pfile);
__attribute__((warn_unused_result)) int WriteInitModel(PFILE pfile, PVMMAPSTRCT pvmGlo);
__attribute__((warn_unused_result)) int WriteModel(PINPUTINFO pinfo, PSTR szFileOut);
__attribute__((warn_unused_result)) int WriteOneDecl(PFILE pfile, PVMMAPSTRCT pvm, PVOID pInfo);
__attribute__((warn_unused_result)) int WriteOneEquation(PFILE pfile, PVMMAPSTRCT pvm, PVOID pInfo);
__attribute__((warn_unused_result)) int WriteOneIndexDefine(PFILE pfile, PVMMAPSTRCT pvm, PVOID pInfo);
__attribute__((warn_unused_result)) int WriteOneInit(PFILE pfile, PVMMAPSTRCT pvm, PVOID pInfo);
__attribute__((warn_unused_result)) int WriteOneName(PFILE pfile, PVMMAPSTRCT pvm, PVOID pInfo);
__attribute__((warn_unused_result)) int WriteOneOutputName(PFILE pfile, PVMMAPSTRCT pvm, PVOID pInfo);
__attribute__((warn_unused_result)) int WriteOneVMEntry(PFILE pfile, PVMMAPSTRCT pvm, PVOID pInfo);
__attribute__((warn_unused_result)) int WriteOne_R_PIDefine(PFILE pfile, PVMMAPSTRCT pvm, PVOID pInfo);
__attribute__((warn_unused_result)) int WriteOne_R_SODefine(PFILE pfile, PVMMAPSTRCT pvm, PVOID pInfo);
__attribute__((warn_unused_result)) int WriteOne_R_ParmInit(PFILE pfile, PVMMAPSTRCT pvm, PVOID pInfo);
__attribute__((warn_unused_result)) int WriteOne_R_PSDecl(PFILE pfile, PVMMAPSTRCT pvm, PVOID pInfo);
__attribute__((warn_unused_result)) int WriteScale(PFILE pfile, PVMMAPSTRCT pvmGlo, PVMMAPSTRCT pvmScale);
__attribute__((warn_unused_result)) int WriteVarMap(PFILE pfile, PVMMAPSTRCT pvmGlo);
__attribute__((warn_unused_result)) int Write_R_CalcDeriv(PFILE pfile, PVMMAPSTRCT pvmGlo, PVMMAPSTRCT pvmDyn,
                                                          PVMMAPSTRCT pvmCalcOut);
__attribute__((warn_unused_result)) int Write_R_CalcJacob(PFILE pfile, PVMMAPSTRCT pvmGlo, PVMMAPSTRCT pvmJacob);
__attribute__((warn_unused_result)) int Write_R_Decls(PFILE pfile, PVMMAPSTRCT pvmGlo);
__attribute__((warn_unused_result)) int Write_R_Events(PFILE pfile, PVMMAPSTRCT pvmGlo, PVMMAPSTRCT pvmEvents);
void Write_R_Includes(PFILE pfile);
void Write_R_InitModel(PFILE pfile, PVMMAPSTRCT pvmGlo);
__attribute__((warn_unused_result)) int Write_R_InitPOS(PFILE pfile, PVMMAPSTRCT pvmGlo, PVMMAPSTRCT pvmScale);
__attribute__((warn_unused_result)) int Write_R_Model(PINPUTINFO pinfo, PSTR szFileOut);
__attribute__((warn_unused_result)) int Write_R_Roots(PFILE pfile, PVMMAPSTRCT pvmGlo, PVMMAPSTRCT pvmRoots);
__attribute__((warn_unused_result)) int Write_R_Scale(PFILE pfile, PVMMAPSTRCT pvmGlo, PVMMAPSTRCT pvmScale);
__attribute__((warn_unused_result)) int Write_R_State_Scale(PFILE pfile, PVMMAPSTRCT pvmScale);

#define MODO_H_DEFINED
#endif

/* End */
