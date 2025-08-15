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

# Change the values of the model parameters from their default values.
pk1_mod$updateParms(c(A0_init = 0))

# Update the initial value(s) of the state variable(s) based on the updated
# parameter value(s).
pk1_mod$updateY0()

# Create an events data frame.
events_df <- data.frame(var = "A0", time = seq(from = 0, to = 48, by = 12),
                        value = 50, method = "add")

# Define output times for simulation.
times <- seq(from = 0, to = 48, by = 0.1)

# Run simulation.
out <- pk1_mod$runModel(times, events = list(data = events_df))

# Plot simulation results.
file_name = "PK1EventsSimulation.png"
png(file_name, units="in", width=6, height=4, res=300)
plot(out[, "time"], out[, "C"], type = "l", lty = 1, lwd = 2,
     xlab = "Time (h)", ylab = "Concentration (mg/L)")
dev.off()