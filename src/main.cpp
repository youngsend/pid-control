#include <math.h>
#include <uWS/uWS.h>
#include <iostream>
#include <string>
#include "json.hpp"
#include "PID.h"

// for convenience
using nlohmann::json;
using std::string;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }

// Checks if the SocketIO event has JSON data.
// If there is data the JSON object in string format will be returned,
// else the empty string "" will be returned.
string hasData(string s) {
  auto found_null = s.find("null");
  auto b1 = s.find_first_of("[");
  auto b2 = s.find_last_of("]");
  if (found_null != string::npos) {
    return "";
  }
  else if (b1 != string::npos && b2 != string::npos) {
    return s.substr(b1, b2 - b1 + 1);
  }
  return "";
}

int main() {
  uWS::Hub h;

  // initialize pid controller.
//  std::vector<double> p = {0.1, 0.0003, 0.9};
//  std::vector<double> p = {0.206356, 0.00136356, 1.96356};
  std::vector<double> p = {0.19, 0.0003, 1.96};
  PID pid(p[0], p[1], p[2]);
  std::cout << "pid: " << p[0] << ", " << p[1] << ", " << p[2] << "\n";

  // parameters for twiddle
  bool tune_pid = false; // if tune pid parameters or not.
  int count = 0;
  double total_cte = 0.0;
  bool best_error_initialized = false;
  double best_error = std::numeric_limits<double>::max();
  std::vector<double> dp = {0.01, 0.0001, 0.1};
  bool move_plus = true;
  std::cout << "dp: " << dp[0] << ", " << dp[1] << ", " << dp[2] << "\n";

  h.onMessage([&pid, &p, &dp, &tune_pid, &count,
                  &total_cte, &best_error_initialized,
                  &best_error, &move_plus](
      uWS::WebSocket<uWS::SERVER> ws,
      char *data, size_t length,
      uWS::OpCode opCode) {
    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
    if (length && length > 2 && data[0] == '4' && data[1] == '2') {
      auto s = hasData(string(data).substr(0, length));

      if (s != "") {
        auto j = json::parse(s);

        string event = j[0].get<string>();

        if (event == "telemetry") {
          // j[1] is the data JSON object
          double cte = std::stod(j[1]["cte"].get<string>());
          double speed = std::stod(j[1]["speed"].get<string>());
          double angle = std::stod(j[1]["steering_angle"].get<string>());
          /**
           * Calculate steering value here, remember the steering value is
           *   [-1, 1].
           * NOTE: Feel free to play around with the throttle and speed.
           *   Maybe use another PID controller to control the speed!
           */

          pid.UpdateError(cte);
          double steer_value = pid.PIDResult();
          if (steer_value > 1) {
            steer_value = 1;
          } else if (steer_value < -1) {
            steer_value = -1;
          }

          if (tune_pid) {
            total_cte += cte * cte;
            count += 1;
            if (count >= 1500) {
              // calculate average cte.
              double average_err = total_cte / count;
              if (!best_error_initialized) {
                best_error = average_err;
                best_error_initialized = true;
                p[0] += dp[0];
                p[1] += dp[1];
                p[2] += dp[2];
                move_plus = true; // + dp
              } else {
                if (average_err < best_error) {
                  // always make dp bigger when smaller average error got.
                  // when average error is getting smaller, no need to search -dp.
                  best_error = average_err;
                  dp[0] *= 1.1;
                  dp[1] *= 1.1;
                  dp[2] *= 1.1;
                  p[0] += dp[0];
                  p[1] += dp[1];
                  p[2] += dp[2];
                  move_plus = true;
                } else {
                  if (move_plus) {
                    // + dp was tried for this average error
                    p[0] -= 2 * dp[0];
                    p[1] -= 2 * dp[1];
                    p[2] -= 2 * dp[2];
                    move_plus = false; // - dp
                  } else {
                    // - dp was tried for this average error.
                    // and this try failed, so move back to original parameters.
                    p[0] += dp[0];
                    p[1] += dp[1];
                    p[2] += dp[2];
                    dp[0] *= 0.9;
                    dp[1] *= 0.9;
                    dp[2] *= 0.9;
                    p[0] += dp[0];
                    p[1] += dp[1];
                    p[2] += dp[2];
                    move_plus = true;
                  }
                }
              }
              pid.ResetPID(p);
              count = 0;
              std::cout << "total squared cte: " << total_cte << "\n";
              total_cte = 0.0;
              std::cout << "pid: " << p[0] << ", " << p[1] << ", " << p[2] << "\n";
              std::cout << "dp: " << dp[0] << ", " << dp[1] << ", " << dp[2] << "\n";
              if (dp[0] + dp[1] + dp[2] < 0.001) {
                std::cout << "OK to stop tuning\n";
              }
            }
          }

          json msgJson;
          msgJson["steering_angle"] = steer_value;
          msgJson["throttle"] = 0.3;
          auto msg = "42[\"steer\"," + msgJson.dump() + "]";
//          std::cout << msg << std::endl;
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
        }  // end "telemetry" if
      } else {
        // Manual driving
        string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }
    }  // end websocket message if
  }); // end h.onMessage

  h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) {
    std::cout << "Connected!!!" << std::endl;
  });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code,
                         char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port)) {
    std::cout << "Listening to port " << port << std::endl;
  } else {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }

  h.run();
}