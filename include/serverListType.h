#ifndef SERVERLISTTYPE_H
#define SERVERLISTTYPE_H

#include "serverType.h"

class ServerListType {
private:
    int numOfServers;
    ServerType *servers; 

public:
    int getFreeServerID() const;
    int getNumberOfBusyServers() const;
    void setServerBusy(int serverID, const CustomerType& cCustomer);
    void updateServers();

    ServerListType(int num = 1);
    ~ServerListType();
};

#endif