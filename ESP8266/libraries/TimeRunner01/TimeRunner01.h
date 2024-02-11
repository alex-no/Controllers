/*
 * TimeRunner01 - Allows to run some programs periodical by pointed periods
 */

#ifndef TIME_RUNNER01_H_
#define TIME_RUNNER01_H_

#include <Arduino.h>

using CallbackFunction = void (*)();
static const int s_timers_capacity = 16;

class TimeRunner01
{
  public:
    static void check(); // This method need to call in the loop method.

    TimeRunner01(unsigned long period, CallbackFunction func, bool enabled = true); // Constructor (period, Callback Function, enabled run)

    void enable();    // Enable to run program
    void disable();   // Disable to run program
    bool isEnabled(); // Show Allowed flag
    
    bool m_skip_overtime; // This flag allows to the skip ticks if runtime of Callback Function more than period

  protected:
    static int s_count;               // Count of runners 
    static TimeRunner01* s_items[s_timers_capacity]; // Runner items 

    CallbackFunction m_func; // Callback Function

    void execute(); // Check perriod and run Callback Function

    int m_period;                  // Pointed period
    unsigned long m_previous_time; // Timestamp of previous call
    bool m_enabled;                // Flag for allow running
};
#endif //TIME_RUNNER01_H_