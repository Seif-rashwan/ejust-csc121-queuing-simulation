/**
 * @file CliMain.cpp
 * @brief CLI entry point for the queuing simulation with an animated dashboard.
 *
 * Usage:
 * Without arguments, the simulation prompts for parameters interactively.

 * $ simulation_cli
 * $ simulation_cli <servers> <transMin> <transMax> <arrivalMin> <arrivalMax>
 *                  <totalCustomers> <delayMs>
 */

#include <algorithm>
#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

#include "WebSimulation.h"

using std::cerr;
using std::cin;
using std::cout;
using std::format;
using std::string;

int ParsePositiveInt(const string& value, const string& name) {
    int parsed = std::stoi(value);
    if (parsed <= 0) {
        throw std::invalid_argument(std::string(name) + " must be a positive integer.");
    }
    return parsed;
}

/**
 * @brief Hides cursor temporarily until the code block ends.
 */
class CursorHide {
   public:
    CursorHide() {
        cout << "\033[?25l" << std::flush;
    }

    ~CursorHide() {
        cout << "\033[?25h" << std::flush;
    }

    CursorHide(const CursorHide&)            = delete;
    CursorHide& operator=(const CursorHide&) = delete;
};

class CLIApplication {
   private:
    int sim_cap_    = WebSimulation::SIMULATION_TIME_CAP_DEFAULT;
    int servers_    = 4;
    int trans_min_  = WebSimulation::DEFAULT_TRANS_MIN;
    int trans_max_  = WebSimulation::DEFAULT_TRANS_MAX;
    int arr_min_    = WebSimulation::DEFAULT_ARRIVAL_MIN;
    int arr_max_    = WebSimulation::DEFAULT_ARRIVAL_MAX;
    int total_cust_ = WebSimulation::DEFAULT_TOT_CUSTOMERS;
    int delay_ms_   = 50;
    std::ofstream log_file_;

    void readCLIParameters() {
        cout << "======================================\n";
        cout << "   Queuing System Simulation - CLI\n";
        cout << "======================================\n\n";

        auto read_positive = [](const string& prompt, int& out) {
            cout << prompt;
            if (!(cin >> out) || out <= 0) {
                throw std::invalid_argument("Value must be a positive integer.");
            }
        };

        read_positive("Number of servers        : ", servers_);
        read_positive("Min service time (ticks) : ", trans_min_);
        read_positive("Max service time (ticks) : ", trans_max_);
        read_positive("Min arrival interval     : ", arr_min_);
        read_positive("Max arrival interval     : ", arr_max_);
        read_positive("Total customers          : ", total_cust_);
        read_positive("Tick delay (ms)          : ", delay_ms_);

        if (trans_min_ > trans_max_) {
            std::swap(trans_min_, trans_max_);
        }

        if (arr_min_ > arr_max_) {
            std::swap(arr_min_, arr_max_);
        }
    }

    static void printCLIUsage() {
        cerr << "Usage: ./simulation_cli [servers transMin transMax arrivalMin arrivalMax "
                "totalCustomers delayMs]\n";
    }

    static void printDashboard(int clock, const WebSimulation& sim) {
        constexpr int INNER = 66;  // width between ║ and ║

        auto row = [INNER](const std::string& text) { return format("║{:<{}}║\n", text, INNER); };
        auto center = [INNER](const std::string& text) { return format("{:^{}}", text, INNER); };
        auto repeat = [](const string& str, int n) {
            string rep;
            rep.reserve(str.size() * n);
            while (n--) {
                rep += str;
            }
            return rep;
        };

        cout << "╔" << repeat("═", INNER) << "╗" << "\n"
             << row(center("Queuing System Simulation")) << "╠" << repeat("═", INNER) << "╣" << "\n"
             << row(format(" Tick: {: 22} | Queue Size: {: 22}", clock, sim.getQueueSize()))
             << row(format(" Customers Arrived: {: 46}", sim.getCustomersArrived()))
             << row(format(" Customers Served:  {: 46}", sim.getCustomersServed()))
             << row(format(" Peak Queue Length: {: 46}", sim.getPeakQueueLength()))
             << row(format(" Avg Waiting Time:  {: 40} ticks", sim.getAverageWaitingTime())) << "╚"
             << repeat("═", INNER) << "╝" << "\n";
    }

    void logTick(int clock, const WebSimulation& sim) {
        if (log_file_.is_open()) {
            log_file_ << "[Tick " << clock << "] "
                      << "Queue: " << sim.getQueueSize() << "  "
                      << "Served: " << sim.getCustomersServed() << "  "
                      << "Arrived: " << sim.getCustomersArrived() << "  "
                      << "AvgWait: " << sim.getAverageWaitingTime() << "  "
                      << "Peak: " << sim.getPeakQueueLength() << "\n";
        }
    }

   public:
    int run(int argc,
            const char* argv[]) {  // NOLINT

        if (argc == 1) {
            readCLIParameters();
        } else if (argc == 8) {
            servers_    = ParsePositiveInt(argv[1], "Number of servers");
            trans_min_  = ParsePositiveInt(argv[2], "Min service time");
            trans_max_  = ParsePositiveInt(argv[3], "Max service time");
            arr_min_    = ParsePositiveInt(argv[4], "Min arrival interval");
            arr_max_    = ParsePositiveInt(argv[5], "Max arrival interval");
            total_cust_ = ParsePositiveInt(argv[6], "Total customers");
            delay_ms_   = ParsePositiveInt(argv[7], "Tick delay");

            if (trans_min_ > trans_max_) {
                std::swap(trans_min_, trans_max_);
            }

            if (arr_min_ > arr_max_) {
                std::swap(arr_min_, arr_max_);
            }
        } else {
            printCLIUsage();
            return 1;
        }

        log_file_.open("build/simulation.log", std::ios::out | std::ios::trunc);
        if (!log_file_.is_open()) {
            cerr << "Warning: Could not open 'build/simulation.log' for writing.\n";
        } else {
            log_file_ << "--- Simulation Started ---\n";
            log_file_ << "Servers: " << servers_ << ", Total Cust: " << total_cust_
                      << ", Trans: " << trans_min_ << "-" << trans_max_ << ", Arrival: " << arr_min_
                      << "-" << arr_max_ << "\n\n";
        }

        WebSimulation sim(sim_cap_, servers_, trans_min_, trans_max_, arr_min_, arr_max_,
                          total_cust_);
        CursorHide hide_cursor;

        cout << "\nStarting simulation: " << servers_ << " servers, " << total_cust_
             << " customers, arrival every " << arr_min_ << "-" << arr_max_ << " ticks, service "
             << trans_min_ << "-" << trans_max_ << " ticks.\n\n";

        sim.start();
        int clock = 0;
        printDashboard(clock, sim);

        while (!sim.isFinished()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms_));

            sim.tick();
            clock++;

            logTick(clock, sim);

            // Move cursor up 9 lines to overwrite the dashboard
            cout << "\033[9A";
            printDashboard(clock, sim);
        }

        if (log_file_.is_open()) {
            log_file_ << "\n--- Simulation Finished ---\n";
            log_file_.close();
        }

        cout << "\n[+] Simulation complete. Details saved to build/simulation.log\n";
        return 0;
    }
};

int main(int argc, const char* argv[]) {
    try {
        CLIApplication app;
        return app.run(argc, argv);
    } catch (const std::exception& err) {
        cerr << "FATAL: " << err.what() << "\n";
        return 1;
    }
}
