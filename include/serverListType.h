#ifndef SERVERLISTTYPE_H
#define SERVERLISTTYPE_H

#include <iostream>
#include "serverType.h"

using namespace std;

class serverListType {
private:
    int numOfServers;
    serverType *servers; // Pointer to create a dynamic array of servers

public:
    // Functions
    int getFreeServerID() const;
    int getNumberOfBusyServers() const;
    void setServerBusy(int serverID, customerType cCustomer, int tTime);
    void setServerBusy(int serverID, customerType cCustomer);
    void updateServers(ostream& outF);

    // Constructor & Destructor
    serverListType(int num = 1);
    ~serverListType();
};

#endif