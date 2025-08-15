library(MCSimMod)

# Get the full name of the package directory that contains the example MCSim
# model specification file.
mod_path <- file.path(system.file(package = "MCSimMod"), "extdata")

# Create a model object using the example MCSim model specification file
# "pbpk_simple.model" included in the MCSimMod package.
pbpk_mod_name <- file.path(mod_path, "pbpk_simple")
pbpk_mod <- createModel(pbpk_mod_name)

# Load the model.
pbpk_mod$loadModel()

# Change oral dose rate to 1 mg/kg/d.
pbpk_mod$updateParms(c(D_oral = 1))

# Define output times for simulation.
times <- seq(from = 0, to = 100, by = 0.1)

# Run simulation.
out_oral <- pbpk_mod$runModel(times)

# Plot total amount lost vs. time.
file_name = "amount_lost_vs_time.png"
png(file_name, units="in", width=6, height=4, res=300)
plot(out_oral[, "time"], out_oral[, "A_tot"],
  type = "l", lty = 1, lwd = 2,
  xlab = "Time (h)", ylab = "Amount (mg)"
)
dev.off()

# Plot concentration in venous blood vs. time.
file_name = "conc_vb_vs_time.png"
png(file_name, units="in", width=6, height=4, res=300)
plot(out_oral[, "time"], out_oral[, "C_VB"],
  type = "l", lty = 1, lwd = 2,
  xlab = "Time (h)", ylab = "Concentration (mg/L)"
)
dev.off()

# Plot concentration in non-blood compartments vs. time.
file_name = "conc_comp_vs_time.png"
png(file_name, units="in", width=6, height=4, res=300)
plot(out_oral[, "time"], out_oral[, "C_L"],
  type = "l", lty = 1, lwd = 2,
  log = "y", ylim = c(0.0001, 10),
  xlab = "Time (h)", ylab = "Concentration (mg/L)"
)
lines(out_oral[, "time"], out_oral[, "C_F"],
  lty = 2, lwd = 2
)
lines(out_oral[, "time"], out_oral[, "C_R"],
  lty = 3, lwd = 2
)
legend("bottomright",
  legend = c("Liver", "Fat", "Rest of Body"),
  lty = c(1, 2, 3), lwd = 2
)
dev.off()