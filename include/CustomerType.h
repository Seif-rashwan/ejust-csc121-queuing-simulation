#ifndef INCLUDE_CUSTOMERTYPE_H_
#define INCLUDE_CUSTOMERTYPE_H_

// Copyright 2026 E-JUST CSC 121 Project

class CustomerType {
public:
    // Functions
    void setCustomerInfo(int customer_number = 0, int arrival_time = 0, 
                      int waiting_time = 0, int transaction_time = 0);
    int getWaitingTime() const;
    void setWaitingTime(int time);
    void incrementWaitingTime();
    int getArrivalTime() const;
    int getTransactionTime() const;
    int getCustomerNumber() const;

    // Constructor
    explicit CustomerType(int customer_number = 0, int arrival_time = 0, 
                        int waiting_time = 0, int transaction_time = 0);

private:
    int customer_number;
    int arrival_time;
    int waiting_time;
    int transaction_time;
};

#endif  // INCLUDE_CUSTOMERTYPE_H_
