// Copyright 2026 E-JUST CSC 121 Project

#include "ServerListType.h"

int ServerListType::getFreeServerID() const {
    for (int i = 0; i < num_servers; i++) {
        if (servers[i].isFree()) {
            return i;
        }
    }
    return -1;
}

int ServerListType::getNumberOfBusyServers() const {
    int busy_servers = 0;
    for (int i = 0; i < num_servers; i++) {
        if (!servers[i].isFree()) {
            busy_servers++;
        }
    }
    return busy_servers;
}

void ServerListType::setServerBusy(int server_id, CustomerType current_customer, 
                                   int transaction_time) {
    servers[server_id].setBusy();
    servers[server_id].setCurrentCustomer(current_customer);
    servers[server_id].setTransactionTime(transaction_time);
}

void ServerListType::setServerBusy(int server_id, CustomerType current_customer) {
    servers[server_id].setBusy();
    servers[server_id].setCurrentCustomer(current_customer);
    servers[server_id].setTransactionTime();
}

void ServerListType::updateServers(std::ostream& out_f) {
    for (int i = 0; i < num_servers; i++) {
        if (!servers[i].isFree()) {
            servers[i].decreaseTransactionTime();
            if (servers[i].getRemainingTransactionTime() == 0) {
                out_f << "Server " << i << " completed Customer "
                     << servers[i].getCurrentCustomerNumber() << "\n";
                servers[i].setFree();
            }
        }
    }
}

ServerListType::ServerListType(int num) {
    num_servers = num;
    servers = new ServerType[num_servers];
}

ServerListType::~ServerListType() {
    delete[] servers;
}
