#ifndef PID_H
#define PID_H

#include <vector>

class PID {
 public:
  /**
   * Constructor
   */
  PID(double Kp, double Ki, double Kd) : _Kp(Kp), _Ki(Ki), _Kd(Kd) {};

  /**
   * Destructor.
   */
  virtual ~PID();

  void ResetPID(const std::vector<double> &p);

  /**
   * Update the PID error variables given cross track error.
   * @param cte The current cross track error
   */
  void UpdateError(double cte);

  /**
   * Calculate the total PID error.
   * @output The total PID error
   */
  double PIDResult();

 private:
  /**
   * PID Errors
   */
  double p_error {0.0};
  double i_error {0.0};
  double d_error {0.0};

  /**
   * PID Coefficients
   */ 
  double _Kp;
  double _Ki;
  double _Kd;
};

#endif  // PID_H