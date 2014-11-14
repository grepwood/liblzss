#include <stdio.h>
#include <stdlib.h>
#include <lzss.h>

int main(int argc, char * argv[]) {
	FILE * infile;
	FILE * oufile;
	struct lzss_t input;

	if( argc != 3) exit(-1);

	infile = fopen(argv[1],"rb");

	fseek(infile,0,SEEK_END);
	input.size = ftell(infile);
	input.ptr = (char*)malloc(input.size);
	input.offset = 0;
	fseek(infile,0,SEEK_SET);
	fread(input.ptr,input.size,1,infile);
	fclose(infile);
	printf("Input buffer is %lu bytes\n",input.size);

	oufile = fopen(argv[2],"wb");
	lzss_decode_mf(&input,oufile);
	fclose(oufile);
	free(input.ptr);
	return 0;
}
