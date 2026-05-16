#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "ServerListType.h"
#include "WaitingCustomerQueue.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 6) {
        cerr << "Usage: ./simulation <simulation_time> <servers> <transaction_time> <arrival_time> "
                "<max_customers>\n";
        return 1;
    }

    int simulation_time       = stoi(argv[1]);
    int number_of_servers     = stoi(argv[2]);
    int transaction_time      = stoi(argv[3]);
    int time_between_arrivals = stoi(argv[4]);
    int max_customers         = stoi(argv[5]);

    if (simulation_time <= 0 || number_of_servers <= 0 || transaction_time <= 0 ||
        time_between_arrivals <= 0 || max_customers <= 0) {
        cerr << "All inputs must be positive integers.\n";
        return 1;
    }

    ServerListType servers(number_of_servers);
    WaitingCustomerQueue<CustomerType> customer_queue(max_customers);

    int total_wait_time       = 0;
    int customers_served      = 0;
    int customers_arrived     = 0;
    int customers_turned_away = 0;
    int peak_queue_length     = 0;

    for (int clock = 1; clock <= simulation_time; clock++) {
        servers.updateServers();

        if (clock % time_between_arrivals == 0 && customers_arrived < max_customers) {
            customers_arrived++;

            if (!customer_queue.isFull()) {
                CustomerType customer(customers_arrived, clock, 0, transaction_time);
                customer_queue.enqueue(customer);
            } else {
                customers_turned_away++;
            }
        }

        int free_server_id = servers.getFreeServerID();

        while (free_server_id != -1 && !customer_queue.isEmpty()) {
            CustomerType customer = customer_queue.front();
            customer_queue.dequeue();

            total_wait_time += customer.getWaitingTime();
            customers_served++;

            servers.setServerBusy(free_server_id, customer);
            free_server_id = servers.getFreeServerID();
        }

        peak_queue_length = max(peak_queue_length, customer_queue.size());

        if (!customer_queue.isEmpty()) {
            customer_queue.incrementWaitingTimes();
        }

        double avg_wait =
            customers_served > 0 ? static_cast<double>(total_wait_time) / customers_served : 0.0;

        cout << "STATE:{";
        cout << "\"tick\":" << clock << ",";
        cout << "\"queueSize\":" << customer_queue.size() << ",";
        cout << "\"served\":" << customers_served << ",";
        cout << "\"avgWait\":" << avg_wait << ",";
        cout << "\"arrived\":" << customers_arrived << ",";
        cout << "\"turnedAway\":" << customers_turned_away << ",";
        cout << "\"peakQueue\":" << peak_queue_length << ",";
        cout << "\"running\":true";
        cout << "}" << "\n";
    }

    return 0;
}