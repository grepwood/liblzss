#include <stdio.h>
#include <stdlib.h>
#include <lzss.h>

int main(int argc, char * argv[]) {
	FILE * infile;
	FILE * oufile;
	int err;
	struct lzss_t output;
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
	lzss_encode_mm(&input,&output);

	oufile = fopen(argv[2],"wb");
	fwrite(output.ptr,output.size,1,oufile);
	fclose(oufile);
	free(input.ptr);
	free(output.ptr);
	return 0;
}
