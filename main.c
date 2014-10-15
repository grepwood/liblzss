#include <stdio.h>
#include <lzss.h>

int main(int argc, char *argv[]) {
	int enc;
	char *s;
	FILE * infile = NULL;
	FILE * outfile = NULL;

	if (argc != 4) {
		printf("Usage: lzss e/d infile outfile\n\te = encode\td = decode\n");
		return 1;
	}
	s = argv[1];
	if (s[1] == 0 && (*s == 'd' || *s == 'D' || *s == 'e' || *s == 'E'))
		enc = (*s == 'e' || *s == 'E');
	else {
		printf("? %s\n", s);
	return 1;
	}
	if ((infile  = fopen(argv[2], "rb")) == NULL) {
		printf("? %s\n", argv[2]);  return 1;
	}
	if ((outfile = fopen(argv[3], "wb")) == NULL) {
		printf("? %s\n", argv[3]);  return 1;
	}
	if (enc) lzss_encode(infile,outfile);  else lzss_decode(infile,outfile);
	fclose(infile);  fclose(outfile);
	return 0;
}
