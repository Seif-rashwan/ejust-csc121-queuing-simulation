#ifndef SERVERLISTTYPE_H
#define SERVERLISTTYPE_H

#include "serverType.h"

class serverListType {
private:
    int numOfServers;
    serverType *servers; 

public:
    int getFreeServerID() const;
    int getNumberOfBusyServers() const;
    void setServerBusy(int serverID, const customerType& cCustomer);
    void updateServers();

    serverListType(int num = 1);
    ~serverListType();
};

#endif