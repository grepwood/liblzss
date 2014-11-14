#include <stdio.h>
#include <stdlib.h>
#include <lzss.h>

int main(int argc, char * argv[]) {
	FILE * infile;
	FILE * oufile;
	struct lzss_t output;

	if( argc != 3) exit(-1);

	infile = fopen(argv[1],"rb");
	lzss_encode_fm(infile,&output);
	fclose(infile);

	oufile = fopen(argv[2],"wb");
	fwrite(output.ptr,output.size,1,oufile);
	fclose(oufile);
	free(output.ptr);
	return 0;
}
