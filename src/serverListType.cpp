#include "serverListType.h"

int serverListType::getFreeServerID() const {
    for (int i = 0; i < numServers; i++) {
        if (servers[i].isFree()) {
            return i;
        }
    }
    return -1;
}

int serverListType::getNumberOfBusyServers() const {
    int busyServers = 0;
    for (int i = 0; i < numServers; i++) {
        if (!servers[i].isFree()) {
            busyServers++;
        }
    }
    return busyServers;
}

void serverListType::setServerBusy(int serverID, customerType currentCustomer, int transactionTime) {
    servers[serverID].setBusy();
    servers[serverID].setCurrentCustomer(currentCustomer);
    servers[serverID].setTransactionTime(transactionTime);
}

void serverListType::setServerBusy(int serverID, customerType currentCustomer) {
    servers[serverID].setBusy();
    servers[serverID].setCurrentCustomer(currentCustomer);
    servers[serverID].setTransactionTime();
}

void serverListType::updateServers(std::ostream& outF) {
    for (int i = 0; i < numServers; i++) {
        if (!servers[i].isFree()) {
            servers[i].decreaseTransactionTime();
            if (servers[i].getRemainingTransactionTime() == 0) {
                outF << "Server " << i << " completed Customer "
                     << servers[i].getCurrentCustomerNumber() << "\n";
                servers[i].setFree();
            }
        }
    }
}

serverListType::serverListType(int num) {
    numServers = num;
    servers = new serverType[numServers];
}

serverListType::~serverListType() {
    delete[] servers;
}