#pragma once

#include "types.h"

typedef struct {
	luanovel_fileidx_t fileindex;
	luanovel_pointer_t eip;
} _status_eip_t;

typedef struct {
	luanovel_pointer_t stringptr;
	unsigned int pos;
	unsigned int timer;
} _status_string_t;

typedef struct {
	_status_eip_t eip;
	_status_string_t text;
	bool last_test_result;
} _status_t;
