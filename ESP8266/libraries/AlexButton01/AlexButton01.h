/*
 * AlexButton01 - Read and return all differenn satatus of button (real or sensor)
 */

#ifndef ALEX_BUTTON01_H_
#define ALEX_BUTTON01_H_

#include <Arduino.h>

class AlexButton01
{
  public:
    AlexButton01(unsigned int pin_button); // Constructor (pin_button)

    boolean state(); // State of button at the moment - pressed or released
    int check(); // Check button status and reyturn its value
    
    //bool normal_status                 = LOW;  // Status of pin if button is not pressed
    bool normal_status                 = false;  // Status of pin if button is not pressed
    unsigned int bounce_contacts_delay = 5;    // Bounce of contacts delay
    unsigned int double_press_time     = 450;  // Time when button pressed double
    unsigned int long_press_time       = 700;  // Time when button pressed long
    
    unsigned int multipress_limit      = 3; // Count of maximum multipress

  protected:
    unsigned long m_previous_time; // Time when button was pressed previous time
    unsigned long m_current_time;  // Current time
    int m_pin_button;             // Counter - how many times button was pressed with shot time interwal
    int m_press_counter;             // Counter - how many times button was pressed with shot time interwal
    bool m_status;                 // Button status (pressed = true. released = false)
    bool m_previous_status;        // Botton previous status
    bool m_long_pressed;           // Botton was pressed for a long time
};
#endif //ALEX_BUTTON01_H_