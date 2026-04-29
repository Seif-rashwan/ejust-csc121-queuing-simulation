#ifndef CUSTOMERTYPE_H
#define CUSTOMERTYPE_H

class customerType {
   private:
    int customerNumber;
    int arrivalTime;
    int waitingTime;
    int transactionTime;

   public:
    // Functions
    void setCustomerInfo(int customerN = 0, int arrTime = 0, int wTime = 0, int tTime = 0);
    int getWaitingTime() const;
    void setWaitingTime(int time);
    void incrementWaitingTime();
    int getArrivalTime() const;
    int getTransactionTime() const;
    int getCustomerNumber() const;

    // Constructor
    customerType(int customerN = 0, int arrTime = 0, int wTime = 0, int tTime = 0);
};

#endif