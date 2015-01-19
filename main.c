#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <lzss.h>
#define ENCODE	1
#define DECODE	2
#define BOTH_WAYS	3
#define STDOUT	4
#define STDIN	8
#define UNIXPIPE 12
void usage(char a) {
	puts("Usage: lzss [OPTIONS]... [FILE]...");
	puts("Compression or decompression of FILES in .lzss format.\n");
	puts("\tz\tforcing compression");
	puts("\td\tforcing decompression");
	puts("\th\thelp, shows this message and exits");
	puts("\tp\tsets compressor power, range 11-14. default 11");
	puts("\to\toutput to file rather than stdout");
	puts("\ti\ttake input from file rather than stdin");
	exit(a);
}
/*
char * add_extension(const char * original) {
	size_t i = strlen(original)+6;
	static const char ext[6] = {'.','l','z','s','s',0};
	char * result = malloc(i);
	memcpy(result,original,i-6);
	memcpy(result+i-6,ext,6);
	return result;
}
*/
void getout(char * path, FILE * fi, FILE * fo) {
	if(path != NULL) free(path);
	if(fi != NULL) fclose(fi);
	if(fo != NULL) fclose(fo);
	puts("Select either decoding or encoding.");
	usage(1);
}

int main(int argc, char **argv) {
	int c;
	FILE *fi = NULL;
	FILE *fo = NULL;
	char *path = NULL;
	uint8_t lzss_power = 11; /* 11-14 */
	uint8_t command = UNIXPIPE;

	while ((c = getopt(argc, argv, "zdhpoi")) != -1) {
		switch (c) {
		case 'z':
			command += ENCODE;
			break;
		case 'd':
			command += DECODE;
			break;
		case 'h':
			usage(0);
			break;
		case 'p':
			lzss_power = strtoul(argv[optind],NULL,10);
			break;
		case 'o':
			command -= STDOUT;
			fo = fopen(argv[optind],"wb");
			break;
		case 'i':
			command -= STDIN;
			fi = fopen(argv[optind],"rb");
			break;
		default:
			fprintf(stderr, "bad arg: -%c\n", c);
			usage(1);
		}
	}
	if(command & STDIN) fi = stdin;
	if(command & STDOUT) fo = stdout;
	switch(command & BOTH_WAYS) {
		case 0:
			getout(path,fi,fo);
			break;
		case ENCODE:
			lzss_encode_ff(fi,fo);
			break;
		case DECODE:
			lzss_decode_ff(fi,fo);
			break;
		case BOTH_WAYS:
			getout(path,fi,fo);
			break;
	}
	fclose(fi);
	fclose(fo);
	return 0;
}
