#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ed25519/ed25519.h"

#define BUFFER_SIZE 1024 * 1024

int luanovel_sign(int argc, char* argv[])
{
	const char* pubkeyfile;
	const char* privkeyfile;
	const char* datafile;
	const char* signfile;
	FILE *fpub = NULL, *fpriv = NULL, *fdata = NULL, *fsign = NULL;

	unsigned char pubkey[32];
	unsigned char privkey[64];
	unsigned char* data = NULL;
	unsigned char signature[64];
	unsigned int len = 0;

	int retval = -1;

	if (argc < 6) {
		fprintf(stderr, "Not enough arguments\n");
		fprintf(stderr, "%s %s <pubkey> <privkey> <file> <signature>\n", argv[0], argv[1]);
		goto cleanup;
	}

	pubkeyfile = argv[2];
	privkeyfile = argv[3];
	datafile = argv[4];
	signfile = argv[5];

	fpub = fopen(pubkeyfile, "rb");
	if (!fpub) {
		fprintf(stderr, "Error opening file for reading: %s\n", pubkeyfile);
		goto cleanup;
	}
	fpriv = fopen(privkeyfile, "rb");
	if (!fpriv) {
		fprintf(stderr, "Error opening file for reading: %s\n", privkeyfile);
		goto cleanup;
	}
	fdata = fopen(datafile, "rb");
	if (!fdata) {
		fprintf(stderr, "Error opening file for reading: %s\n", datafile);
		goto cleanup;
	}
	if (!strcmp(signfile, "-")) {
		fsign = freopen(signfile, "wb", stdout);
		if (!fsign) {
			fprintf(stderr, "Error opening file for writing: %s\n", signfile);
			goto cleanup;
		}
	}
	if (fread(pubkey, sizeof(pubkey), 1, fpub) != 1) {
		fprintf(stderr, "Error reading public key\n");
		goto cleanup;
	}
	if (fread(privkey, sizeof(privkey), 1, fpriv) != 1) {
		fprintf(stderr, "Error reading private key\n");
		goto cleanup;
	}

	fseek(fdata, 0, SEEK_END);
	len = (unsigned int)ftell(fdata);
	data = (unsigned char*)malloc(BUFFER_SIZE);
	fseek(fdata, 0, SEEK_SET);

	while (len > 0) {
		unsigned int to_read = (len > BUFFER_SIZE) ? BUFFER_SIZE : len;
		unsigned int have_read = 0;
		while (1) {
			int n_read;
			n_read = fread(data + have_read, 1, to_read, fdata);
			if (n_read == 0) break;
			to_read -= n_read;
			have_read += n_read;
		}
		len -= have_read;
		ed25519_sign(signature, data, have_read, pubkey, privkey);
		if (fwrite(signature, sizeof(signature), 1, stdout) != 1) {
			fprintf(stderr, "Error writing signature\n");
			goto cleanup;
		}
	}
	retval = 0;
cleanup:
	if (data) free(data);
	if (fpub) fclose(fpub);
	if (fpriv) fclose(fpriv);
	if (fdata) fclose(fdata);
	if (fsign) fclose(fsign);
	return retval;
}
