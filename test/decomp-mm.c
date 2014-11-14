#include <stdio.h>
#include <stdlib.h>
#include <lzss.h>

int main(int argc, char * argv[]) {
	FILE * infile;
	FILE * oufile;
	struct lzss_t output;
	struct lzss_t input;

	if(argc != 3) exit(0);

	infile = fopen(argv[1],"rb");
	fseek(infile,0,SEEK_END);
	input.size = ftell(infile);
	input.ptr = (char*)malloc(input.size);
	input.offset = 0;
	fseek(infile,0,SEEK_SET);
	fread(input.ptr,input.size,1,infile);
	fclose(infile);
	printf("Input buffer is %lu bytes\n",input.size);

	lzss_decode_mm(&input,&output);
	free(input.ptr);
	oufile = fopen(argv[2],"wb");
	fwrite(output.ptr,output.size,1,oufile);
	fclose(oufile);
	free(output.ptr);
	return 0;
}
