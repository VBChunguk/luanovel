#include <stdio.h>
#include <stdlib.h>
#include "ed25519/ed25519.h"

#define BUFFER_SIZE 1024 * 1024

int luanovel_verify(int argc, char* argv[])
{
	const char* pubkeyfile;
	const char* datafile;
	const char* signfile;
	FILE *fpub = NULL, *fdata = NULL, *fsign = NULL;

	unsigned char pubkey[32];
	unsigned char* data = NULL;
	unsigned char signature[64];
	unsigned int len = 0;

	int retval = -1;

	if (argc < 5) {
		fprintf(stderr, "Not enough arguments\n");
		fprintf(stderr, "%s %s <pubkey> <file> <signature>\n", argv[0], argv[1]);
		goto cleanup;
	}

	pubkeyfile = argv[2];
	datafile = argv[3];
	signfile = argv[4];

	fpub = fopen(pubkeyfile, "rb");
	if (!fpub) {
		fprintf(stderr, "Error opening file for reading: %s\n", pubkeyfile);
		goto cleanup;
	}
	fdata = fopen(datafile, "rb");
	if (!fdata) {
		fprintf(stderr, "Error opening file for reading: %s\n", datafile);
		goto cleanup;
	}
	fsign = fopen(signfile, "rb");
	if (!fsign) {
		fprintf(stderr, "Error opening file for reading: %s\n", signfile);
		goto cleanup;
	}
	if (fread(pubkey, sizeof(pubkey), 1, fpub) != 1) {
		fprintf(stderr, "Error reading public key\n");
		goto cleanup;
	}

	fseek(fdata, 0, SEEK_END);
	len = (unsigned int)ftell(fdata);
	data = (unsigned char*)malloc(BUFFER_SIZE);
	fseek(fdata, 0, SEEK_SET);

	while (len > 0) {
		unsigned int to_read = (len > BUFFER_SIZE) ? BUFFER_SIZE : len;
		unsigned int have_read = 0;
		if (fread(signature, sizeof(signature), 1, fsign) != 1) {
			retval = 1;
			goto cleanup;
		}
		while (1) {
			int n_read;
			n_read = fread(data + have_read, 1, to_read, fdata);
			if (n_read == 0) break;
			to_read -= n_read;
			have_read += n_read;
		}
		len -= have_read;
		if (!ed25519_verify(signature, data, have_read, pubkey)) {
			retval = 1;
			goto cleanup;
		}
	}
	if (fread(signature, 1, sizeof(signature), fsign) != 0) retval = 1;
	else retval = 0;
cleanup:
	if (data) free(data);
	if (fpub) fclose(fpub);
	if (fdata) fclose(fdata);
	if (fsign) fclose(fsign);
	return retval;
}
