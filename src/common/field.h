#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cstring>

namespace storage {

enum field_type {
  FD_INVALID,
  INTEGER,
  DOUBLE,
  VARCHAR,
};

struct field_info {
  field_info() {
      PM_EQU(offset, 0);
      PM_EQU(ser_len,0);
      PM_EQU(deser_len, 0);
      PM_EQU(type, field_type::FD_INVALID);
      PM_EQU(inlined, 1);
      PM_EQU(enabled, 1);
  }

  field_info(off_t _offset, size_t _ser_len, size_t _deser_len, field_type _type,
              bool _inlined, bool _enabled) {
      PM_EQU(offset, _offset);
      PM_EQU(ser_len, _ser_len+1);
      PM_EQU(deser_len, _deser_len+1);
      PM_EQU(type, _type);
      PM_EQU(inlined, _inlined);
      PM_EQU(enabled, _enabled);

  }

  off_t offset;
  size_t ser_len;
  size_t deser_len;
  field_type type;
  bool inlined;
  bool enabled;
};

}

