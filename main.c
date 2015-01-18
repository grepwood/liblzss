#include <stdio.h>
#include <lzss.h>
#define ENCODE	1
#define DECODE	2
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
	puts("\tc\toutput to stdout instead of a file");
	puts("\ti\ttake input from stdin instead of a file");
	exit(a);
}

char * add_extension(const char * original) {
	size_t i = strlen(original)+6;
	static const char ext[6] = {'.','l','z','s','s',0};
	char * result = malloc(i);
	memcpy(result,original,i-6);
	memcpy(result+i-6,ext,6);
	return result;
}

int main(int argc, char **argv) {
	int c;
	char *fi = NULL;
	char *fo = NULL;
	char *path = NULL;
	uint8_t lzss_power = 0; /* 0-14 */
	uint8_t command = 0;

	while ((c = getopt(argc, argv, "zdhpci")) != -1) {
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
		case 'c':
			command += STDOUT;
			break;
		case 'i':
			command += STDIN;
			break;
		default:
			fprintf(stderr, "bad arg: -%c\n", c);
			usage(1);
		}
	}
	if(!((command & 3)%3)) {
		puts("Select either decoding or encoding.");
		usage(1);
	}
	switch(command & UNIXPIPE) {
		case 0: /* file input, file output */
			path = add_extension(...);
			argv[optind]
		case STDOUT:
			fo = stdout;
		case STDIN:
			fi = stdin;
		case UNIXPIPE:
			fi = stdin;
			fo = stdout;
	}
	switch(command & 3) {
		case ENCODE:
			lzss_encode_ff(fi,fo);
			break;
		case DECODE:
			lzss_decode_ff(fi,fo);
			break;
	}
	fclose(fi);
	fclose(fo);
	return 0;
}
