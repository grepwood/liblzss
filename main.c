#include <stdio.h>
#include <stdlib.h>
#include <lzss.h>
/*
int notmain(int argc, char *argv[]) {
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
	if (enc) lzss_encode_ff(infile,outfile);  else lzss_decode_ff(infile,outfile);
	fclose(infile);  fclose(outfile);
	return 0;
}
*/
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
	int temp;
	for(input.offset = 0; input.offset < input.size; ++input.offset) {
		temp = input.ptr[input.offset];
		temp &= 0x000000ff;
		printf("%X %hX\n",input.offset,temp);
	}
	input.offset = 0;

	printf("Allocated %i bytes\n",input.size);

	oufile = fopen(argv[2],"wb");
	lzss_decode_mf(&input,oufile);
	fclose(oufile);
	free(input.ptr);
	return 0;
}
