# MCSimMod package
<Insert intro>

`deSolve` needed for `MCSimMod`:

`library.packages("deSolve")`

## Windows Install
Make sure RTools (https://cran.r-project.org/bin/windows/Rtools/) is installed prior to RMCSim install.

Before installing, add the appropriate RTools bins to the path. Each RTools base directory will be different depending on the version.

For example, this would be the necessary paths for RTools 4.0:
`Sys.setenv(PATH = paste("C:/Rtools40/usr/bin", "C:/Rtools40/mingw32/bin",Sys.getenv("PATH"), sep=";"))`

Ultimately, the RTools usr/bin and mingw32/bin must be added to PATH before the install.

Once RTools is installed, run

`install.packages(RMCSim)` <Future: depending on how the install happens, this line might be different>

## Linux install
Simply run:

`install.packages(RMCSim)` <Future: depending on how the install happens, this line might be different>

## Developer installation

Git clone the repository. Within an R session in the working directory:

```R
install.packages(c('devtools'))

devtools::document()
devtools::build()
devtools::install()
devtools::test()
covr::report(file='coverage_html/index.html')
```

You can also call from the command-line:

```bash
R -e "devtools::document()"
R -e "devtools::build()"
R -e "devtools::install()"
R -e "devtools::test()"
R -e "covr::report(file='coverage_html/index.html')"
```
