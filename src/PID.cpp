#include "PID.h"

PID::~PID() {}

void PID::UpdateError(double cte) {
   d_error = cte - p_error;
   p_error = cte;
   i_error += cte;
}

double PID::PIDResult() {
  return - Kp * p_error - Kd * d_error - Ki * i_error;
}