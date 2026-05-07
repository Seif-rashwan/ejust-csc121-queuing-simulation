#include <ctime>
#include <iostream>
#include "ServerListType.h"
#include "WaitingCustomerQueue.h"

using std::cin;
using std::cout;
using std::endl;

// دالة لأخذ معطيات المحاكاة من المستخدم
void SetSimulationParameters(int& sTime, int& numOfServers, int& transTime, int& tBetweenArrivals) {
    cout << "Enter simulation time: ";
    cin >> sTime;
    cout << "Enter number of servers: ";
    cin >> numOfServers;
    cout << "Enter transaction time: ";
    cin >> transTime;
    cout << "Enter time between customer arrivals: ";
    cin >> tBetweenArrivals;
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
    int total_wait_time   = 0;
    int customers_served  = 0;
    int customers_arrived = 0;

    // 3. بداية المحاكاة (اللوب الأساسية)
    for (int clock = 1; clock <= simulation_time; clock++) {
        // أ. تحديث حالة السيرفرات (تقليل وقت الناس اللي بتخدم حالياً)
        servers.updateServers();

        // ب. تحديث وقت الانتظار للناس اللي واقفة في الطابور
        if (!customer_queue.isEmpty()) {
            customer_queue.incrementWaitingTimes();
        }

        if (clock % time_between_arrivals == 0 && !customer_queue.isFull()) {
            customers_arrived++;
            CustomerType new_customer(customers_arrived, clock, 0, transaction_time);
            customer_queue.enqueue(new_customer);
            cout << "--> Customer " << customers_arrived << " arrived at tick " << clock << endl;
        }

        if (customer_queue.size() > peak_queue_length) {
            peak_queue_length = customer_queue.size();
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
    cout << "\n===================================" << endl;
    cout << "        SIMULATION RESULTS         " << endl;
    cout << "===================================" << endl;
    cout << "Total simulation time: " << simulation_time << endl;
    cout << "Total customers arrived: " << customers_arrived << endl;
    cout << "Total customers served: " << customers_served << endl;
    cout << "Customers left in queue: " << customers_arrived - customers_served << endl;
    cout << "Peak queue length: " << peak_queue_length << endl;

    if (customers_served > 0) {
        cout << "Average waiting time: " << static_cast<double>(total_wait_time) / customers_served
             << endl;
    }

    return 0;
}
