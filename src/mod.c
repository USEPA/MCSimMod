/* mod.c

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

   Entry point for model code generator.

   Calls ReadModel() to define the model and WriteModel() to create
   the output file.

   The INPUTINFO struct is defined as follows:

     wContext	:	A flag for the current context of the input.

     pvmGloVars :       Vars of type ID_LOCAL* are accessible
                        only within the respective section.  If
                        states are given a value outside of the
                        Dynamics section, it is used as an INITIAL
                        value, otherwise they are initialized to 0.0.

     pvmDynEqns:	List of equations to go into the CalcDeriv(),
     pvmJacobEqns:	Jacobian(), and ScaleModel() routines, respectively.
     pvmScaleEqns:	The LHS of equations of state variables in Dynamics
                        are actually dt().  The hType field gives the type of
                        the LHS and also a flag for space in the uppermost
                        bit.
     pvmCalcOutEqns:    List of equations to into CalcOutputs().  These
                        can be used to scale output variables *only*.
                        The routinine is called just be outputting
                        values specified in Print().
*/
#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getopt.h"
#include "lexerr.h"
#include "mod.h"
#include "modi.h"
#include "modiSBML.h"
#include "modo.h"
#include "strutil.h"

/* Globals */
static char vszOptions[] = "hHDRG";
static char vszFilenameDefault[] = "model.c";
char szFileWithExt[MAX_FILENAMESIZE];

/* ----------------------------------------------------------------------------
   AnnounceProgram
*/
void AnnounceProgram(void) {
  Rprintf("\n________________________________________\n");
  Rprintf("\nMod " VSZ_VERSION " - Model Generator for MCSim\n\n");

  Rprintf("MCSim and associated software comes with ABSOLUTELY NO WARRANTY;\n"
          "This is free software, and you are welcome to redistribute it\n"
          "under certain conditions; see the GNU General Public License.\n\n");

#ifdef HAVE_LIBSBML
  Rprintf("Using LibSBML.\n\n");
#endif

} /* AnnounceProgram */

/* ----------------------------------------------------------------------------
   PromptFilenames

   prompts for both input and output file names. The space allocated
   for inputting the files is reallocated to their actual size.
*/
__attribute__((warn_unused_result)) int PromptFilenames(PSTR *pszFileIn, PSTR *pszFileOut) {
  if (!(*pszFileIn = (PSTR)calloc(1, MAX_FILENAMESIZE))) {
    PROPAGATE_EXIT(ReportError(NULL, RE_OUTOFMEM | RE_FATAL, "PromptFilenames", NULL));
  }

  if (!(*pszFileOut = (PSTR)calloc(1, MAX_FILENAMESIZE))) {
    PROPAGATE_EXIT(ReportError(NULL, RE_OUTOFMEM | RE_FATAL, "PromptFilenames", NULL));
  }

  Rprintf("Input filename? ");

  if (fgets(*pszFileIn, MAX_FILENAMESIZE, stdin)) {
    *pszFileIn = strtok(*pszFileIn, " \t\n");
  } else { /* Nothing entered, quit */
    Rprintf("input file not specified\n");
    return EXIT_NOERROR;
  }

  if ((*pszFileIn)[0]) { /* Input file specified */
    Rprintf("Output filename? ");

    if (fgets(*pszFileOut, MAX_FILENAMESIZE, stdin)) {
      *pszFileOut = strtok(*pszFileOut, " \t\n");
    }
  }

  if (!(*pszFileOut) || !(*pszFileOut)[0]) { /* If no output specified */
    free(*pszFileOut);                       /* .. use default later */
    *pszFileOut = NULL;
  } else {
    if (!(*pszFileIn = (PSTR)realloc(*pszFileIn, MyStrlen(*pszFileIn) + 1))) {
      PROPAGATE_EXIT(ReportError(NULL, RE_OUTOFMEM | RE_FATAL, "PromptFilenames", NULL));
    }
    if (!(*pszFileOut = (PSTR)realloc(*pszFileOut, MyStrlen(*pszFileOut) + 1))) {
      PROPAGATE_EXIT(ReportError(NULL, RE_OUTOFMEM | RE_FATAL, "PromptFilenames", NULL));
    }
  }
  return 0;
} /* PromptFilenames */

/* ----------------------------------------------------------------------------
   ShowHelp
*/
void ShowHelp() {
  Rprintf("Help:\n");
  Rprintf("Usage: mod [options] [input-file [output-file]]\n");
  Rprintf("Options:\n");
  Rprintf("  -h  Display this information\n");
  Rprintf("  -H  Display this information\n");
  Rprintf("  -D  Debug mode\n");
  Rprintf("  -R  Generate an R deSolve compatible C file\n");

  Rprintf("Creates file 'output-file' (or 'model.c', by default)\n");
  Rprintf("according to the input-file specifications.\n\n");

} /* ShowHelp */

/* ----------------------------------------------------------------------------
   GetCmdLineArgs

   Retrieves options and filenames from the command line arguments passed to
   the program.

   The command line syntax is:

     mod [options] [input-file [output-file]]

   If the output filename is not given a default (model.c) is used.
   Options with arguments can be used to give a collection of input
   files to process and merge into a single output file.
   If neither the input, nor output filenames are given, the
   program prompts for them both.

   The options can appear anywhere in the line and in any order.

   The options are parsed with _getopt(). After _getopt() is called,
   the args in rgszArg have been permuted so that non-option args are
   first, which in this case means the input and output filenames.

   Uses the following globals:

     char *optarg;    -- Contains the string argument to each option in turn
     int   optind;    -- Index in ARGV of the next elem to be scanned
     char *nextchar;  -- The next char to be scanned in the option-element
     int   opterr;    -- 0 value flags to inhibit GNU error messages

*/
__attribute__((warn_unused_result)) int GetCmdLineArgs(int nArg, char *const *rgszArg, PSTR *pszFileIn,
                                                       PSTR *pszFileOut, PINPUTINFO pinfo) {
  int c;

  opterr = 1; /* inhibit internal -getopt error messages */

  *pszFileIn = *pszFileOut = (PSTR)NULL;

  while (1) {

    c = PROPAGATE_EXIT_OR_RETURN_RESULT(_getopt(nArg, rgszArg, vszOptions));
    if (c == EOF) { /* Finish with option args */
      break;
    }

    switch (c) {
    case 'D':
      /* Could setup to run with certain debug flags, not used  */
      Rprintf(">> Debug mode using option '%s': "
              "Not implemented, ignored.\n\n",
              optarg);
      break;

    case 'H':
    case 'h':
      ShowHelp();
      return EXIT_NOERROR;
      break;

    case 'R':
      Rprintf(">> Generating code for linking with R deSolve package.\n\n");
      pinfo->bforR = TRUE;
      break;

    case '?':
    default:
      ShowHelp();
      return EXIT_NOERROR;

    } /* switch */

  } /* while */

  switch (nArg - optind) { /* Remaining args should be filenames */

  case 2: /* Output and input file specified */
    *pszFileOut = strdup(rgszArg[optind + 1]);
    if (*pszFileOut == NULL) {
      return EXIT_ERROR;
    }

    /* Fall through! */

  case 1: /* Input file specified */
    *pszFileIn = strdup(rgszArg[optind]);
    if (*pszFileIn == NULL) {
      free(*pszFileOut);
      return EXIT_ERROR;
    }
    break;

  case 0: /* No file names specified */
    PROPAGATE_EXIT(PromptFilenames(pszFileIn, pszFileOut));
    break;

  default:
    Rprintf("mod: too many parameters on command line\n");
    ShowHelp();
    return EXIT_ERROR;
    break;

  } /* switch */

  while (*pszFileIn && (*pszFileIn)[0] &&      /* Files specified   */
         !MyStrcmp(*pszFileIn, *pszFileOut)) { /* and not different */

    Rprintf("\n** Input and output filename must be different.\n");
    PROPAGATE_EXIT(PromptFilenames(pszFileIn, pszFileOut));
  } /* while */

  if (!(*pszFileIn && (*pszFileIn)[0])) { /* no input name given is an error */
    Rprintf("Error: an input file name must be specified - Exiting\n\n");
    return EXIT_ERROR;
  }

  /* store input file name for use by modo.c */
  pinfo->szInputFilename = *pszFileIn;

#ifdef _WIN32
  /* avoid compiler errors because of invalid escapes
     when the path is presented in source */
  PSTR pStr = strchr(pinfo->szInputFilename, '\\');
  while (pStr) {
    *pStr = '/';
    pStr = strchr(pStr + 1, '\\');
  }
#endif /* _WIN32 */

  /* use default output file name if it is missing */
  if (!*pszFileOut) {
    *pszFileOut = vszFilenameDefault;
  }
  return 0;
} /* GetCmdLineArgs */

/* ----------------------------------------------------------------------------
   InitInfo
*/

void InitInfo(PINPUTINFO pinfo, PSTR szModGenName) {
  pinfo->wContext = CN_GLOBAL;
  pinfo->bDelays = FALSE;
  pinfo->bforR = FALSE;
  pinfo->bTemplateInUse = FALSE;
  pinfo->szModGenName = szModGenName;

#ifdef _WIN32
  /* avoid compiler errors because of invalid escapes
     when the path is presented in source */
  PSTR pStr = strchr(pinfo->szModGenName, '\\');
  while (pStr) {
    *pStr = '/';
    pStr = strchr(pStr + 1, '\\');
  }
#endif /* _WIN32 */

  pinfo->scale_eqns_cnt = 0;

  pinfo->pvmGloVars = NULL;
  pinfo->pvmDynEqns = NULL;
  pinfo->pvmScaleEqns = NULL;
  pinfo->pvmJacobEqns = NULL;
  pinfo->pvmCalcOutEqns = NULL;
  pinfo->pvmEventEqns = NULL;
  pinfo->pvmRootEqns = NULL;

  pinfo->pvmCpts = NULL;
  pinfo->pvmLocalCpts = NULL;

} /* InitInfo */

/* ----------------------------------------------------------------------------
   Cleanup

   deallocate memory.
*/

void Cleanup(PINPUTINFO pinfo) {
  PVMMAPSTRCT next;

  while (pinfo->pvmGloVars) {
    next = pinfo->pvmGloVars->pvmNextVar;
    if (pinfo->pvmGloVars->szName) {
      free(pinfo->pvmGloVars->szName);
    }
    if (pinfo->pvmGloVars->szEqn) {
      free(pinfo->pvmGloVars->szEqn);
    }
    if (pinfo->pvmGloVars) {
      free(pinfo->pvmGloVars);
    }
    pinfo->pvmGloVars = next;
  }

  while (pinfo->pvmDynEqns) {
    next = pinfo->pvmDynEqns->pvmNextVar;
    if (pinfo->pvmDynEqns->szName) {
      free(pinfo->pvmDynEqns->szName);
    }
    if (pinfo->pvmDynEqns->szEqn) {
      free(pinfo->pvmDynEqns->szEqn);
    }
    if (pinfo->pvmDynEqns) {
      free(pinfo->pvmDynEqns);
    }
    pinfo->pvmDynEqns = next;
  }

  while (pinfo->pvmScaleEqns) {
    next = pinfo->pvmScaleEqns->pvmNextVar;
    if (pinfo->pvmScaleEqns->szName) {
      free(pinfo->pvmScaleEqns->szName);
    }
    if (pinfo->pvmScaleEqns->szEqn) {
      free(pinfo->pvmScaleEqns->szEqn);
    }
    if (pinfo->pvmScaleEqns) {
      free(pinfo->pvmScaleEqns);
    }
    pinfo->pvmScaleEqns = next;
  }

  while (pinfo->pvmJacobEqns) {
    next = pinfo->pvmJacobEqns->pvmNextVar;
    if (pinfo->pvmJacobEqns->szName) {
      free(pinfo->pvmJacobEqns->szName);
    }
    if (pinfo->pvmJacobEqns->szEqn) {
      free(pinfo->pvmJacobEqns->szEqn);
    }
    if (pinfo->pvmJacobEqns) {
      free(pinfo->pvmJacobEqns);
    }
    pinfo->pvmJacobEqns = next;
  }

  while (pinfo->pvmCalcOutEqns) {
    next = pinfo->pvmCalcOutEqns->pvmNextVar;
    if (pinfo->pvmCalcOutEqns->szName) {
      free(pinfo->pvmCalcOutEqns->szName);
    }
    if (pinfo->pvmCalcOutEqns->szEqn) {
      free(pinfo->pvmCalcOutEqns->szEqn);
    }
    if (pinfo->pvmCalcOutEqns) {
      free(pinfo->pvmCalcOutEqns);
    }
    pinfo->pvmCalcOutEqns = next;
  }

  while (pinfo->pvmEventEqns) {
    next = pinfo->pvmEventEqns->pvmNextVar;
    if (pinfo->pvmEventEqns->szName) {
      free(pinfo->pvmEventEqns->szName);
    }
    if (pinfo->pvmEventEqns->szEqn) {
      free(pinfo->pvmEventEqns->szEqn);
    }
    if (pinfo->pvmEventEqns) {
      free(pinfo->pvmEventEqns);
    }
    pinfo->pvmEventEqns = next;
  }

  while (pinfo->pvmRootEqns) {
    next = pinfo->pvmRootEqns->pvmNextVar;
    if (pinfo->pvmRootEqns->szName) {
      free(pinfo->pvmRootEqns->szName);
    }
    if (pinfo->pvmRootEqns->szEqn) {
      free(pinfo->pvmRootEqns->szEqn);
    }
    if (pinfo->pvmRootEqns) {
      free(pinfo->pvmRootEqns);
    }
    pinfo->pvmRootEqns = next;
  }

  while (pinfo->pvmCpts) {
    next = pinfo->pvmCpts->pvmNextVar;
    if (pinfo->pvmCpts->szName) {
      free(pinfo->pvmCpts->szName);
    }
    if (pinfo->pvmCpts->szEqn) {
      free(pinfo->pvmCpts->szEqn);
    }
    if (pinfo->pvmCpts) {
      free(pinfo->pvmCpts);
    }
    pinfo->pvmCpts = next;
  }

  while (pinfo->pvmLocalCpts) {
    next = pinfo->pvmJacobEqns->pvmNextVar;
    if (pinfo->pvmLocalCpts->szName) {
      free(pinfo->pvmLocalCpts->szName);
    }
    if (pinfo->pvmLocalCpts->szEqn) {
      free(pinfo->pvmLocalCpts->szEqn);
    }
    if (pinfo->pvmLocalCpts) {
      free(pinfo->pvmLocalCpts);
    }
    pinfo->pvmLocalCpts = next;
  }
} /* Cleanup */

/* ----------------------------------------------------------------------------
   main -- Entry point for the simulation model preprocessor
  return -1 on error, 0 on success
*/
int c_mod(char **modelNamePtr, char **outputNamePtr) {
  // since we are now loading this a a library instead of calling an executable,
  // the following need to be reset for each call because they are global
  // variables (and thus stay modified in memory after returning from this call)
  optarg = 0;
  optind = 0;

  // Rprintf("c_mod %s %s\n", *modelNamePtr, *outputNamePtr);

  int nArg = 4;
  PSTR rgszArg[] = {"MCSIMMOD","-R",*modelNamePtr, *outputNamePtr};


  INPUTINFO info;
  INPUTINFO tempinfo;
  PSTR szFileIn, szFileOut;

  AnnounceProgram();

  InitInfo(&info, rgszArg[0]);
  InitInfo(&tempinfo, rgszArg[0]);

  int ret = GetCmdLineArgs(nArg, rgszArg, &szFileIn, &szFileOut, &info);
  // Rprintf("c_mod %s %s\n", szFileIn, szFileOut);
  if (ret == EXIT_ERROR || ret == EXIT_NOERROR) {
    free(szFileIn);
    free(szFileOut);
    Cleanup(&info);
    return -1;
  }

  ret = ReadModel(&info, &tempinfo, szFileIn);
  if (ret == EXIT_ERROR || ret == EXIT_NOERROR) {
    Rprintf("Error reading model %s\n", szFileIn);
    free(szFileIn);
    free(szFileOut);
    Cleanup(&info);
    return -1;
  }

  /* I think that here we should manipulate info if a pure template has
     been read, assuming we care about that case, otherwise it should be
     an error to define a pure template without SBML to follow */

  if (info.bforR == TRUE) {
    ret = Write_R_Model(&info, szFileOut);
  } else {
    ret = WriteModel(&info, szFileOut);
  }
  if (ret == EXIT_ERROR || ret == EXIT_NOERROR) {
    free(szFileIn);
    free(szFileOut);
    Cleanup(&info);
    return -1;
  }
  free(szFileIn);
  free(szFileOut);
  Cleanup(&info);

  return 0;
}
