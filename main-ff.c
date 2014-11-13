#include <stdio.h>
#include <stdlib.h>
#include <lzss.h>

int main(int argc, char * argv[]) {
	FILE * infile;
	FILE * oufile;

	if( argc != 3) exit(-1);

	infile = fopen(argv[1],"rb");
	oufile = fopen(argv[2],"wb");
	lzss_decode_ff(infile,oufile);
	fclose(oufile);
	fclose(infile);
	return 0;
}
