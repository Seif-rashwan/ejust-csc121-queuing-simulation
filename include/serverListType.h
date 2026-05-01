#ifndef SERVERLISTTYPE_H
#define SERVERLISTTYPE_H

#include <iostream>
#include "serverType.h"

class serverListType {
public:
    // Functions
    int getFreeServerID() const;
    int getNumberOfBusyServers() const;
    void setServerBusy(int serverID, customerType currentCustomer, int transactionTime);
    void setServerBusy(int serverID, customerType currentCustomer);
    void updateServers(std::ostream& outF);

    // Constructor & Destructor
    serverListType(int num = 1);
    ~serverListType();

private:
    int numServers;
    serverType* servers;  // Pointer to create a dynamic array of servers
};

#endif  // SERVERLISTTYPE_H