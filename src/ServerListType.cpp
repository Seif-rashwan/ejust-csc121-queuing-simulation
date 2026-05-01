#include "ServerListType.h"
#include <iostream>

using namespace std;

ServerListType::ServerListType(int num) {
    if (num <= 0) {
        numOfServers = 1;  // Default to at least 1 server
    } else {
        numOfServers = num;
    }
    servers = new ServerType[numOfServers];
}

ServerListType::~ServerListType() {
    delete[] servers;
}

int ServerListType::getFreeServerID() const {
    for (int i = 0; i < numOfServers; i++) {
        if (servers[i].isFree()) {
            return i;
        }
    }
    return -1;
}

int ServerListType::getNumberOfBusyServers() const {
    int busyServers = 0;
    for (int i = 0; i < numOfServers; i++) {
        if (!servers[i].isFree()) {
            busyServers++;
        }
    }
    return busyServers;
}

void ServerListType::setServerBusy(int serverID, const CustomerType& cCustomer) {
    if (serverID >= 0 && serverID < numOfServers) {
        servers[serverID].setBusy();
        servers[serverID].setCurrentCustomer(cCustomer);
        servers[serverID].setTransactionTime();
    }
}

void ServerListType::updateServers() {
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