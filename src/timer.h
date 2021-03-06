//
// Created by aedoran on 5/31/18.
//

#ifndef MC_MP2_DIRECT_TIMER_H
#define MC_MP2_DIRECT_TIMER_H

#include <chrono>
#include "qc_mpi.h"

class Timer {
 public:
  Timer() {
    MPI_info::comm_rank(&master);
  }
  void Start() {
    start = std::chrono::system_clock::now();
  }
  void Stop() {
    stop = std::chrono::system_clock::now();
    span = std::chrono::duration_cast<std::chrono::duration<double>>(stop - start);
  }
  std::chrono::system_clock::time_point StartTime() {
    return start;
  }
  std::chrono::system_clock::time_point EndTime() {
    return stop;
  }
  double Span() {
    return span.count();
  }
  friend std::ostream& operator<< (std::ostream& os, Timer& timer) {
    timer.Stop();
    if (0 == timer.master) {
      os << std::fixed << std::showpos << std::setprecision(7) << timer.span.count();
    }
    return os;
  }
 private:
  int master;
  std::chrono::system_clock::time_point start, stop;
  std::chrono::duration<double> span;
};

#endif //MC_MP2_DIRECT_TIMER_H
