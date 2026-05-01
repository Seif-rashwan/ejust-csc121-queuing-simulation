#include <iostream>
#include "serverListType.h"

using namespace std;

serverListType::serverListType(int num) {
    numOfServers = num;
    servers = new serverType[numOfServers];
}

serverListType::~serverListType() {
    delete[] servers;
}

int serverListType::getFreeServerID() const {
    for (int i = 0; i < numOfServers; i++) {
        if (servers[i].isFree()) {
            return i;
        }
    }
    return -1; 
}

int serverListType::getNumberOfBusyServers() const {
    int busyServers = 0;
    for (int i = 0; i < numOfServers; i++) {
        if (!servers[i].isFree()) {
            busyServers++;
        }
    }
    return busyServers;
}

void serverListType::setServerBusy(int serverID, const customerType& cCustomer) {
    servers[serverID].setBusy();
    servers[serverID].setCurrentCustomer(cCustomer);
    servers[serverID].setTransactionTime();
}

void serverListType::updateServers() {
    for (int i = 0; i < numOfServers; i++) {
        if (!servers[i].isFree()) {
            servers[i].decreaseTransactionTime();
            if (servers[i].getRemainingTransactionTime() == 0) {
                cout << "Server No: " << (i + 1) << " is now free." << endl;
                servers[i].setFree();
            }
        }
    }
}