/**
 * @file CliMain.cpp
 * @brief CLI entry point for the queuing simulation with an animated dashboard.
 *
 * Usage:
 * Without arguments, the simulation prompts for parameters interactively.
 * $ simulation_cli
 * $ simulation_cli <servers> <transMin> <transMax> <arrivalMin> <arrivalMax>
 *                  <totalCustomers> <delayMs> <maxQueueSize>
 */

#include <algorithm>
#include <chrono>
#include <exception>
#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "SimulationEngine.h"

#ifdef _WIN32
/**
 * @class WindowsConsoleSetup
 * @brief Automatically configures the console to UTF-8 code page on construction
 *        and restores the original code page on destruction.
 */
class WindowsConsoleSetup {
   private:
    UINT old_output_cp_;
    UINT old_input_cp_;

   public:
    WindowsConsoleSetup() : old_output_cp_(GetConsoleOutputCP()), old_input_cp_(GetConsoleCP()) {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    }

    ~WindowsConsoleSetup() {
        SetConsoleOutputCP(old_output_cp_);
        SetConsoleCP(old_input_cp_);
    }

    WindowsConsoleSetup(const WindowsConsoleSetup&)            = delete;
    WindowsConsoleSetup& operator=(const WindowsConsoleSetup&) = delete;
};
#endif

using std::cerr;
using std::cin;
using std::cout;
using std::format;
using std::string;

/**
 * @brief Parses a positive integer command-line argument.
 *
 * @param value Raw argument text.
 * @param name Human-readable parameter name for error messages.
 * @return Parsed positive integer.
 * @throws std::invalid_argument if the parsed value is not positive.
 * @throws std::exception if conversion fails.
 */
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

/**
 * @class CLIApplication
 * @brief Owns CLI parameter parsing, dashboard rendering,
 *        logging, and simulation execution.
 */
class CLIApplication {
   private:
    int sim_cap_    = SimulationEngine::SIMULATION_TIME_CAP_DEFAULT;
    int servers_    = 4;
    int trans_min_  = SimulationEngine::DEFAULT_TRANS_MIN;
    int trans_max_  = SimulationEngine::DEFAULT_TRANS_MAX;
    int arr_min_    = SimulationEngine::DEFAULT_ARRIVAL_MIN;
    int arr_max_    = SimulationEngine::DEFAULT_ARRIVAL_MAX;
    int total_cust_ = SimulationEngine::DEFAULT_TOT_CUSTOMERS;
    int delay_ms_   = 50;
    int max_queue_  = SimulationEngine::DEFAULT_MAX_QUEUE;
    std::ofstream log_file_;

    /**
     * @brief Reads simulation parameters interactively from stdin.
     * @throws std::invalid_argument if any value is not positive.
     */
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
        read_positive("Max Queue Size           : ", max_queue_);

        if (trans_min_ > trans_max_) {
            std::swap(trans_min_, trans_max_);
        }

        if (arr_min_ > arr_max_) {
            std::swap(arr_min_, arr_max_);
        }
    }

    /**
     * @brief Prints positional command-line usage to stderr.
     */
    static void printCLIUsage() {
        cerr << "Usage: ./simulation_cli [servers transMin transMax arrivalMin arrivalMax "
             << "totalCustomers delayMs maxQueueSize]\n";
    }

    /**
     * @brief Renders one dashboard frame from the current simulation state.
     * @param clock Current display tick.
     * @param engine Simulation engine providing metrics.
     */
    static void printDashboard(int clock, const SimulationEngine& engine) {
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
             << row(format(" Tick: {: 22} | Queue Size: {: 22}", clock, engine.getQueueSize()))
             << row(format(" Customers Arrived:         {: 38}", engine.getCustomersArrived()))
             << row(format(" Customers Served:          {: 38}", engine.getCustomersServed()))
             << row(format(" Customers Turned Away:     {: 38}", engine.getCustomersTurnedAway()))
             << row(format(" Peak Queue Length:         {: 38}", engine.getPeakQueueLength()))
             << row(format(" Avg Waiting Time:          {: 32} ticks",
                           engine.getAverageWaitingTime()))
             << row(format(" Total Wait Time:           {: 32} ticks", engine.getTotalWaitTime()))
             << "╚" << repeat("═", INNER) << "╝" << "\n";
    }

    /**
     * @brief Writes one simulation tick summary to the log file when open.
     * @param
     * clock Current display tick.
     * @param engine Simulation engine providing metrics.
     */
    void logTick(int clock, const SimulationEngine& engine) {
        if (log_file_.is_open()) {
            log_file_ << "[Tick " << clock << "] "  // NOLINT
                      << "Queue: " << engine.getQueueSize() << "  "
                      << "Served: " << engine.getCustomersServed() << "  "
                      << "Arrived: " << engine.getCustomersArrived() << "  "
                      << "Avg. Wait: " << engine.getAverageWaitingTime() << "  "
                      << "Peak: " << engine.getPeakQueueLength() << "\n";
        }
    }

   public:
    /**
     * @brief Runs the CLI simulation from interactive input or command-line arguments.
     * @param argc Argument count from main.
     * @param argv Argument vector from main.
     * @return Process exit code.
     */
    int run(int argc, const char* argv[]) {  // NOLINT
        if (argc == 1) {
            readCLIParameters();
        } else if (argc >= 8) {
            servers_    = ParsePositiveInt(argv[1], "Number of servers");
            trans_min_  = ParsePositiveInt(argv[2], "Min service time");
            trans_max_  = ParsePositiveInt(argv[3], "Max service time");
            arr_min_    = ParsePositiveInt(argv[4], "Min arrival interval");
            arr_max_    = ParsePositiveInt(argv[5], "Max arrival interval");
            total_cust_ = ParsePositiveInt(argv[6], "Total customers");
            delay_ms_   = ParsePositiveInt(argv[7], "Tick delay");

            if (argc >= 9) {
                max_queue_ = ParsePositiveInt(argv[8], "Max Queue Size");
            }

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
                      << "-" << arr_max_ << ", Max Queue: " << max_queue_ << "\n\n";
        }

        CursorHide hide_cursor;
        SimulationEngine engine(sim_cap_, servers_, trans_min_, trans_max_, arr_min_, arr_max_,
                                total_cust_, max_queue_);

        cout << "\nSimulation: " << servers_ << " servers | " << total_cust_ << " customers | "
             << "Service: " << trans_min_ << "-" << trans_max_ << " ticks | Arrival: " << arr_min_
             << "-" << arr_max_ << " | Queue cap: " << max_queue_ << "\n\n";

        engine.start();
        int clock = 0;
        printDashboard(clock, engine);

        while (!engine.isFinished()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms_));
            engine.tick();
            clock++;
            logTick(clock, engine);

            // Move cursor up 11 lines to overwrite the dashboard
            cout << "\033[11A";
            printDashboard(clock, engine);
        }

        if (log_file_.is_open()) {
            log_file_ << "\n--- Simulation Finished ---\n";
            log_file_.close();
        }

        cout << "\n[+] Simulation complete. Details saved to build/simulation.log\n";
        return 0;
    }
};

/**
 * @brief Starts the CLI application and reports fatal errors.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Process exit code.
 */
int main(int argc, const char* argv[]) {
#ifdef _WIN32
    WindowsConsoleSetup win_console_setup;
#endif
    try {
        CLIApplication app;
        return app.run(argc, argv);
    } catch (const std::exception& err) {
        cerr << "FATAL: " << err.what() << "\n";
        return 1;
    }
}
