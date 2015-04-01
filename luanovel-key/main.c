#include <stdio.h>
#include <string.h>

int luanovel_keygen(int argc, char* argv[]);
int luanovel_sign(int argc, char* argv[]);
int luanovel_verify(int argc, char* argv[]);

int main(int argc, char* argv[])
{
	const char* op;

	if (argc < 2) {
		fprintf(stderr, "%s {keygen,sign,verify}", argv[0]);
		return 0;
	}

	op = argv[1];
	if (!strcmp(op, "keygen")) {
		return luanovel_keygen(argc, argv);
	}
	if (!strcmp(op, "sign")) {
		return luanovel_sign(argc, argv);
	}
	if (!strcmp(op, "verify")) {
		int retval = luanovel_verify(argc, argv);
		if (retval == 1) {
			fprintf(stderr, "Verify failed.");
		}
		else if (retval == 0) {
			fprintf(stderr, "Verify succeeded.");
		}
		return retval;
	}
	fprintf(stderr, "%s {keygen,sign,verify}", argv[0]);
	return 0;
}
