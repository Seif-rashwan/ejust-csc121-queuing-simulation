#ifndef CUSTOMERTYPE_H
#define CUSTOMERTYPE_H

class customerType {
   public:
    // Functions
    void setCustomerInfo(int customerNumber = 0, int arrivalTime = 0, int waitingTime = 0,
                         int transactionTime = 0);
    int getWaitingTime() const;
    void setWaitingTime(int time);
    void incrementWaitingTime();
    int getArrivalTime() const;
    int getTransactionTime() const;
    int getCustomerNumber() const;

    // Constructor
    customerType(int customerNumber = 0, int arrivalTime = 0, int waitingTime = 0,
                 int transactionTime = 0);

   private:
    int customerNumber;
    int arrivalTime;
    int waitingTime;
    int transactionTime;
};

#endif  // CUSTOMERTYPE_H