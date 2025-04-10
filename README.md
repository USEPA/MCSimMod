# MCSimMod: An R Package for Working with MCSim Models


[![Docs Badge](https://img.shields.io/badge/Documentation-online-brightgreen)](https://usepa.github.io/MCSimMod)
[![](https://www.r-pkg.org/badges/version/MCSimMod)](https://cran.r-project.org/web/packages/MCSimMod/)
[![CRAN RStudio mirror downloads](https://cranlogs.r-pkg.org/badges/MCSimMod)](https://cran.r-project.org/web/packages/MCSimMod/)
[![GitHub Actions Workflow Status](https://img.shields.io/github/actions/workflow/status/USEPA/MCSimMod/main.yml)](https://github.com/USEPA/MCSimMod/actions?query=branch%3Amain)
[![GitHub Release](https://img.shields.io/github/v/release/USEPA/MCSimMod)](https://github.com/USEPA/MCSimMod/releases)

MCSimMod is an R package that facilitates ordinary differential equation (ODE) modeling. It allows one to perform simulations for ODE models that are encoded in the [GNU MCSim](https://www.gnu.org/software/mcsim/) model specification language using ODE solvers from the R package [deSolve](https://cran.r-project.org/web/packages/deSolve/index.html).

Documentation is available at https://usepa.github.io/MCSimMod.

To work with `MCSimMod`, the package `deSolve` must be installed.
```R
install.packages("deSolve")
```

## Installation

As a prerequisite for installation on a Windows operating system, be sure to install [RTools](https://cran.r-project.org/bin/windows/Rtools/). RTools includes utilities that compile C source code for use in R. Installation of RTools is not required when using `MCSimMod` on a Unix operating system.

To install the latest stable version from [CRAN](https://cran.r-project.org/web/packages/MCSimMod/), use the command:

```R
install.packages("MCSimMod")
```

For the latest development version, install from GitHub using `devtools::install_github()`. If the package `devtools` has not already been installed, use `install.packages("devtools")`. Then, install `MCSimMod` using the following command:
```R
devtools::install_github("https://github.com/USEPA/MCSimMod.git", build_vignettes = TRUE)
```

Alternatively, one can install `MCSimMod` from a compressed "tarball" file. For example, if you have a compressed tarball file named `MCSimMod.tar.gz`, use the following command:

```R
install.packages("MCSimMod.tar.gz", repos = NULL, type = "source")
```

## Getting Started
To learn about the `MCSimMod` package and how to use it, check out the vignettes.
```R
browseVignettes(package = "MCSimMod")
```

If you are interested in contributing or want to report a bug, please start a discussion or submit an issue [here](https://github.com/USEPA/MCSimMod.git).

## Developer Installation

If you wish to contribute to development of `MCSimMod`, first clone the [MCSimMod repository](https://github.com/USEPA/MCSimMod.git). Then, start an R session and set the current working directory to a directory within the repository. To document, build, install, and test the package, use the following commands.
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
install.packages("styler", version = "1.10.3")

# style all files in the package, including vignettes and tests
styler::style_pkg(".")
```

A check is added in continuous integration to ensure that the code is formatted correctly. If the code is not formatted correctly, the style check will fail.


## Disclaimer

The United States Environmental Protection Agency (EPA) GitHub project code is provided on an "as is" basis and the user assumes responsibility for its use.  EPA has relinquished control of the information and no longer has responsibility to protect the integrity, confidentiality, or availability of the information.  Any reference to specific commercial products, processes, or services by service mark, trademark, manufacturer, or otherwise, does not constitute or imply their endorsement, recommendation or favoring by EPA.  The EPA seal and logo shall not be used in any manner to imply endorsement of any commercial product or activity by EPA or the United States Government.
