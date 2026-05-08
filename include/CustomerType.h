#ifndef INCLUDE_CUSTOMERTYPE_H_
#define INCLUDE_CUSTOMERTYPE_H_

class CustomerType {
   private:
    int customer_number_;
    int arrival_time_;
    int waiting_time_;
    int transaction_time_;

   public:
    void setCustomerInfo(int customer_n = 0, int arr_time = 0, int w_time = 0,
                         int time_spent = 0);

    int getWaitingTime() const;
    void setWaitingTime(int time);
    void incrementWaitingTime();
    int getArrivalTime() const;
    void setArrivalTime(int time);
    int getTransactionTime() const;
    int getCustomerNumber() const;

    explicit CustomerType(int customer_n = 0, int arr_time = 0, int w_time = 0, int t_time = 0);
};

#endif  // INCLUDE_CUSTOMERTYPE_H_