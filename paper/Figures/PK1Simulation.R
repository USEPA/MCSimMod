library(MCSimMod)

# Get the full name of the package directory that contains the example MCSim
# model specification file.
mod_path <- file.path(system.file(package = "MCSimMod"), "extdata")

# Create a model object using the example MCSim model specification file
# "pred_prey.model" included in the MCSimMod package.
pk1_mod_name <- file.path(mod_path, "pk1")
pk1_mod <- createModel(pk1_mod_name)

# Load the model.
pk1_mod$loadModel()

# Change the values of the model parameters.
pk1_mod$updateParms(c(k01 = 0.8, k12 = 0.4, Vd = 45, A0_init = 200))

# Update the initial value(s) of the state variable(s) based on the updated
# parameter value(s).
pk1_mod$updateY0()

# Define output times for simulation.
times <- seq(from = 0, to = 20, by = 0.1)

# Run simulation.
out_oral <- pk1_mod$runModel(times)

# Plot simulation results.
file_name = "PK1SimulationOral.png"
png(file_name, units="in", width=6, height=4, res=300)
plot(out_oral[, "time"], out_oral[, "C"], type = "l", lty = 1, lwd = 2,
     xlab = "Time (h)", ylab = "Concentration (mg/L)")
dev.off()

# Change the values of the model parameters.
pk1_mod$updateParms(c(k12 = 0.4, Vd = 45, A0_init = 0, A1_init = 200))

# Update the initial value(s) of the state variable(s) based on the updated
# parameter value(s).
pk1_mod$updateY0()

# Run simulation.
out_IV <- pk1_mod$runModel(times)

# Plot simulation results.
file_name = "PK1SimulationIV.png"
png(file_name, units="in", width=6, height=4, res=300)
plot(out_IV[, "time"], out_IV[, "C"], type = "l", lty = 1, lwd = 2,
     xlab = "Time (h)", ylab = "Concentration (mg/L)")
dev.off()