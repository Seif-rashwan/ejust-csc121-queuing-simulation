// Copyright 2026 Seif Rashwan
#ifndef INCLUDE_SERVERLISTTYPE_H_
#define INCLUDE_SERVERLISTTYPE_H_

/**
 * @file ServerListType.h
 * @brief Defines the dynamic collection of simulation servers.
 */

#include "ServerType.h"

/**
 * @class ServerListType
 * @brief Owns and updates a fixed-size array of ServerType objects.
 */
class ServerListType {
   private:
    int num_of_servers_  = 0;
    ServerType* servers_ = nullptr;

   public:
    /**
     * @brief Finds the first free server.
     * @return Zero-based server index, or -1 when every server is busy.
     */
    int getFreeServerID() const;

    /**
     * @brief Counts currently busy servers.
     * @return Number of servers that are not free.
     */
    int getNumberOfBusyServers() const;

    /**
     * @brief Assigns a customer to a server and starts its timer.
     * @param server_id Zero-based server index.
     * @param c_customer Customer assigned to the server.
     */
    void setServerBusy(int server_id, const CustomerType& c_customer);

    /**
     * @brief Advances all busy servers by one tick and frees completed servers.
     */
    void updateServers();

    /**
     * @brief Constructs a server list with at least one server.
     * @param num Requested server count; non-positive values become one.
     */
    explicit ServerListType(int num = 1);
    ServerListType(const ServerListType&)            = delete;
    ServerListType& operator=(const ServerListType&) = delete;

    /**
     * @brief Releases the owned server array.
     */
    ~ServerListType();
};

#endif  // INCLUDE_SERVERLISTTYPE_H_
