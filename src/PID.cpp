#include "PID.h"

PID::~PID() {}

void PID::UpdateError(double cte) {
   d_error = cte - p_error;
   p_error = cte;
   i_error += cte;
}

double PID::PIDResult() {
  return - _Kp * p_error - _Kd * d_error - _Ki * i_error;
}

void PID::ResetPID(const std::vector<double> &p) {
  _Kp = p[0];
  _Ki = p[1];
  _Kd = p[2];

  // reset errors to zero after resetting pid parameters.
  p_error = 0.0;
  i_error = 0.0;
  d_error = 0.0;
}