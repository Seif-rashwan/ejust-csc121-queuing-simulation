/**
 * @file Main.cpp
 * @brief Standalone CLI entry point for the queuing simulation.
 *
 * Note: This file contains its own main() function and is built into a separate
 * executable (simulation_cli). It is NOT linked with web_simulation.cpp.
 */
#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "ServerListType.h"
#include "WaitingCustomerQueue.h"

using std::cin;
using std::cout;

// دالة لأخذ معطيات المحاكاة من المستخدم
static void SetSimulationParameters(int& s_time, int& num_of_servers, int& trans_time,
                                    int& t_between_arrivals) {
    cout << "Enter simulation time: ";
    if (!(cin >> s_time)) {
        throw std::runtime_error("Invalid input: expected an integer for simulation time.");
    }

    cout << "Enter number of servers: ";
    if (!(cin >> num_of_servers)) {
        throw std::runtime_error("Invalid input: expected an integer for number of servers.");
    }

    cout << "Enter transaction time: ";
    if (!(cin >> trans_time)) {
        throw std::runtime_error("Invalid input: expected an integer for transaction time.");
    }

    cout << "Enter time between customer arrivals: ";
    if (!(cin >> t_between_arrivals)) {
        throw std::runtime_error("Invalid input: expected an integer for time between arrivals.");
    }

    if (s_time <= 0 || num_of_servers <= 0 || trans_time <= 0 || t_between_arrivals <= 0) {
        throw std::invalid_argument("All simulation parameters must be positive integers.");
    }

    if (t_between_arrivals > s_time) {
        cout << "Warning: time between arrivals (" << t_between_arrivals
             << ") exceeds simulation time (" << s_time << "). No customers will arrive.\n";
    }
}

int main() {
    int simulation_time;
    int number_of_servers;
    int transaction_time;
    int time_between_arrivals;
    int peak_queue_length = 0;

    // 1. استقبال الإعدادات
    SetSimulationParameters(simulation_time, number_of_servers, transaction_time,
                            time_between_arrivals);

    // 2. تهيئة السيرفرات والطابور
    ServerListType servers(number_of_servers);
    WaitingCustomerQueue<CustomerType> customer_queue;

    // متغيرات للإحصائيات النهائية
    int total_wait_time       = 0;
    int customers_served      = 0;
    int customers_arrived     = 0;
    int customers_turned_away = 0;

    // 3. بداية المحاكاة (اللوب الأساسية)
    for (int clock = 1; clock <= simulation_time; clock++) {
        // أ. تحديث حالة السيرفرات (تقليل وقت الناس اللي بتخدم حالياً)
        servers.updateServers();

        // ب. وصول العملاء الجدد
        if (clock % time_between_arrivals == 0) {
            customers_arrived++;  // always count the arrival regardless of queue state
            if (!customer_queue.isFull()) {
                CustomerType new_customer(customers_arrived, clock, 0, transaction_time);
                customer_queue.enqueue(new_customer);
                cout << "--> Customer " << customers_arrived << " arrived at tick " << clock
                     << '\n';
            } else {
                customers_turned_away++;
                cout << "--> Customer " << customers_arrived << " turned away (queue full) at tick "
                     << clock << '\n';
            }
        }

        // ج. نقل العملاء من الطابور للسيرفرات الفاضية
        int free_server_id = servers.getFreeServerID();
        while (free_server_id != -1 && !customer_queue.isEmpty()) {
            // سحب أول عميل
            CustomerType front_customer = customer_queue.front();
            customer_queue.dequeue();

            // تجميع الإحصائيات
            total_wait_time += front_customer.getWaitingTime();
            customers_served++;

            // تسليم العميل للسيرفر الفاضي
            servers.setServerBusy(free_server_id, front_customer);

            // تدوير على سيرفر فاضي تاني لو لسه في ناس في الطابور
            free_server_id = servers.getFreeServerID();
        }

        // د. تسجيل الطول الأقصى للطابور
        peak_queue_length = std::max(customer_queue.size(), peak_queue_length);

        // ه. تحديث وقت الانتظار للناس اللي لسه في الطابور بعد كل محاولات التعيين
        if (!customer_queue.isEmpty()) {
            customer_queue.incrementWaitingTimes();
        }
    }

    // احتساب وقت انتظار العملاء المتبقين في الطابور في نهاية المحاكاة
    int customers_in_queue = customer_queue.size();
    while (!customer_queue.isEmpty()) {
        total_wait_time += customer_queue.front().getWaitingTime();
        customer_queue.dequeue();
    }

    // 4. طباعة التقرير النهائي (الإحصائيات)
    cout << "\n===================================\n";
    cout << "        SIMULATION RESULTS         \n";
    cout << "===================================\n";
    cout << "Total simulation time:        " << simulation_time << '\n';
    cout << "Total customers arrived:      " << customers_arrived << '\n';
    cout << "  - Served:                   " << customers_served << '\n';
    cout << "  - Left in queue:            " << customers_in_queue << '\n';
    cout << "  - Turned away (queue full): " << customers_turned_away << '\n';
    cout << "Peak queue length:            " << peak_queue_length << '\n';

    int total_customers_counted = customers_served + customers_in_queue;
    if (total_customers_counted > 0) {
        cout << "Average waiting time:         "
             << static_cast<double>(total_wait_time) / total_customers_counted << '\n';
    }

    return 0;
}
