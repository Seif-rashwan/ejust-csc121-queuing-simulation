// Copyright 2026 Seif Rashwan
#ifndef INCLUDE_SERVERLISTTYPE_H_
#define INCLUDE_SERVERLISTTYPE_H_

#include "ServerType.h"

class ServerListType {
   private:
    int numOfServers;
    ServerType* servers;

   public:
    int getFreeServerID() const;
    int getNumberOfBusyServers() const;
    void setServerBusy(int serverID, const CustomerType& cCustomer);
    void updateServers();

    explicit ServerListType(int num = 1);
    ~ServerListType();
};

#endif  // INCLUDE_SERVERLISTTYPE_H_
