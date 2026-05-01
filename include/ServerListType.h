#ifndef INCLUDE_SERVERLISTTYPE_H_
#define INCLUDE_SERVERLISTTYPE_H_

#include <iostream>
#include "ServerType.h"
#include "CustomerType.h"

// Copyright 2026 E-JUST CSC 121 Project

class ServerListType {
public:
    // Functions
    int getFreeServerID() const;
    int getNumberOfBusyServers() const;
    void setServerBusy(int server_id, CustomerType current_customer, int transaction_time);
    void setServerBusy(int server_id, CustomerType current_customer);
    void updateServers(std::ostream& out_f);

    // Constructor & Destructor
    explicit ServerListType(int num = 1);
    ~ServerListType();

private:
    int num_servers;
    ServerType* servers;  // Pointer to create a dynamic array of servers
};

#endif  // INCLUDE_SERVERLISTTYPE_H_
