#ifndef PID_H
#define PID_H

class PID {
 public:
  /**
   * Constructor
   */
  PID(double Kp, double Ki, double Kd) : Kp(Kp), Ki(Ki), Kd(Kd) {};

  /**
   * Destructor.
   */
  virtual ~PID();

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
  double Kp;
  double Ki;
  double Kd;
};

#endif  // PID_H