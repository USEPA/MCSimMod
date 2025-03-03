# MCSimMod: An R Package for Working with MCSim Models

MCSimMod is an R package that facilitates ordinary differential equation (ODE) modeling. It allows one to perform simulations for ODE models that are encoded in the [GNU MCSim](https://www.gnu.org/software/mcsim/) model specification language using ODE solvers from the R package [deSolve](https://cran.r-project.org/web/packages/deSolve/index.html).

If you are interested in contributing or want to report a bug, please start a discussion or submit an issue [here](https://github.com/USEPA/MCSimMod.git).

To work with `MCSimMod`, the package `deSolve` must be installed.
```R
install.packages("deSolve")
```

To install `MCSimMod` directly from GitHub, `devtools` is required.
```R
install.packages("devtools")
```

## Installation
When working in a Windows operating system, make sure [RTools](https://cran.r-project.org/bin/windows/Rtools/) is installed before attempting to install `MCSimMod`.

To install `MCSimMod` directly from GitHub, use the following command.
```R
devtools::install_github("https://github.com/USEPA/MCSimMod.git")
```

Alternatively, install `MCSimMod` from a compressed "tarball" file using the following command.
```R
install.packages("path/to/MCSimMod.tar.gz", repos=NULL, type="source")
```

## Getting Started
To learn about the `MCSimMod` package and how to use it, check out the vignettes.
```R
browseVignettes(package="MCSimMod")
```

## Developer Installation

If you wish to contribute to development of `MCSimMod`, first clone the [MCSimMod repository](https://github.com/USEPA/MCSimMod.git) Then, start an R session and set the current working directory to a directory within the repository. To document, build, install, and test the package, use the following commands.
```R
devtools::document()
devtools::build()
devtools::install()
devtools::test()
covr::report(file='coverage_html/index.html')
```

You can also issue these commands from a Windows or Unix command line terminal as follows.
```bash
R -e "devtools::document()"
R -e "devtools::build()"
R -e "devtools::install()"
R -e "devtools::test()"
R -e "covr::report(file='coverage_html/index.html')"
```

## Code Formatting
To maintain a consistent format for all `MCSimMod` source code, we use the [styler](https://styler.r-lib.org/) package to format R code. Note that we use a specific version of the package.
```R
# install the pinned version
install.packages("styler", version="1.10.3")

# style all files in the package, including vignettes and tests
styler::style_pkg(".")
```

A check is added in continuous integration to ensure that the code is formatted correctly. If the code is not formatted correctly, the build will fail.