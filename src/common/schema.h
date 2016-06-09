#pragma once

#include "libpm.h"
#include "field.h"

#include <iostream>
#include <vector>
#include <iomanip>

namespace storage {

class schema {
 public:
  schema(std::vector<field_info> _columns) {
      PM_EQU((columns), (NULL));
      PM_EQU((ser_len), (0));
      PM_EQU((deser_len), (0));

    num_columns = _columns.size();
    columns = (field_info*) pmalloc(num_columns*(sizeof(field_info)));//new field_info[num_columns];
    unsigned int itr;

    for (itr = 0; itr < num_columns; itr++) {
      PM_EQU((columns[itr]), (_columns[itr]));
      PM_EQU((ser_len), (ser_len + columns[itr].ser_len));
      PM_EQU((deser_len), (deser_len + columns[itr].deser_len));
    }

    pmemalloc_activate(columns);
  }

  ~schema() {
    delete[] columns;
  }

  void display() {
    unsigned int itr;

    for (itr = 0; itr < num_columns; itr++) {
      std::cerr << std::setw(20);
      std::cerr << "offset    : " << columns[itr].offset << " ";
      std::cerr << "ser_len   : " << columns[itr].ser_len << " ";
      std::cerr << "deser_len : " << columns[itr].deser_len << " ";
      std::cerr << "type      : " << (int) columns[itr].type << " ";
      std::cerr << "inlined   : " << (int) columns[itr].inlined << " ";
      std::cerr << "enabled   : " << (int) columns[itr].enabled << " ";
      std::cerr << "\n";
    }

    std::cerr << "\n";
  }

  field_info* columns;
  size_t ser_len;
  size_t deser_len;
  unsigned int num_columns;
};

}

