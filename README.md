# MCSimMod: Run MCSim MODels in R environment

MCSimMod is an R package that converts the MCSim domain-specific language (DSL) into a dynamic library (windows) or shared object (linux) for use with the ordinary differential equation solver: `deSolve`

If you are interested in contributing or want to report a bug, please submit a issue or start a discussion. See [CONTRIBUTING](CONTRIBUTING.md) for more information. 

`deSolve` needed for `MCSimMod`:

`install.packages("deSolve")`

If installing from source, `devtools` is required:

`install.packages("devtools")

OS-specific installation instructions are available below.

## Windows Install
Make sure [RTools](https://cran.r-project.org/bin/windows/Rtools/) is installed prior to MCSimMod install.

Once RTools is installed, install MCSimMod from github

`devtools::install_github("https://github.com/USEPA/MCSimMod.git")`

Or from source:

```
setwd('path/to/MCSimMod')
devtools::install_local()
```

## Linux install
Simply run:

`devtools::install_github("https://github.com/USEPA/MCSimMod.git")`

Or from source:

```
setwd('path/to/MCSimMod')
devtools::install_local()
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

# Getting started (also available in "quickstart" vignette)

\frac{\textrm{d}}{\textrm{d}t}y(t) &= m,
  
y(0) &= y_0,

using a string that both provides the ODE for the state variable, $y$, and sets the (default) values of the model parameters to $y_0 = 2$ and $m = 0.5$. (More details about the structure of the model specification text are provided in a separate tutorial.)
```{r, results='hide'}
mod_string = "
States = {y};
y0 = 2;
m = 0.5;
Initialize {
    y = y0;
}
Dynamics {
    dt(y) = m;
}
End.
"
model = MCSimMod::Model(mString=mod_string)
```

Once the model object is created, we can "load" the model (so that it's ready for use in a given R session) and perform a simulation that provides results for the desired output times ($t = 0, 0.1, 0.2, \ldots, 20.0$) using the following commands.
```{r, results='hide'}
model$loadModel()
times = seq(from = 0, to = 20, by = 0.1)
out = model$runModel(times)
```

The final command shown above performs a model simulation and stores the simulation results in a "matrix" data structure called `out`. The first five rows of this data structure are shown below. Note that the independent variable, which is $t$ in the case of the linear model we've created here, is always labeled "time" in the output data structure.
```{r, echo=FALSE, results='asis'}
library(knitr)
kable(out[1:5, ])
```

We can examine the parameter values and initial conditions that were used for this simulation with the following commands.
```{r}
model$parms
model$Y0
```

Finally, we can create a visual representation of the simulation results. For example, we can plot the value of $y$ vs. time ($t$) using the following command.
```{r, fig.dim=c(6, 4), fig.align='center'}
# Plot simulation results.
plot(out[, "time"], out[, "y"], type = "l", lty = 1, lwd = 2, xlab = "Time",
     ylab = "y")
```

We can remove output files that were created when building the model (i.e., files with names ending in ".c", ".o", "_inits.R", and ".dll" or ".so") by calling the `cleanup` method. Using the argument `deleteModel=TRUE` causes the model file (with name ending in ".model") that was created from the model string to also be deleted.

```{r, results='hide'}
# Cleanup required for vignette to pass
model$cleanup(deleteModel=TRUE)
```
