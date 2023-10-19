#include "stress.h"
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <sstream>


const int rt_priority = 50;

stress::stress(type type, int num, int duration, int intensity)
    : m_type(type)
{
    pid_t fork_pid = fork();

    if(-1 == fork_pid) {
        throw std::runtime_error("[stress][#1] Could not fork.");
    }
    else if(0 == fork_pid) {
        if(num > 0) {
            pid_t current_pid = getpid();
            system(("chrt -o -p 0 " + std::to_string(current_pid)).c_str());

            int ret = 0;

            switch(type) {
            case CPU_RT: {
                if(intensity != -1) {
                    ret = execlp("/usr/bin/stress-ng", "/usr/bin/stress-ng", "--cpu", std::to_string(num).c_str(), "-t", std::to_string(duration).c_str(), "-l", std::to_string(intensity).c_str(), "--sched", "fifo", "--sched-prio", std::to_string(rt_priority).c_str(), nullptr);
                }
                else {
                    ret = execlp("/usr/bin/stress-ng", "/usr/bin/stress-ng", "--cpu", std::to_string(num).c_str(), "-t", std::to_string(duration).c_str(), "--sched", "fifo", "--sched-prio", std::to_string(rt_priority).c_str(), nullptr);
                }
                break;
            }
            case IO: {
                partition_id device_id = get_partition_id();
                pid_t current_pid = getpid();

                system("mount -t cgroup2 nodev /cgroup2");
                system("mkdir /cgroup2/cg2");
                system("echo \"+io\" > /cgroup2/cgroup.subtree_control");
                system(("echo \"" + std::to_string(device_id.major) + ":" + std::to_string(device_id.minor) + " wbps=1000000000\" > /cgroup2/cg2/io.max").c_str());
                system(("echo " + std::to_string(current_pid) + " > /cgroup2/cg2/cgroup.procs").c_str());

                ret = execlp("/usr/bin/stress-ng", "/usr/bin/stress-ng", "--hdd", std::to_string(num).c_str(), "-t", std::to_string(duration).c_str(), nullptr);
                break;
            }
            case TIMER: {
                ret = execlp("/usr/bin/stress-ng", "/usr/bin/stress-ng", "--timer", std::to_string(num).c_str(), "-t", std::to_string(duration).c_str(), nullptr);
                break;
            }
            }

            if(-1 == ret) {
                throw std::runtime_error("[stress][#2] Could not launch stress-ng. execlp failed.");
            }
        }
    }
}

stress::~stress() {
    stop();
}

void stress::stop() {
    switch (m_type) {
    case CPU_RT: case TIMER: case IO:
        system("killall stress-ng");
        break;
    }
}

partition_id stress::get_partition_id() {
    std::string device_name;
    char hostname[1024];
    gethostname(hostname, 1024);
    if(std::string(hostname) == "hpc1") {
        device_name = "nvme1n1";
    }
    else if(std::string(hostname) == "hpc2") {
        device_name = "nvme0n1";
    }
    else if(std::string(hostname) == "tpc1" || std::string(hostname) == "tpc2") {
        device_name = "sda";
    }
    else {
        throw std::runtime_error("[stress][#3] PC is unknown. Could not get io device name.");
    }

    std::ifstream partitions_file("/proc/partitions");
    if (!partitions_file.is_open()) {
        throw std::runtime_error("[stress][#4] Failed to open /proc/partitions.");
    }

    partition_id result = {0, 0};
    std::string line;
    while (std::getline(partitions_file, line)) {
        std::istringstream iss(line);
        std::string current_device_name;

        int major, minor, blocks;
        // Attempt to parse major, minor, and blocks fields
        if (iss >> major >> minor >> blocks) {
            // Read the rest of the line as the device name
            std::getline(iss, current_device_name);

            // Remove leading and trailing whitespace from device_name
            current_device_name = current_device_name.substr(current_device_name.find_first_not_of(" \t"), current_device_name.find_last_not_of(" \t") + 1);

            // Check if the device_name matches the one you're looking for (e.g., "nvme0n1")
            if (current_device_name == device_name) {
                result.major = major;
                result.minor = minor;
                break;
            }
        }
    }

    partitions_file.close();
    return result;
}
