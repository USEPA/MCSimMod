library(MCSimMod)

# Get the full name of the package directory that contains the example MCSim
# model specification file.
mod_path <- file.path(system.file(package = "MCSimMod"), "extdata")

# Create a model object using the example MCSim model specification file
# "pred_prey.model" included in the MCSimMod package.
pp_mod_name <- file.path(mod_path, "pred_prey")
pp_mod <- createModel(pp_mod_name)

# Load the model.
pp_mod$loadModel()

# Define output times for simulation.
times <- seq(from = 0, to = 50, by = 0.1)

# Run simulation.
out <- pp_mod$runModel(times)

# Plot simulation results.
file_name = "PredPreyModelSimulation.png"
png(file_name, units="in", width=6, height=4, res=300)
plot(out[, "time"], out[, "x"], type = "l", lty = 1, lwd = 2,
     xlab = "Time (d)", ylab = "Number (1000s)", ylim = c(0, 2))
lines(out[, "time"], out[, "y"], lty = 2, lwd = 2)
legend("topright", c("Rabbits", "Foxes"), lty = c(1, 2), lwd = 2)
dev.off()

# Plot simulation results.
file_name = "PredPreyModelSimulationPhasePlot.png"
png(file_name, units="in", width=6, height=6, res=300)
plot(out[, "x"], out[, "y"], type = "l", lty = 1, lwd = 2,
     xlab = "Number of Rabbits (1000s)", ylab = "Number of Foxes (1000s)")
dev.off()