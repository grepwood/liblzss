#include <stdio.h>
#include <stdlib.h>
#include <lzss.h>

int main(int argc, char * argv[]) {
	FILE * infile;
	FILE * oufile;
	int err;
	struct lzss_t input;

	if( argc != 3) exit(-1);

	infile = fopen(argv[1],"rb");

	fseek(infile,0,SEEK_END);
	input.size = ftell(infile);
	input.ptr = (char*)malloc(input.size);
	fseek(infile,0,SEEK_SET);
	err = fread(input.ptr,input.size,1,infile);
	if(err != 1) fputs("fread not 1\n",stderr);
	fclose(infile);

	oufile = fopen(argv[2],"wb");
	lzss_decode_mf(&input,oufile);
	fclose(oufile);
	free(input.ptr);
	return 0;
}
