#include "ServerListType.h"
#include <iostream>

ServerListType::ServerListType(int num) {
    if (num <= 0) {
        numOfServers = 1;
    } else {
        numOfServers = num;
    }
    servers = new ServerType[numOfServers];
}

ServerListType::~ServerListType() {
    delete[] servers;
}

int ServerListType::getFreeServerId() const {
    for (int i = 0; i < numOfServers; i++) {
        if (servers[i].isFree()) {
            return i;
        }
    }
    return -1;
}

int ServerListType::getNumberOfBusyServers() const {
    int busy_servers = 0;
    for (int i = 0; i < numOfServers; i++) {
        if (!servers[i].isFree()) {
            busyServers++;
        }
    }
    return busyServers;
}

void ServerListType::setServerBusy(int server_id, const CustomerType& c_customer) {
    if (server_id >= 0 && server_id < numOfServers) {
        servers[server_id].setBusy();
        servers[server_id].setCurrentCustomer(c_customer);
        servers[server_id].setTransactionTimeFromCustomer();
        //
        std::cout << "  [Server " << (server_id + 1) << "] now serving Customer "
                  << c_customer.getCustomerNumber() << " (will take "
                  << c_customer.getTransactionTime() << " ticks)" << std::endl;
    }
}

void ServerListType::updateServers() {
    for (int i = 0; i < numOfServers; i++) {
        if (!servers[i].isFree()) {
            servers[i].decreaseTransactionTime();
            if (servers[i].getRemainingTransactionTime() == 0) {
                std::cout << "  [Server " << (i + 1) << "] finished with Customer "
                          << servers[i].getCurrentCustomerNumber() << " and is now FREE."
                          << std::endl;
                servers[i].setFree();
            }
        }
    }
}
