#include <stdio.h>
#include "ed25519/ed25519.h"

int luanovel_keygen(int argc, char* argv[])
{
	const char* pubkeyfile;
	const char* privkeyfile;
	FILE *fpub = NULL, *fpriv = NULL;

	unsigned char seed[32];
	unsigned char pubkey[32];
	unsigned char privkey[64];

	int retval = -1;

	if (argc < 4) {
		fprintf(stderr, "Not enough arguments\n");
		fprintf(stderr, "%s %s <pubkey> <privkey>\n", argv[0], argv[1]);
		goto cleanup;
	}

	pubkeyfile = argv[2];
	privkeyfile = argv[3];

	if (ed25519_create_seed(seed)) {
		fprintf(stderr, "Error generating seed\n");
		goto cleanup;
	}
	ed25519_create_keypair(pubkey, privkey, seed);

	fpub = fopen(pubkeyfile, "wb");
	if (!fpub) {
		fprintf(stderr, "Error opening file for writing: %s\n", pubkeyfile);
		goto cleanup;
	}
	fpriv = fopen(privkeyfile, "wb");
	if (!fpriv) {
		fprintf(stderr, "Error opening file for writing: %s\n", privkeyfile);
		goto cleanup;
	}
	if (fwrite(pubkey, sizeof(pubkey), 1, fpub) != 1) {
		fprintf(stderr, "Error writing public key\n");
		goto cleanup;
	}
	if (fwrite(privkey, sizeof(privkey), 1, fpriv) != 1) {
		fprintf(stderr, "Error writing private key\n");
		goto cleanup;
	}

	fprintf(stderr, "Public key saved to %s.\n", pubkeyfile);
	fprintf(stderr, "Private key saved to %s.\n", privkeyfile);
	fprintf(stderr, "Generated public key:\n");
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 8; j++) {
			fprintf(stderr, "%02x ", pubkey[i * 8 + j]);
		}
		fprintf(stderr, "\n");
	}
	retval = 0;
cleanup:
	if (fpub) fclose(fpub);
	if (fpriv) fclose(fpriv);
	return retval;
}
