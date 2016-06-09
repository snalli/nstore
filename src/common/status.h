#pragma once

#include <string>
#include <cstdio>
#include <cstdlib>

namespace storage {

class status {
 public:

  status(unsigned int _num_txns)
      : txn_counter(0),
        num_txns(_num_txns) {

    period = ((num_txns > 10) ? (num_txns / 10) : 1);

  }

  void display() {
    if (++txn_counter % period == 0) {
      double __txn_counter = txn_counter;
      double __num_txns = num_txns;
      double res = ((__txn_counter * 100) / __num_txns);
      std::cerr << "Finished :: " << res << "\r";
      // fflush(stdout);
    }

    if (txn_counter == num_txns)
      printf("\n");
  }

  unsigned int txn_counter = 0;
  unsigned int num_txns;
  unsigned int period;

};

}
