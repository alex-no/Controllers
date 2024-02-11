/*
 * TimeRunner01 Arduino
 */

#include <TimeRunner01.h>

int TimeRunner01::s_count = 0;
TimeRunner01* TimeRunner01::s_items[s_timers_capacity] = {0};

TimeRunner01::TimeRunner01(unsigned long period, CallbackFunction func, bool enabled)
  : m_period(period)
  , m_func(func)
  , m_previous_time(0)
  , m_enabled(enabled)
  , m_skip_overtime(true)
{
  s_items[s_count] = this;
  if (s_count <= s_timers_capacity) {
    s_count++;
  }
};

void TimeRunner01::enable()
{
  m_enabled = true;
  m_previous_time = millis();
};

void TimeRunner01::disable()
{
  m_enabled = false;
  m_previous_time = 0;
};

bool TimeRunner01::isEnabled()
{
  return m_enabled;
};

void TimeRunner01::execute()
{
  if (m_enabled) {
    if(millis() - m_previous_time >= m_period) {
      m_previous_time += m_period;

      m_func();

      while (m_skip_overtime && millis() - m_previous_time >= m_period) {
        m_previous_time += m_period;
      }
    }
  }
};

void TimeRunner01::check()
{
  for (int i = 0; i < TimeRunner01::s_count; i++) {
    TimeRunner01::s_items[i]->execute();
  }
};
