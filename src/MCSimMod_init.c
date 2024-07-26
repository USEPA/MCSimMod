#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>

/* FIXME: 
   Check these declarations against the C/Fortran source code.
*/

/* .C calls */
extern void c_mod(void *, void *);

static const R_CMethodDef CEntries[] = {
    {"c_mod", (DL_FUNC) &c_mod, 2},
    {NULL, NULL, 0}
};

void R_init_MCSimMod(DllInfo *dll)
{
    R_registerRoutines(dll, CEntries, NULL, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}