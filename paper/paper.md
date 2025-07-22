---
title: 'MCSimMod: An R Package for Working with Ordinary Differential Equation Models Encoded in the MCSim Model Specification Language'
tags:
  - R
  - MCSim
  - ordinary differential equations
  - mathematical models
  - initial value problems
authors:
  - name: Dustin F. Kapraun
    orcid: 0000-0001-5570-6383
    corresponding: true (This is how to denote the corresponding author)
    affiliation: 1
  - name: Todd J. Zurlinden
    orcid: 0000-0003-1372-3913
    affiliation: 1
  - name: Ryan D. Friese
    orcid: 0000-0002-4121-2195
    affiliation: 2
  - name: Andrew J. Shapiro
    orcid: 0000-0002-5233-8092
    affiliation: 1
affiliations:
 - name: U.S. Environmental Protection Agency, U.S.A.
   index: 1
 - name: Pacific Northwest National Laboratory, U.S.A.
   index: 2
date: 11 June 2025
bibliography: paper.bib
---

# Summary

Many physical and biological phenomena can be described using mathematical models based on ordinary differential equations (ODEs). In such a model, an ODE describes how a "state variable" changes (quantitatively) with respect to an independent variable (e.g., time or position). In general, an ODE model can include several state variables, each with its own ODE, so the model can be expressed as a system of ODEs. Thus, if $y$ is a vector of $n$ state variables, an ODE model that describes the state of the system at $t$ (i.e., at a specific time or value of the independent variable) can be expressed as
\begin{equation} \label{eq:ode_general}
  \frac{\textrm{d}}{\textrm{d}t} y(t) = f(y(t), \theta, t),
\end{equation}
where $f$ is a vector-valued function (of dimension $n$) and $\theta$ is a vector of parameters (i.e., constants or variables other than the state variables and the independent variable).

In order to obtain a specific solution for a system of ODEs, one must know the initial state of the system,
\begin{equation} \label{eq:ic_general}
  y_0 = y(t_0),
\end{equation}
where $t_0$ is the inital value of the independent variable. \autoref{eq:ic_general} is often described as a statement of the "initial conditions" of the system, and solving a system of ODEs subject to such initial conditions is called solving an initial value problem (IVP).

For the R programming language and environment [@R], there are at least two packages available that facilitate solving of IVPs. The R package `deSolve` [@deSolve] can be used to solve IVPs for ODEs that have been encoded either in R or in a compiled language, such as C or Fortran. For models encoded in a compiled language, one can compile the model source code to generate machine code that typically executes much more quickly than R code, which must be "interpreted" anew each time it's executed on a computer. The `deSolve` package includes functions that provide interfaces to well-documented, public-domain IVP solution routines implemented in FORTRAN, including four ODE integrators from the package ODEPACK, and in C, including solvers of the Runge-Kutta family. The R package `mrgsolve` [@mrgsolve] also includes functions that can be used to solve IVPs. These `mrgsolve` functions provide interfaces to IVP solution routines implemented in C++. Despite their different implementations, the packages `deSolve` and `mrgsolve` use many of the same underlying IVP solution algorithms.

We developed the R package [`MCSimMod`](https://cran.r-project.org/package=MCSimMod) to facilitate ODE modeling within the R environment. `MCSimMod` allows one to solve IVPs for ODE models that have been described in the MCSim [@MCSim] model specification language using ODE integration functions from the R package `deSolve` [@deSolve]. This system enables users to take advantage of the flexibility and post hoc data analysis capabilities of the interpreted language R while also achieving computational speeds typically associated with compiled programming languages like C and FORTRAN. Furthermore, this system encourages modelers to use separate files for defining models and executing model simulations, which can, in turn, improve modularity and reusability of source code developed for modeling analyses.

# Statement of need

Physiologically based pharmacokinetic (PBPK) models, which are a class of ODE models that describe absorption, distribution, metabolism, and excretion of a substance by a biological organism, are frequently used to inform human health risk assessments for environmental chemicals and the development of pharmaceuticals. For many years, the programming language ACSL and the associated programming environment acslX were the tools of choice for many scientists and researchers that work with PBPK models, but in 2015, the company that maintained acslX announced that it would no longer support or sell the acslX software. Prior to the decline of acslX, some PBPK modelers used the free and open-source software tools R and MCSim MCSim (usually separately) to perform computational modeling work, but once acslX became unavailable, many PBPK modelers began using R and MCSim together to implement PBPK models. (See, for example, PBPK models published by @Pearce2017_httk; @Bernstein2021_template; and @Campbell2023_chloroprene.) R and MCSim each have benefits and drawbacks when it comes to working with ODE models. R is a flexible and popular programming language and environment for analyzing data and generating graphics. However, because R is in an interpreted language, R statements must be translated into machine instructions each time they are executed on a computer. Consequently, complex calculations (such as those associated with PBPK model simulations) encoded in R are generally performed relatively slowly. MCSim is a more specialized software tool designed for implementing and calibrating mathematical models -- it is not a general purpose programming tool like R. However, MCSim takes advantage of compiled languages (as acslX did) to perform model simulations quickly, making it an appealing choice when one needs to perform many simulations, as is typically the case for Monte Carlo (MC) analyses and Markov chain Monte Carlo (MCMC) parameter estimation. One can leverage the strengths of both R and MCSim by defining an ODE model in the relatively simple MCSim model specification language, translating the model to C using an MCSim utility (called "mod"), compiling the C model to obtain machine code, and then performing model simulations rapidly and easily by writing R scripts that make use of the compiled code through the `deSolve` package. Unfortunately, installing R, MCSim, and other required software and ensuring that they work together properly can be challenging and has presented obstacles for many in the PBPK modeling community. We developed the R package `MCSimMod` as an easy-to install, user-friendly software tool that takes advantage of the flexibility of R and the computational speed of MCSim to meet the needs of PBPK modelers (especially those already familiar with MCSim), but MCSimMod can be used to solve any IVP and it is therefore a valuable resource for anyone seeking to work with ODE models in R.

# Acknowledgements

The authors would like to acknowledge Dr. Celia Schacht and Dr. Caroline Ring for reviewing a preliminary draft of this manuscript and providing helpful suggestions for improvement.

# Disclaimer

The views expressed in this manuscript are those of the authors and do not necessarily represent the views or policies of the U.S. Environmental Protection Agency.

# References