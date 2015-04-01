#include "stdafx.h"
#include "resource-manager.h"
#include "helper.h"
#include <string>
#include <map>

typedef struct {
	resource_request_t* request;
	FILE* fp;
	size_t size;
	luanovel_pointer_t start;
	luanovel_pointer_t offset;
} _resource_closure_t;
typedef _resource_closure_t* resource_closure_t;

std::map<std::string, FILE*> resourcefile;

int resource_open(const char* locale)
{
	std::string locale_str(locale);
	if (resourcefile.find(locale_str) != resourcefile.end()) return 0;

	FILE* fp = fopen(locale, "rb");
	if (fp == NULL) return 1;
	// TODO: check integrity
	resourcefile[locale_str] = fp;
	return 0;
}

void* resource_make_closure(resource_request_t* request)
{
	resource_closure_t ret;
	FILE* fp;
	int n_read = 0;
	luanovel_pointer_t fileheader;

	ret = (resource_closure_t)malloc(sizeof(_resource_closure_t));
	if (ret == NULL) return NULL;
	memset(ret, 0, sizeof(_resource_closure_t));
	ret->request = request;
	
	fp = resourcefile[request->locale];
	ret->fp = fp;
	fseek(fp, request->pointer, SEEK_SET);
	n_read = fread(&fileheader, 4, 1, fp);
	if (n_read < 1) {
		free(ret);
		return NULL;
	}

	fseek(fp, fileheader, SEEK_SET);
	ret->size = helper_read_varint(fp);
	ret->start = ftell(fp);
	
	return ret;
}

void resource_closure_destroy(void* closure)
{
	free(closure);
}

cairo_status_t resource_cairo_read(void* closure, unsigned char* data, unsigned int len)
{
	resource_closure_t cls = (resource_closure_t)closure;
	FILE* fp = cls->fp;
	size_t bytes_left = cls->size - cls->offset;
	unsigned int to_read = (len > bytes_left) ? bytes_left : len;
	unsigned int cnt = 0;

	fseek(fp, cls->start + cls->offset, SEEK_SET);
	while (to_read > 0) {
		size_t n_read = fread(data + cnt, 1, to_read, fp);
		if (n_read == 0) break;
		cnt += n_read;
		cls->offset += n_read;
		to_read -= n_read;
	}
	if (to_read > 0) return CAIRO_STATUS_READ_ERROR;
	return CAIRO_STATUS_SUCCESS;
}
