library(MCSimMod)

# Get the full name of the package directory that contains the example MCSim
# model specification file.
mod_path <- file.path(system.file(package = "MCSimMod"), "extdata")

# Create a model object using the example MCSim model specification file
# "exponential.model" included in the MCSimMod package.
exp_mod_name <- file.path(mod_path, "exponential")
exp_mod <- createModel(exp_mod_name)

# Load the model.
exp_mod$loadModel()

# Change the values of the model parameters from their default values: set the
# initial amount (A0) to 100 and the exponential rate (r) to -0.5.
exp_mod$updateParms(c(A0 = 100, r = -0.5))

# Update the initial value(s) of the state variable(s) based on the updated
# parameter value(s).
exp_mod$updateY0()

# Define output times for simulation.
times <- seq(from = 0, to = 20, by = 0.1)

# Run simulation.
out <- exp_mod$runModel(times)

# Plot simulation results.
file_name = "ExponentialModelSimulation.png"
png(file_name, units="in", width=6, height=4, res=300)
plot(out[, "time"], out[, "A"], type = "l", lty = 1, lwd = 2, xlab = "Time",
     ylab = "Amount")
dev.off()