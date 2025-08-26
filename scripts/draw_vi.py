import matplotlib.pyplot as plt

# ---------------------------
# Utility to read numeric data from a file
# ---------------------------
def read_from_simple_file(file_path='./log.txt'):
    data_list = []
    with open(file_path, 'r') as file:
        for line in file:
            line = line.strip()
            if not line or '#' in line:
                continue
            data_list.append(float(line))
    return data_list

# ---------------------------
# Read N and K values from acccurve/gyrocurve files
# ---------------------------
def read_curve_NK(file_path):
    N = K = None
    with open(file_path, 'r') as f:
        for line in f:
            line = line.strip()
            if line.startswith("White Veloc. Noise (N):"):
                N = float(line.split()[4])  # m/s/sqrt(s)
            elif line.startswith("Accel. Random Walk (K):") or line.startswith("Gyro Random Walk (K):"):
                K = float(line.split()[4])  # m/s^2/sqrt(s) or rad/s^2/sqrt(s)
    return N, K

# ---------------------------
# Plot function with N/K values displayed below figure
# ---------------------------
def viz_allan_variance(title="", all_data=[("label", [], [])], curve_files=None):
    plt.figure(figsize=(8, 6))

    # Plot measured + simulated data
    for data_i in all_data:
        label, x, y = data_i
        if 'sim' in label:
            plt.plot(x, y, label=label)
        else:
            plt.scatter(x, y, label=label, s=8)
        plt.loglog(x, y)

    # Write N/K values below plot
    text_lines = []
    if curve_files:
        axes = ['X', 'Y', 'Z']
        for i, file in enumerate(curve_files):
            N, K = read_curve_NK(file)
            text_lines.append(f"{axes[i]} axis: N = {N:.6e}, K = {K:.6e}")

        # adjust bottom margin for text
        plt.subplots_adjust(bottom=0.25)
        plt.figtext(0.5, 0.01, "\n".join(text_lines), ha='center', fontsize=10)

    plt.xlabel(r'$\tau [sec]$')
    # Determine ylabel from first label
    if 'gyro' in all_data[0][0]:
        plt.ylabel(r'$\sigma(\tau) [rad/s]$')
    elif 'acc' in all_data[0][0]:
        plt.ylabel(r'$\sigma(\tau) [m/s^2]$')

    plt.grid()
    plt.legend()
    plt.title(title)
    plt.savefig("./" + title + ".png", bbox_inches='tight')
    plt.show()

# ---------------------------
# Main function
# ---------------------------
def main():
    print("Allan Variance Plot BMI270 with N/K values below plots.")

    imu_name = "imuSimulation"

    # ---------- Gyroscope data ----------
    gyro_time_list = read_from_simple_file(f'../data/data_{imu_name}_gyr_t.txt')
    gyro_x_list = read_from_simple_file(f'../data/data_{imu_name}_gyr_x.txt')
    gyro_y_list = read_from_simple_file(f'../data/data_{imu_name}_gyr_y.txt')
    gyro_z_list = read_from_simple_file(f'../data/data_{imu_name}_gyr_z.txt')

    sim_gyro_x_list = read_from_simple_file(f'../data/data_{imu_name}_fitting_gyr_x.txt')
    sim_gyro_y_list = read_from_simple_file(f'../data/data_{imu_name}_fitting_gyr_y.txt')
    sim_gyro_z_list = read_from_simple_file(f'../data/data_{imu_name}_fitting_gyr_z.txt')

    viz_allan_variance(
        title="allan_variance_of_gyro_bmi270",
        all_data=[("gyro_x", gyro_time_list, gyro_x_list),
                  ("gyro_y", gyro_time_list, gyro_y_list),
                  ("gyro_z", gyro_time_list, gyro_z_list),
                  ("sim_gyro_x", gyro_time_list, sim_gyro_x_list),
                  ("sim_gyro_y", gyro_time_list, sim_gyro_y_list),
                  ("sim_gyro_z", gyro_time_list, sim_gyro_z_list)],
        curve_files=['../data/gyrocurve_0.txt',
                     '../data/gyrocurve_1.txt',
                     '../data/gyrocurve_2.txt']
    )

    # ---------- Accelerometer data ----------
    acc_time_list = read_from_simple_file(f'../data/data_{imu_name}_acc_t.txt')
    acc_x_list = read_from_simple_file(f'../data/data_{imu_name}_acc_x.txt')
    acc_y_list = read_from_simple_file(f'../data/data_{imu_name}_acc_y.txt')
    acc_z_list = read_from_simple_file(f'../data/data_{imu_name}_acc_z.txt')

    sim_acc_x_list = read_from_simple_file(f'../data/data_{imu_name}_fitting_acc_x.txt')
    sim_acc_y_list = read_from_simple_file(f'../data/data_{imu_name}_fitting_acc_y.txt')
    sim_acc_z_list = read_from_simple_file(f'../data/data_{imu_name}_fitting_acc_z.txt')

    viz_allan_variance(
        title="allan_variance_of_acc_bmi270",
        all_data=[("acc_x", acc_time_list, acc_x_list),
                  ("acc_y", acc_time_list, acc_y_list),
                  ("acc_z", acc_time_list, acc_z_list),
                  ("sim_acc_x", acc_time_list, sim_acc_x_list),
                  ("sim_acc_y", acc_time_list, sim_acc_y_list),
                  ("sim_acc_z", acc_time_list, sim_acc_z_list)],
        curve_files=['../data/acccurve_0.txt',
                     '../data/acccurve_1.txt',
                     '../data/acccurve_2.txt']
    )


if __name__ == "__main__":
    main()

