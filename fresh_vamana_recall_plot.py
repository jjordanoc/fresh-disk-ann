import matplotlib.pyplot as plt
import csv
import numpy as np
from scipy.interpolate import CubicSpline

# Function to get the file names for each measurement
def get_measurements(number):
    return f"cmake-build-relwithdebinfo/results-{number}.csv"

out_degree_bounds = [25, 37, 50]

# List of measurements (file names)
measurements = [get_measurements(i) for i in out_degree_bounds]  # Add other numbers if needed

# Define a list of colors to differentiate the curves
colors = ['b', 'g', 'r', 'c', 'm', 'y', 'k']  # Extend as necessary

# Create the plot
plt.figure(figsize=(10, 6))

# Loop over each measurement file and plot the data
for idx, filename in enumerate(measurements):
    cycles = []
    recall_5 = []

    # Open the CSV file and read data
    with open(filename, 'r') as file:
        reader = csv.reader(file)
        next(reader)  # Skip header if there is one
        for row in reader:
            cycles.append(float(row[0]))  # First column: cycles
            recall_5.append(float(row[1]))  # Second column: 5-recall@5

    # Convert lists to numpy arrays
    cycles = np.array(cycles)
    recall_5 = np.array(recall_5)

    # Interpolation using cubic splines for smoother curves
    cs = CubicSpline(cycles, recall_5)

    # Generate a set of points for a smooth curve
    x_new = np.linspace(min(cycles), max(cycles), 500)
    y_new = cs(x_new)

    # Plot the interpolated curve
    plt.plot(cycles, recall_5, label=f'Out degree bound = {out_degree_bounds[measurements.index(filename)]}', color=colors[idx % len(colors)])

# Adding labels, title, and legend
plt.xlabel('Cycles')
plt.ylabel('5-recall@5')
plt.title('5-recall@5 vs. Cycles')
plt.legend()

# Show the plot
plt.grid(True)
plt.tight_layout()
plt.show()
