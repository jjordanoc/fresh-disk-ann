import matplotlib.pyplot as plt
import csv

# Read data from CSV
cycles = []
recall_5 = []

filename = "cmake-build-relwithdebinfo/results.csv"

# Open the CSV file
with open(filename, 'r') as file:
    reader = csv.reader(file)
    for row in reader:
        cycles.append(float(row[0]))  # First column: cycles
        recall_5.append(float(row[1]))  # Second column: 5-recall@5

# Plot the data
plt.figure(figsize=(10, 6))
plt.plot(cycles, recall_5, marker='o', color='b', label='5-recall@5')

# Adding labels and title
plt.xlabel('Cycles')
plt.ylabel('5-recall@5')
plt.title('Recall vs. Cycles')
plt.legend()

# Show the plot
plt.grid(True)
# plt.tight_layout()
plt.show()
