/*
 * AlexButton01 Arduino
 * Version 1.01
 */

#include <AlexButton01.h>

AlexButton01::AlexButton01(unsigned int pin_button)
  : m_pin_button(pin_button)
  , m_previous_time(millis())
  , m_previous_status(false)
  , m_long_pressed(false)
  , m_press_counter(0)
{
};

/**
 * Button state:
 *  false - button is not pressed at the moment
 *  true - button is pressed at the moment
 */
boolean AlexButton01::state()
{
  return digitalRead(m_pin_button) != normal_status;
};

/**
 * Button modes:
 *  0 - nothing happens with the button
 *  1 - button pressed ones
 *  2 - the button is pressed twice
 *  3 - the button is pressed triple
 *  4 - the button is held down for a long time
 * -1 - button released
 */
int AlexButton01::check()
{
  int result = 0;

  m_status = state(); // Current  button state
  m_current_time = millis();
  unsigned long diff_time = m_current_time - m_previous_time;
  
  if (m_previous_status != m_status && (diff_time > bounce_contacts_delay)) { // Button state is changed -> crunching bounce (do nothing for a few milliseconds)
    if (m_status) {
      if (m_press_counter == 0 || diff_time < double_press_time) {
        if (multipress_limit > 1) {
          m_press_counter++;
        } else {
          m_press_counter = 1;
        }
      }
      result = m_press_counter > multipress_limit ? 0 : m_press_counter;
    } else {
      m_long_pressed = false;
      result = -1;
    }
    // Save new time and status
    m_previous_time   = m_current_time;
    m_previous_status = m_status;
  } else if (m_status && !m_long_pressed && (diff_time > long_press_time)) {
    m_long_pressed  = true;
    m_press_counter = 0;
    result = 4;
  } else if (!m_status && ((diff_time > double_press_time + 1) || multipress_limit <= 1)) {
    m_press_counter = 0;
  }

  return result;
};
