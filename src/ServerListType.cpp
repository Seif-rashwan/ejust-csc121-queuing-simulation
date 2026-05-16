#include "ServerListType.h"

ServerListType::ServerListType(int num) {
    if (num <= 0) {
        num_of_servers_ = 1;
    } else {
        num_of_servers_ = num;
    }

    servers_ = new ServerType[num_of_servers_];
}

ServerListType::~ServerListType() {
    delete[] servers_;
}

// cppcheck-suppress unusedFunction
int ServerListType::getFreeServerID() const {
    for (int i = 0; i < num_of_servers_; i++) {
        if (servers_[i].isFree()) {
            return i;
        }
    }
    return -1;
}

// cppcheck-suppress unusedFunction
int ServerListType::getNumberOfBusyServers() const {
    int busy_servers = 0;
    for (int i = 0; i < num_of_servers_; i++) {
        if (!servers_[i].isFree()) {
            busy_servers++;
        }
    }
    return busy_servers;
}

void ServerListType::setServerBusy(int server_id, const CustomerType& c_customer) {
    if (server_id >= 0 && server_id < num_of_servers_) {
        servers_[server_id].setBusy();
        servers_[server_id].setCurrentCustomer(c_customer);
        servers_[server_id].setTransactionTime();
    }
}

void ServerListType::updateServers() {
    for (int i = 0; i < num_of_servers_; i++) {
        if (!servers_[i].isFree()) {
            servers_[i].decreaseTransactionTime();
            if (servers_[i].getRemainingTransactionTime() == 0) {
                servers_[i].setFree();
            }
        }
    }
}