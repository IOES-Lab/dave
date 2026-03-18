import os

import numpy as np
import matplotlib.pyplot as plt

# -------------------------------------------
# CONFIGURATION
# -------------------------------------------
titletext = "Ray Range Reduced/10m Range/512 Beams/300 Rays"
clims_base = [-60, 0]
nBeams = 512
maxRange = 5
xPlotRange = 10
yPlotRange = xPlotRange * np.cos(45 * np.pi / 180)

results_dir = os.path.join(os.path.dirname(__file__), "..", "..", "..", "..", "results")
os.makedirs(results_dir, exist_ok=True)

filename = os.path.join(results_dir, "SonarRawData_000001.csv")
beam_angle_file = os.path.join(results_dir, "SonarRawData_beam_angles.csv")
bw = 29.9e3
plotSkips = 1
epsilon = 1e-10
scatter_output_file = os.path.join(results_dir, "SonarScatter.png")
beam_output_file = os.path.join(results_dir, "SonarBeamProfiles.png")


# -------------------------------------------
# LOAD COMPLEX CSV DATA
# -------------------------------------------
def load_complex_csv(filepath):
    with open(filepath) as f:
        lines = f.readlines()

    data_lines = [line for line in lines if not line.startswith("#")]
    data_lines = [line.replace("i", "j") for line in data_lines]
    data = []
    for line in data_lines:
        str_vals = line.strip().split(",")
        row = [complex(val) for val in str_vals]
        data.append(row)

    return np.array(data, dtype=complex)


Data = load_complex_csv(filename)

# -------------------------------------------
# PROCESS DATA
# -------------------------------------------
range_vector = Data[:, 0].real
echo_data = Data[:, 1 : nBeams + 1 : plotSkips] * np.sqrt(3)
plotData = np.abs(echo_data)
scatter_data = 20 * np.log10(plotData + epsilon)

sonarBeams = np.genfromtxt(beam_angle_file, delimiter=",", skip_header=4)
sonarBeams = sonarBeams.flatten()[: echo_data.shape[1]]

range_grid, beam_grid = np.meshgrid(range_vector, sonarBeams, indexing="ij")
x = range_grid * np.cos(beam_grid)
y = range_grid * np.sin(beam_grid)
# -------------------------------------------
# SCATTER PLOT
# -------------------------------------------
plt.figure(figsize=(10, 6))
scatterPointSize = 8
plt.scatter(
    x.flatten(), y.flatten(), scatterPointSize, scatter_data.flatten(), cmap="hot", marker="o"
)

clims = clims_base + 20 * np.log10(np.max(plotData))
plt.clim(clims)
plt.colorbar(label="Echo Level")
plt.title(titletext)
plt.xlabel("X [m]")
plt.ylabel("Y [m]")
plt.axis("equal")
plt.xlim(1.02 * np.array([0, xPlotRange]))
plt.ylim(1.02 * np.array([-yPlotRange, yPlotRange]))
plt.gca().set_facecolor("k")
plt.tight_layout()
plt.savefig(scatter_output_file, dpi=200)
plt.close()

# -------------------------------------------
# LINE PLOTS FOR INDIVIDUAL BEAMS
# -------------------------------------------
plt.figure(figsize=(14, 4))
iPlots = np.arange(1, nBeams, 30)
nPlots = len(iPlots)

for i, idx in enumerate(iPlots[1:-1], start=1):
    temp = np.abs(Data[:, idx])
    plt.subplot(1, nPlots - 2, i)
    plt.plot(temp[: len(range_vector)], range_vector)
    plt.ylim(1.02 * np.array([0, xPlotRange]))
    plt.xlabel("Echo Strength")
    if i == 1:
        plt.ylabel("Range [m]")

plt.suptitle("Beam Profiles")
plt.tight_layout()
plt.savefig(beam_output_file, dpi=200)
plt.close()

print(f"Saved {scatter_output_file}")
print(f"Saved {beam_output_file}")
