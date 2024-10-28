# MCSimMod: Run MCSim MODels in R environment

MCSimMod is an R package that converts the [GNU MCSim](https://www.gnu.org/software/mcsim/) domain-specific language (DSL) into a dynamic library (windows) or shared object (linux) for use with the [desolve](https://cran.r-project.org/web/packages/deSolve/index.html) ordinary differential equation solver.

If you are interested in contributing or want to report a bug, please submit a issue or start a discussion. See [CONTRIBUTING](CONTRIBUTING.md) for more information.

`deSolve` needed for `MCSimMod`:

`install.packages("deSolve")`

If installing from github, `devtools` is required:

`install.packages("devtools")`

OS-specific installation instructions are available below.

## Windows Install
Make sure [RTools](https://cran.r-project.org/bin/windows/Rtools/) is installed prior to MCSimMod install.

Once RTools is installed, install MCSimMod from github

`devtools::install_github("https://github.com/USEPA/MCSimMod.git")`

Or from source:

Use the pre-built MCSimMod.tar.gz provided.

```
install.packages('path/to/MCSimMod.tar.gz', repos=NULL, type='source')
```

## Linux install
Simply run:

`devtools::install_github("https://github.com/USEPA/MCSimMod.git")`

Or from source:
Use the pre-built MCSimMod.tar.gz provided.

```
install.packages('path/to/MCSimMod.tar.gz', repos=NULL, type='source')
```

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
