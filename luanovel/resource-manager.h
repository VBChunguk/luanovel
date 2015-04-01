#pragma once

#include "types.h"

typedef struct {
	const char* locale;
	luanovel_pointer_t pointer;
} resource_request_t;

int resource_open(const char* locale);
void* resource_make_closure(resource_request_t* request);
void resource_closure_destroy(void* closure);
cairo_status_t resource_cairo_read(void* closure, unsigned char* data, unsigned int len);
