#include "./acc_lib/allan_acc.h"
#include "./acc_lib/fitallan_acc.h"
#include "./gyr_lib/allan_gyr.h"
#include "./gyr_lib/fitallan_gyr.h"
#include "type.h"

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <string>

// Global pointers for Allan objects
imu::AllanGyr *gyr_x;
imu::AllanGyr *gyr_y;
imu::AllanGyr *gyr_z;
imu::AllanAcc *acc_x;
imu::AllanAcc *acc_y;
imu::AllanAcc *acc_z;

// ---------------- Utility functions ----------------

// Split a string by a pattern
static inline std::vector<std::string> SplitString(const std::string &str,
                                                   const std::string &pattern) {
    std::vector<std::string> res;
    if (str.empty()) return res;

    std::string strs = str + pattern;
    size_t pos = strs.find(pattern);
    while (pos != std::string::npos) {
        res.push_back(strs.substr(0, pos));
        strs = strs.substr(pos + 1);
        pos = strs.find(pattern);
    }
    return res;
}

// Convert time "yyyy-mm-dd hh:MM:ss" to timestamp in seconds
static inline time_t StandardTimeToTimestampSecond(const std::string &time_string) {
    struct tm tm{};
    sscanf(time_string.c_str(), "%d-%d-%d %d:%d:%d",
           &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
           &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
    tm.tm_year -= 1900;
    tm.tm_mon--;
    return mktime(&tm);
}

// Convert time "yyyy-mm-dd hh:MM:ss.mmmmmm" to milliseconds
static inline double StandardTimeToTimestampMilliSecond(const std::string &time_string) {
    double second = StandardTimeToTimestampSecond(time_string.substr(0, 19));
    double millisecond = std::stod(time_string.substr(20, 6));
    return second * 1000 + millisecond;
}

// ---------------- IMU reading functions ----------------

// Read Glog format
bool ReadImuDataGlog(const std::string &file) {
    std::ifstream infile(file);
    std::string line;
    while (std::getline(infile, line)) {
        if (line.find("#") != std::string::npos) continue;
        auto substrs = SplitString(line, ",");
        if (substrs.size() != 7) return false;

        std::string yy = "2021";
        std::string mm = SplitString(line, " ")[0].substr(1, 2);
        std::string dd = SplitString(line, " ")[0].substr(3, 2);
        std::string hhmmss_mmmmmm = SplitString(line, " ")[1];
        std::string timestamp_string = yy + "-" + mm + "-" + dd + " " + hhmmss_mmmmmm;
        double timestamp = StandardTimeToTimestampMilliSecond(timestamp_string) / 1000.0;

        ImuReading imu_reading(timestamp,
                               std::stod(substrs[1]),
                               std::stod(substrs[2]),
                               std::stod(substrs[3]),
                               std::stod(substrs[4]),
                               std::stod(substrs[5]),
                               std::stod(substrs[6]));

        gyr_x->PushRadPerSec(imu_reading.gyro.x, imu_reading.timestamp);
        gyr_y->PushRadPerSec(imu_reading.gyro.y, imu_reading.timestamp);
        gyr_z->PushRadPerSec(imu_reading.gyro.z, imu_reading.timestamp);
        acc_x->pushMPerSec2(imu_reading.acc.x, imu_reading.timestamp);
        acc_y->pushMPerSec2(imu_reading.acc.y, imu_reading.timestamp);
        acc_z->pushMPerSec2(imu_reading.acc.z, imu_reading.timestamp);
    }
    return true;
}

// Read simple format
bool ReadImuDataSimpleFormat(const std::string &file) {
    std::ifstream infile(file);
    std::string line;
    while (std::getline(infile, line)) {
        if (line.find("#") != std::string::npos) continue;
        auto substrs = SplitString(line, " ");
        if (substrs.size() != 7) return false;

        double timestamp = std::stod(substrs[0]);
        ImuReading imu_reading(timestamp,
                               std::stod(substrs[1]),
                               std::stod(substrs[2]),
                               std::stod(substrs[3]),
                               std::stod(substrs[4]),
                               std::stod(substrs[5]),
                               std::stod(substrs[6]));

        gyr_x->PushRadPerSec(imu_reading.gyro.x, imu_reading.timestamp);
        gyr_y->PushRadPerSec(imu_reading.gyro.y, imu_reading.timestamp);
        gyr_z->PushRadPerSec(imu_reading.gyro.z, imu_reading.timestamp);
        acc_x->pushMPerSec2(imu_reading.acc.x, imu_reading.timestamp);
        acc_y->pushMPerSec2(imu_reading.acc.y, imu_reading.timestamp);
        acc_z->pushMPerSec2(imu_reading.acc.z, imu_reading.timestamp);
    }
    return true;
}
#if 0
// ---------------- Read IMU CSV: accel x y z, gyro x y z ----------------
bool ReadImuDataCSV(const std::string &file, double sample_dt = 0.01) {
    // sample_dt = time between samples in seconds (default 100 Hz)
    std::ifstream infile(file);
    if (!infile.is_open()) {
        std::cerr << "Failed to open file: " << file << std::endl;
        return false;
    }

    std::string line;
    double timestamp = 0.0;  // start timestamp
    while (std::getline(infile, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto substrs = SplitString(line, ",");
        if (substrs.size() != 6) {
            std::cerr << "Invalid CSV format, line: " << line << std::endl;
            return false;
        }

        // Parse acceleration
        double ax = std::stod(substrs[0]);
        double ay = std::stod(substrs[1]);
        double az = std::stod(substrs[2]);

        // Parse gyroscope
        double gx = std::stod(substrs[3]);
        double gy = std::stod(substrs[4]);
        double gz = std::stod(substrs[5]);

        // Push data to Allan classes
        acc_x->pushMPerSec2(ax, timestamp);
        acc_y->pushMPerSec2(ay, timestamp);
        acc_z->pushMPerSec2(az, timestamp);

        gyr_x->PushRadPerSec(gx, timestamp);
        gyr_y->PushRadPerSec(gy, timestamp);
        gyr_z->PushRadPerSec(gz, timestamp);

        timestamp += sample_dt; // increment timestamp
    }

    return true;
}
#endif
// Read IMU CSV data with timestamp in the first column
bool ReadImuDataCSV(const std::string &file) {
    std::ifstream infile(file);
    if (!infile.is_open()) {
        std::cerr << "Failed to open file: " << file << std::endl;
        return false;
    }

    std::string line;
    bool first_line = true;

    while (std::getline(infile, line)) {
        if (line.empty() || line[0] == '#') continue;

        // Skip header if present
        if (first_line) {
            first_line = false;
            if (line.find("time") != std::string::npos) continue;
        }

        auto substrs = SplitString(line, ",");
        if (substrs.size() != 7) {
            std::cerr << "Invalid CSV format, line: " << line << std::endl;
            return false;
        }
         
	std::string csv_time = (substrs[0]);
        double timestamp = std::stod(csv_time)/1e9;
        double ax = std::stod(substrs[1]);
        double ay = std::stod(substrs[2]);
        double az = std::stod(substrs[3]);
        double gx = std::stod(substrs[4]);
        double gy = std::stod(substrs[5]);
        double gz = std::stod(substrs[6]);

        // Push data to Allan objects
        acc_x->pushMPerSec2(ax, timestamp);
        acc_y->pushMPerSec2(ay, timestamp);
        acc_z->pushMPerSec2(az, timestamp);

        gyr_x->PushRadPerSec(gx, timestamp);
        gyr_y->PushRadPerSec(gy, timestamp);
        gyr_z->PushRadPerSec(gz, timestamp);
    }

    infile.close();
    return true;
}

// ---------------- Data writing functions ----------------
void WriteDataAllThreeAxis(const std::string &data_save_path,
                           const std::string &data_name,
                           const std::vector<double> &timestamp,
                           const std::vector<double> &write_data_x,
                           const std::vector<double> &write_data_y,
                           const std::vector<double> &write_data_z) {
    std::ofstream out_t(data_save_path + "data_" + data_name + "_t.txt", std::ios::trunc);
    std::ofstream out_x(data_save_path + "data_" + data_name + "_x.txt", std::ios::trunc);
    std::ofstream out_y(data_save_path + "data_" + data_name + "_y.txt", std::ios::trunc);
    std::ofstream out_z(data_save_path + "data_" + data_name + "_z.txt", std::ios::trunc);

    out_t << std::setprecision(10);
    out_x << std::setprecision(10);
    out_y << std::setprecision(10);
    out_z << std::setprecision(10);

    for (size_t i = 0; i < timestamp.size(); ++i) {
        out_t << timestamp[i] << "\n";
        out_x << write_data_x[i] << "\n";
        out_y << write_data_y[i] << "\n";
        out_z << write_data_z[i] << "\n";
    }
}

// ---------------- Main function ----------------
int main() {
    std::string imu_data_path = "../data/imu_raw.csv";
    std::string imu_name = "imuSimulation";
    std::string data_save_path = "../data/";
    int max_cluster = 100;

    // Create Allan objects
    gyr_x = new imu::AllanGyr("gyr x", max_cluster);
    gyr_y = new imu::AllanGyr("gyr y", max_cluster);
    gyr_z = new imu::AllanGyr("gyr z", max_cluster);
    acc_x = new imu::AllanAcc("acc x", max_cluster);
    acc_y = new imu::AllanAcc("acc y", max_cluster);
    acc_z = new imu::AllanAcc("acc z", max_cluster);

    // Read IMU data
    std::cout << "Reading IMU data..." << std::endl;
    //if (!ReadImuDataSimpleFormat(imu_data_path)) return 0;
    if (!ReadImuDataCSV(imu_data_path)) return 0; // assuming 100 Hz
    // Calculate Allan variance
    gyr_x->CalculateAllanVariance();
    gyr_y->CalculateAllanVariance();
    gyr_z->CalculateAllanVariance();
    acc_x->CalculateAllanVariance();
    acc_y->CalculateAllanVariance();
    acc_z->CalculateAllanVariance();

    // Fit Allan results
    imu::FitAllanGyr fit_gyr_x(gyr_x->GetVariance(), gyr_x->GetTimestamp(), gyr_x->GetFrequency());
    imu::FitAllanGyr fit_gyr_y(gyr_y->GetVariance(), gyr_y->GetTimestamp(), gyr_y->GetFrequency());
    imu::FitAllanGyr fit_gyr_z(gyr_z->GetVariance(), gyr_z->GetTimestamp(), gyr_z->GetFrequency());
    imu::FitAllanAcc fit_acc_x(acc_x->GetVariance(), acc_x->GetTimestamp(), acc_x->GetFrequency());
    imu::FitAllanAcc fit_acc_y(acc_y->GetVariance(), acc_y->GetTimestamp(), acc_y->GetFrequency());
    imu::FitAllanAcc fit_acc_z(acc_z->GetVariance(), acc_z->GetTimestamp(), acc_z->GetFrequency());

    // Write data
    WriteDataAllThreeAxis(data_save_path, imu_name + "_gyr", gyr_x->GetTimestamp(), gyr_x->GetDeviation(), gyr_y->GetDeviation(), gyr_z->GetDeviation());
    WriteDataAllThreeAxis(data_save_path, imu_name + "_acc", acc_x->GetTimestamp(), acc_x->GetDeviation(), acc_y->GetDeviation(), acc_z->GetDeviation());

    // Write fitted data
    WriteDataAllThreeAxis(data_save_path, imu_name + "_fitting_gyr", gyr_x->GetTimestamp(),
                          fit_gyr_x.CalculateSimDeviation(gyr_x->GetTimestamp()),
                          fit_gyr_y.CalculateSimDeviation(gyr_y->GetTimestamp()),
                          fit_gyr_z.CalculateSimDeviation(gyr_z->GetTimestamp()));
    WriteDataAllThreeAxis(data_save_path, imu_name + "_fitting_acc", acc_x->GetTimestamp(),
                          fit_acc_x.CalculateSimDeviation(acc_x->GetTimestamp()),
                          fit_acc_y.CalculateSimDeviation(acc_y->GetTimestamp()),
                          fit_acc_z.CalculateSimDeviation(acc_z->GetTimestamp()));

    std::cout << "Processing done. Data saved to " << data_save_path << std::endl;

    return 0;
}

