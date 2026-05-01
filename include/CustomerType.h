#ifndef INCLUDE_CUSTOMERTYPE_H_
#define INCLUDE_CUSTOMERTYPE_H_

class CustomerType {
   private:
    int customerNumber;
    int arrivalTime;
    int waitingTime;
    int transactionTime;

   public:
    // دوال ضبط واسترجاع البيانات
    void setCustomerInfo(int customerN = 0, int arrTime = 0, int wTime = 0, int tTime = 0);
    int getWaitingTime() const;
    void setWaitingTime(int time);
    void incrementWaitingTime();
    int getArrivalTime() const;
    int getTransactionTime() const;
    int getCustomerNumber() const;

    // Constructor
    explicit CustomerType(int customerN = 0, int arrTime = 0, int wTime = 0, int tTime = 0);
};

#endif  // INCLUDE_CUSTOMERTYPE_H_