import matplotlib.pyplot as plt
import csv
import numpy as np
from scipy.interpolate import CubicSpline

MODE = "ALPHA"

if MODE == "INDEX_SIZE":
    # Function to get the filename based on Out degree bound, Delete percentage, and Alpha
    def get_measurement_filename(out_degree_bound, delete_percentage):
        return f"cmake-build-relwithdebinfo/results-{out_degree_bound}-{delete_percentage}-{1.2}.csv"

    # Delete percentages and out degree bounds to iterate over
    delete_percentages = [0.05, 0.1, 0.5]
    out_degree_bounds = [37]

    # Create a 3x1 grid of subplots (one for each delete percentage)
    fig, axes = plt.subplots(3, 1, figsize=(18, 10))

    # Set titles for each subplot (corresponding to the delete percentage)
    for i, dp in enumerate(delete_percentages):
        axes[i].set_title(f"Delete Percentage = {dp * 100}%")
        axes[i].set_xlabel('Cycles')
        axes[i].set_ylabel('5-recall@5')
        axes[i].grid(True)

        # Loop over the out degree bounds and plot their curves on the current subplot
        for out_degree_bound in out_degree_bounds:
            cycles = []
            recall_5 = []
            filename = get_measurement_filename(out_degree_bound, dp)

            try:
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

                # Generate a smooth line for the curve
                x_new = np.linspace(min(cycles), max(cycles), 500)
                y_new = cs(x_new)

                # Plot the interpolated curve
                axes[i].plot(x_new, y_new, label=f"Out Degree {out_degree_bound}", linewidth=2)

            except FileNotFoundError:
                print(f"File {filename} not found. Skipping.")

        # Add a legend to each subplot
        axes[i].legend()

    # Adjust the layout to prevent overlapping
    plt.tight_layout()

    # Show the plots
    plt.show()
elif MODE == "OUT_DEGREE":
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
        plt.plot(x_new, y_new, label=f'Out degree bound = {out_degree_bounds[measurements.index(filename)]}', color=colors[idx % len(colors)])

    # Adding labels, title, and legend
    plt.xlabel('Cycles')
    plt.ylabel('5-recall@5')
    plt.title('5-recall@5 vs. Cycles')
    plt.legend()

    # Show the plot
    plt.grid(True)
    plt.tight_layout()
    plt.show()
elif MODE == "ALPHA":
    # Function to get the filename based on Out degree bound, Delete percentage, and Alpha
    def get_measurement_filename(out_degree_bound, delete_percentage, alpha):
        return f"cmake-build-relwithdebinfo/results-{out_degree_bound}-{delete_percentage}-{alpha}.csv"

    # Fixed parameters
    out_degree_bound = 37
    delete_percentage = 0.05

    # Alpha values to iterate over
    alpha_values = [1.0, 1.1, 1.2]

    # Define colors for each alpha
    alpha_colors = {
        1.0: 'b',   # Blue for alpha = 1.0
        1.1: 'g',   # Green for alpha = 1.1
        1.2: 'r'    # Red for alpha = 1.2
    }

    # Create the plot
    plt.figure(figsize=(10, 6))

    # Loop over alpha values and plot the data
    for alpha in alpha_values:
        cycles = []
        recall_5 = []

        # Get the filename for this alpha
        filename = get_measurement_filename(out_degree_bound, delete_percentage, alpha)

        # Open the CSV file and read data
        try:
            with open(filename, 'r') as file:
                reader = csv.reader(file)
                next(reader)  # Skip header if there is one
                for row in reader:
                    cycles.append(float(row[0]))  # First column: cycles
                    recall_5.append(float(row[1]))  # Second column: 5-recall@5

            # Convert lists to numpy arrays for interpolation
            cycles = np.array(cycles)
            recall_5 = np.array(recall_5)

            # Interpolation using cubic splines for smoother curves
            cs = CubicSpline(cycles, recall_5)

            # Generate a set of points for a smooth curve
            x_new = np.linspace(min(cycles), max(cycles), 500)
            y_new = cs(x_new)

            # Plot the interpolated curve with the specified color for this alpha value
            plt.plot(x_new, y_new, label=f'Alpha = {alpha}', color=alpha_colors[alpha])

        except FileNotFoundError:
            print(f"Warning: File {filename} not found. Skipping alpha = {alpha}")

    # Adding labels and title
    plt.xlabel('Cycles')
    plt.ylabel('5-recall@5')
    plt.title(f'Recall vs Cycles (Out Degree = {out_degree_bound}, Delete Percentage = {delete_percentage})')

    # Adding a legend to differentiate the curves by alpha
    plt.legend()

    # Show the plot
    plt.grid(True)
    plt.tight_layout()
    plt.show()
