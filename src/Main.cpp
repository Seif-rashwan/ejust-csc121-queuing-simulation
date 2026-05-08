#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include "ServerListType.h"
#include "WaitingCustomerQueue.h"

using std::cin;
using std::cout;

// دالة لأخذ معطيات المحاكاة من المستخدم
void SetSimulationParameters(int& s_time, int& num_of_servers, int& trans_time,
                             int& t_between_arrivals) {
    cout << "Enter simulation time: ";
    cin >> s_time;
    cout << "Enter number of servers: ";
    cin >> num_of_servers;
    cout << "Enter transaction time: ";
    cin >> trans_time;
    cout << "Enter time between customer arrivals: ";
    cin >> t_between_arrivals;
}

int main() {  // NOLINT(bugprone-exception-escape)
    int simulation_time;
    int number_of_servers;
    int transaction_time;
    int time_between_arrivals;

    // 1. استقبال الإعدادات
    SetSimulationParameters(simulation_time, number_of_servers, transaction_time,
                            time_between_arrivals);

    // 2. تهيئة السيرفرات والطابور
    ServerListType servers(number_of_servers);
    WaitingCustomerQueue<CustomerType> customer_queue;

    // متغيرات للإحصائيات النهائية
    int total_wait_time   = 0;
    int customers_served  = 0;
    int customers_arrived = 0;

    std::random_device random_dev;
    std::mt19937 gen(random_dev());

    // 3. بداية المحاكاة (اللوب الأساسية)
    for (int clock = 1; clock <= simulation_time; clock++) {
        // أ. تحديث حالة السيرفرات (تقليل وقت الناس اللي بتخدم حالياً)
        servers.updateServers();

        // ب. تحديث وقت الانتظار للناس اللي واقفة في الطابور
        if (!customer_queue.isEmpty()) {
            customer_queue.incrementWaitingTimes();
        }

        // ج. وصول عميل جديد (باحتمالية عشوائية مبنية على الوقت بين العملاء)
        int arrival_chance = 0;
        if (time_between_arrivals > 0) {
            std::uniform_int_distribution<int> dist(0, time_between_arrivals - 1);
            arrival_chance = dist(gen);
        }
        if (arrival_chance == 0) {
            customers_arrived++;
            // إنشاء عميل جديد وإضافته للطابور
            CustomerType new_customer(customers_arrived, clock, 0, transaction_time);
            customer_queue.enqueue(new_customer);
            cout << "--> Customer " << customers_arrived << " arrived at tick " << clock << "\n";
        }

        // د. نقل العملاء من الطابور للسيرفرات الفاضية
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
    }

    // 4. طباعة التقرير النهائي (الإحصائيات)
    cout << "\n===================================" << "\n";
    cout << "        SIMULATION RESULTS         " << "\n";
    cout << "===================================" << "\n";
    cout << "Total simulation time: " << simulation_time << "\n";
    cout << "Total customers arrived: " << customers_arrived << "\n";
    cout << "Total customers served: " << customers_served << "\n";
    cout << "Customers left in queue: " << customers_arrived - customers_served << "\n";

    if (customers_served > 0) {
        cout << "Average waiting time: " << static_cast<double>(total_wait_time) / customers_served
             << "\n";
    }

    return 0;
}