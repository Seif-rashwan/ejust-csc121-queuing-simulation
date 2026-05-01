#include <iostream>
#include <cstdlib>
#include <ctime>
#include "serverListType.h"
#include "waitingCustomerQueue.h"

using namespace std;

// دالة لأخذ معطيات المحاكاة من المستخدم
void setSimulationParameters(int& sTime, int& numOfServers, int& transTime, int& tBetweenArrivals) {
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
    int simulationTime;
    int numberOfServers;
    int transactionTime;
    int timeBetweenArrivals;

    // 1. استقبال الإعدادات
    setSimulationParameters(simulationTime, numberOfServers, transactionTime, timeBetweenArrivals);

    // 2. تهيئة السيرفرات والطابور
    serverListType servers(numberOfServers);
    waitingCustomerQueue<customerType> customerQueue;

    // متغيرات للإحصائيات النهائية
    int totalWaitTime = 0;
    int customersServed = 0;
    int customersArrived = 0;

    srand(time(0)); // عشان العشوائية في وصول العملاء

    // 3. بداية المحاكاة (اللوب الأساسية)
    for (int clock = 1; clock <= simulationTime; clock++) {
        
        // أ. تحديث حالة السيرفرات (تقليل وقت الناس اللي بتخدم حالياً)
        servers.updateServers();

        // ب. تحديث وقت الانتظار للناس اللي واقفة في الطابور
        if (!customerQueue.isEmptyQueue()) {
            customerQueue.updateWaitingTimes();
        }

        // ج. وصول عميل جديد (باحتمالية عشوائية مبنية على الوقت بين العملاء)
        int arrivalChance = rand() % timeBetweenArrivals;
        if (arrivalChance == 0) {
            customersArrived++;
            // إنشاء عميل جديد وإضافته للطابور
            customerType newCustomer(customersArrived, clock, 0, transactionTime);
            customerQueue.addQueue(newCustomer);
            cout << "--> Customer " << customersArrived << " arrived at tick " << clock << endl;
        }

        // د. نقل العملاء من الطابور للسيرفرات الفاضية
        int freeServerID = servers.getFreeServerID();
        while (freeServerID != -1 && !customerQueue.isEmptyQueue()) {
            // سحب أول عميل
            customerType frontCustomer = customerQueue.front();
            customerQueue.deleteQueue();

            // تجميع الإحصائيات
            totalWaitTime += frontCustomer.getWaitingTime();
            customersServed++;

            // تسليم العميل للسيرفر الفاضي
            servers.setServerBusy(freeServerID, frontCustomer);
            
            // تدوير على سيرفر فاضي تاني لو لسه في ناس في الطابور
            freeServerID = servers.getFreeServerID();
        }
    }

    // 4. طباعة التقرير النهائي (الإحصائيات)
    cout << "\n===================================" << endl;
    cout << "        SIMULATION RESULTS         " << endl;
    cout << "===================================" << endl;
    cout << "Total simulation time: " << simulationTime << endl;
    cout << "Total customers arrived: " << customersArrived << endl;
    cout << "Total customers served: " << customersServed << endl;
    cout << "Customers left in queue: " << customersArrived - customersServed << endl;
    
    if (customersServed > 0) {
        cout << "Average waiting time: " << (double)totalWaitTime / customersServed << endl;
    }

    return 0;
}