#include <algorithm>
#include <iostream>
#include <utility>

#include "WebSimulation.h"

/**
 * @brief Runs the web-queue simulation from command-line parameters.
 *
 * Accepts optional positional arguments to override defaults:
 *   argv[1] safetyTimeCap        : maximum tick budget before forced stop
 *   argv[2] numServers           : number of parallel servers/cashiers
 *   argv[3] transactionTimeMin   : minimum service time per customer (ticks)
 *   argv[4] transactionTimeMax   : maximum service time per customer (ticks)
 *   argv[5] arrivalIntervalMin   : minimum gap between customer arrivals
 *   argv[6] arrivalIntervalMax   : maximum gap between customer arrivals
 *   argv[7] totalCustomers       : total customers to generate
 *
 * Argument modes:
 * - argc >= 8: all values above are read from CLI.
 * - argc >= 7: reads argv[1..6], keeps totalCustomers default.
 * - otherwise: all values remain at built-in defaults.
 *
 * After parsing, min/max ranges are clamped by swapping if inverted.
 * The simulation prints a tick-0 snapshot, advances until completion,
 * then emits final statistics. Runtime errors are reported to stderr.
 */
int main(int argc, const char* argv[]) {
    try {
        // Defaults are used when arguments are omitted.
        int sim_cap    = WebSimulation::SIMULATION_TIME_CAP_DEFAULT;
        int servers    = 4;
        int trans_min  = WebSimulation::DEFAULT_TRANS_MIN;
        int trans_max  = WebSimulation::DEFAULT_TRANS_MAX;
        int arr_min    = WebSimulation::DEFAULT_ARRIVAL_MIN;
        int arr_max    = WebSimulation::DEFAULT_ARRIVAL_MAX;
        int total_cust = WebSimulation::DEFAULT_TOT_CUSTOMERS;

        if (argc >= 8) {
            sim_cap    = std::stoi(argv[1]);
            servers    = std::stoi(argv[2]);
            trans_min  = std::stoi(argv[3]);
            trans_max  = std::stoi(argv[4]);
            arr_min    = std::stoi(argv[5]);
            arr_max    = std::stoi(argv[6]);
            total_cust = std::stoi(argv[7]);
        } else if (argc >= 7) {
            sim_cap   = std::stoi(argv[1]);
            servers   = std::stoi(argv[2]);
            trans_min = std::stoi(argv[3]);
            trans_max = std::stoi(argv[4]);
            arr_min   = std::stoi(argv[5]);
            arr_max   = std::stoi(argv[6]);
        }

        // Range defensive clamping
        if (trans_min > trans_max) {
            std::swap(trans_min, trans_max);
        }

        if (arr_min > arr_max) {
            std::swap(arr_min, arr_max);
        }

        WebSimulation sim(sim_cap, servers, trans_min, trans_max, arr_min, arr_max, total_cust);

        // emit tick-0 snapshot before anything happens
        sim.outputState();
        sim.start();

        while (!sim.isFinished()) {
            sim.tick();
            sim.outputState();
        }

        sim.outputFinalStats();

    } catch (const std::exception& e) {
        // Errors go to stderr, never corrupt the stdout JSON stream
        std::cerr << "FATAL: " << e.what() << "\n";
        return 1;
    }

    return 0;
}