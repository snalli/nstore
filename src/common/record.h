#pragma once

#include <iostream>
#include <string>
#include <cassert>
#include <climits>
#include <thread>

#include "schema.h"
#include "field.h"

namespace storage {

class record {
 public:

  record(schema* _sptr, int is_persistent = 0) {
      this->is_persistent = is_persistent;
      PM_EQU((sptr), (_sptr));
      PM_EQU((data_len), (_sptr->ser_len));
	if(this->is_persistent)
		PM_EQU((data), ((char*) pmalloc(data_len*sizeof(char))));//new char[data_len];
	else
		data = new char[data_len];
  }

  ~record() {
    delete[] data;
  }

  // Free non-inlined data
  void clear_data() {
    unsigned int field_itr;
    for (field_itr = 0; field_itr < sptr->num_columns; field_itr++) {
      if (sptr->columns[field_itr].inlined == 0) {
        char* ptr = (char*) get_pointer(field_itr);
        delete ptr;
      }
    }
  }

  void display() {
    std::string data;

    unsigned int field_itr;
    for (field_itr = 0; field_itr < sptr->num_columns; field_itr++)
      data += get_data(field_itr) + " ";

    printf("record : %p %s \n", this, data.c_str());

  }

  std::string get_data(const int field_id) {
    std::string field;
    field_info finfo = sptr->columns[field_id];
    char type = finfo.type;
    size_t offset = finfo.offset;

    switch (type) {
      case field_type::INTEGER:
        int ival;
        memcpy(&ival, &(data[offset]), sizeof(int));
        field = std::to_string(ival);
        break;

      case field_type::DOUBLE:
        double dval;
        memcpy(&dval, &(data[offset]), sizeof(double));
        field = std::to_string(dval);
        break;

      case field_type::VARCHAR: {
        char* vcval = NULL;
        memcpy(&vcval, &(data[offset]), sizeof(void*));
        if (vcval != NULL) {
          field = std::string(vcval);
        }
      }
        break;

      default:
        std::cerr << "Invalid type : " << type << std::endl;
        exit(EXIT_FAILURE);
        break;
    }

    return field;
  }

  void* get_pointer(const int field_id) {
    void* vcval = NULL;
    memcpy(&vcval, &(data[sptr->columns[field_id].offset]), sizeof(void*));
    return vcval;
  }

  void set_data(const int field_id, record* rec_ptr) {
    char type = sptr->columns[field_id].type;
    size_t offset = sptr->columns[field_id].offset;
    size_t len = sptr->columns[field_id].ser_len;

    switch (type) {
      case field_type::INTEGER:
      case field_type::DOUBLE:
      case field_type::VARCHAR:
	// data copy and not pointer copy. Going by the variable len field.
	PM_MEMCPY((&(data[offset])), (&(rec_ptr->data[offset])), (len));
        break;

      default:
        std::cerr << "Invalid type : " << type << std::endl;
        break;
    }
  }

  void set_int(const int field_id, int ival) {
    //assert(sptr->columns[field_id].type == field_type::INTEGER);
    PM_MEMCPY((&(data[sptr->columns[field_id].offset])), (&ival), (sizeof(int)));
  }

  void set_double(const int field_id, double dval) {
    //assert(sptr->columns[field_id].type == field_type::DOUBLE);
    PM_MEMCPY((&(data[sptr->columns[field_id].offset])), (&dval), (sizeof(double)));
  }

  void set_varchar(const int field_id, std::string vc_str) {
    //assert(sptr->columns[field_id].type == field_type::VARCHAR);
	char *vc = NULL;
	if (is_persistent)
		vc = (char*) pmalloc((vc_str.size()+1)*sizeof(char));
	else
		vc = new char[vc_str.size() + 1];

    PM_STRCPY((vc), (vc_str.c_str()));
    // Pointer assignment. Why do you have to be all fancy ?
    // Here you are addressing the destination via its virtual address,
    // but you could also address it using its name. It would have the same effect.
    PM_MEMCPY((&(data[sptr->columns[field_id].offset])), (&vc), (sizeof(void*)));
  }

  void set_pointer(const int field_id, void* pval) {
    //assert(sptr->columns[field_id].type == field_type::VARCHAR);
    PM_MEMCPY((&(data[sptr->columns[field_id].offset])), (&pval), (sizeof(void*)));
  }

	// Only called on the DB load path and we need pmalloc on it
	// Never called on DB update path and we don't need pmalloc there.
  void persist_data() {
    pmemalloc_activate(data);

    unsigned int field_itr;
    for (field_itr = 0; field_itr < sptr->num_columns; field_itr++) {
      if (sptr->columns[field_itr].inlined == 0) {
        void* ptr = get_pointer(field_itr);
        //printf("persist data :: %p \n", ptr);
        pmemalloc_activate(ptr);
      }
    }
  }

  schema* sptr;
  char* data;
  size_t data_len;
  int is_persistent = 0;
};

}

