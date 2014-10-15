/* LZSS encoder-decoder  (c) Haruhiko Okumura */

#include <stdio.h>
#include <stdlib.h>

#define EI 11  /* typically 10..13 */
#define EJ  4  /* typically 4..5 */
#define P   1  /* If match length <= P then output one character */
#define N (1 << EI)  /* buffer size */
#define F ((1 << EJ) + P)  /* lookahead buffer size */

#define LZSS_INPUT_IS_MEMORY 1
#define LZSS_INPUT_IS_FILE 2
#define LZSS_OUTPUT_IS_MEMORY 4
#define LZSS_OUTPUT_IS_FILE 8
#define LZSS_PREDICT_SIZE 16

int bit_buffer = 0, bit_mask = 128;
unsigned char buffer[N * 2];
unsigned long codecount = 0;

static void error(void) {
    printf("Output error\n");  exit(1);
}

static void putbit1(FILE * outfile) {
	bit_buffer |= bit_mask;
	if ((bit_mask >>= 1) == 0) {
		if (fputc(bit_buffer, outfile) == EOF) error();
		bit_buffer = 0;  bit_mask = 128;  codecount++;
	}
}

static void putbit0(FILE * outfile) {
	if ((bit_mask >>= 1) == 0) {
		if (fputc(bit_buffer, outfile) == EOF) error();
		bit_buffer = 0;  bit_mask = 128;  codecount++;
	}
}

static void flush_bit_buffer(FILE * outfile) {
	if (bit_mask != 128) {
		if (fputc(bit_buffer, outfile) == EOF) error();
		codecount++;
	}
}

static void output1(int c, FILE * outfile) {
	int mask;

	putbit1(outfile);
	mask = 256;
	while (mask >>= 1) {
		if (c & mask) putbit1(outfile);
		else putbit0(outfile);
	}
}

static void output2(int x, int y, FILE * outfile) {
	int mask;

	putbit0(outfile);
	mask = N;
	while (mask >>= 1) {
		if (x & mask) putbit1(outfile);
		else putbit0(outfile);
	}
	mask = (1 << EJ);
	while (mask >>= 1) {
		if (y & mask) putbit1(outfile);
		else putbit0(outfile);
	}
}

unsigned long lzss_encode(FILE * infile, FILE * outfile, unsigned char mode) {
    int i, j, f1, x, y, r, s, bufferend, c;
    unsigned long textcount = 0;
    if(mode & (LZSS_OUTPUT_IS_FILE | LZSS_OUTPUT_IS_MEMORY)) {
		fputs("lzss_encode cannot output to both memory and a file\n",stderr);
		return 0;
	}
	if(mode & (LZSS_INPUT_IS_FILE | LZSS_INPUT_IS_MEMORY)) {
		fputs("lzss_encode cannot take input from both memory and a file\n",stderr);
		return 0;
	}
	if(mode & LZSS_PREDICT_SIZE & (LZSS_OUTPUT_IS_FILE | LZSS_OUTPUT_IS_MEMORY)) {
		fputs("lzss_encode can only predict output size if there is no output\n",stderr);
		return 0;
	}

    for (i = 0; i < N - F; i++) buffer[i] = ' ';
    for (i = N - F; i < N * 2; i++) {
        if ((c = fgetc(infile)) == EOF) break;
        buffer[i] = c;  textcount++;
    }
    bufferend = i;  r = N - F;  s = 0;
    while (r < bufferend) {
        f1 = (F <= bufferend - r) ? F : bufferend - r;
        x = 0;  y = 1;  c = buffer[r];
        for (i = r - 1; i >= s; i--)
            if (buffer[i] == c) {
                for (j = 1; j < f1; j++)
                    if (buffer[i + j] != buffer[r + j]) break;
                if (j > y) {
                    x = i;  y = j;
                }
            }
        if (y <= P) output1(c,outfile);
        else output2(x & (N - 1), y - 2,outfile);
        r += y;  s += y;
        if (r >= N * 2 - F) {
            for (i = 0; i < N; i++) buffer[i] = buffer[i + N];
            bufferend -= N;  r -= N;  s -= N;
            while (bufferend < N * 2) {
                if ((c = fgetc(infile)) == EOF) break;
                buffer[bufferend++] = c;  textcount++;
            }
        }
    }
    return codecount;
}

static int getbit(int n, FILE * infile) /* get n bits */
{
    int i, x;
    static int buf, mask = 0;

    x = 0;
    for (i = 0; i < n; i++) {
        if (mask == 0) {
            if ((buf = fgetc(infile)) == EOF) return EOF;
            mask = 128;
        }
        x <<= 1;
        if (buf & mask) x++;
        mask >>= 1;
    }
    return x;
}

unsigned long lzss_decode(FILE * infile, void * outfile, unsigned char mode) {
	int i, j, k, r, c;
	unsigned long textcount = 0;
	if(mode & (LZSS_OUTPUT_IS_FILE | LZSS_OUTPUT_IS_MEMORY)) {
		fputs("lzss_decode cannot output to both memory and a file\n",stderr);
		return textcount;
	}
	if(mode & (LZSS_INPUT_IS_FILE | LZSS_INPUT_IS_MEMORY)) {
		fputs("lzss_decode cannot take input from both memory and a file\n",stderr);
		return textcount;
	}
	for (i = 0; i < N - F; i++) buffer[i] = ' ';
	r = N - F;
	while ((c = getbit(1,infile)) != EOF) {
		if (c) {
			if ((c = getbit(8,infile)) == EOF) break;
			if(mode & LZSS_COUNT_SIZE) textcount++;
			if(mode & LZSS_OUTPUT_IS_FILE) fputc(c,(FILE)outfile);
/*			if(mode & LZSS_OUTPUT_IS_MEMORY) ... */
			buffer[r++] = c;  r &= (N - 1);
		} else {
			if ((i = getbit(EI,infile)) == EOF) break;
			if ((j = getbit(EJ,infile)) == EOF) break;
			for (k = 0; k <= j + 1; k++) {
				c = buffer[(i + k) & (N - 1)];
				if(mode & LZSS_PREDICT_SIZE) textcount++;
				if(mode & LZSS_OUTPUT_IS_FILE) fputc(c,(FILE)outfile);
/*				if(mode & LZSS_OUTPUT_IS_MEMORY) ... */
				buffer[r++] = c;  r &= (N - 1);
			}
		}
	}
	return textcount;
}
