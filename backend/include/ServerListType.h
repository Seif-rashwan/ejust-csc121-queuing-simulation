// Copyright 2026 Seif Rashwan
#ifndef INCLUDE_SERVERLISTTYPE_H_
#define INCLUDE_SERVERLISTTYPE_H_

#include "ServerType.h"

class ServerListType {
   private:
    int num_of_servers_;
    ServerType* servers_;

   public:
    int getFreeServerID() const;
    int getNumberOfBusyServers() const;
    void setServerBusy(int server_id, const CustomerType& c_customer);
    void updateServers();

    explicit ServerListType(int num = 1);
    ServerListType(const ServerListType&)            = delete;
    ServerListType& operator=(const ServerListType&) = delete;

    ~ServerListType();
};

#endif  // INCLUDE_SERVERLISTTYPE_H_