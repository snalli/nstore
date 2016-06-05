#pragma once

#include <string>
#include <cstdio>
#include <cstdlib>
#include <sys/time.h>

namespace storage {

class timer {
 public:

  timer() {
    total = timeval();
  }

  double duration() {
    double duration;

    duration = (total.tv_sec) * 1000.0;      // sec to ms
    duration += (total.tv_usec) / 1000.0;      // us to ms

    return duration;
  }

  void start() {
    gettimeofday(&t1, NULL);
  }

  void end() {
    gettimeofday(&t2, NULL);
    timersub(&t2, &t1, &diff); // PM_WRITE ?? diff is the target
    timeradd(&diff, &total, &total); // PM_WRITE ?? total is the target
  }

  void reset(){
    total = timeval();
  }

  timeval t1, t2, diff;
  timeval total;
};

}
