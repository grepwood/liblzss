/* LZSS encoder-decoder  (c) Haruhiko Okumura */

#include <stdio.h>
#include <stdlib.h>

#define EI 11  /* typically 10..13 */
#define EJ  4  /* typically 4..5 */
#define P   1  /* If match length <= P then output one character */
#define N (1 << EI)  /* buffer size */
#define F ((1 << EJ) + P)  /* lookahead buffer size */

int bit_buffer = 0, bit_mask = 128;
unsigned long codecount = 0, textcount = 0;
unsigned char buffer[N * 2];
FILE *infile, *outfile;

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

void lzss_encode(FILE * infile, FILE * outfile) {
    int i, j, f1, x, y, r, s, bufferend, c;

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
    flush_bit_buffer(outfile);
    printf("text:  %ld bytes\n", textcount);
    printf("code:  %ld bytes (%ld%%)\n",
        codecount, (codecount * 100) / textcount);
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

void lzss_decode(FILE * infile, FILE * outfile) {
	int i, j, k, r, c;

	for (i = 0; i < N - F; i++) buffer[i] = ' ';
	r = N - F;
	while ((c = getbit(1,infile)) != EOF) {
		if (c) {
			if ((c = getbit(8,infile)) == EOF) break;
			fputc(c, outfile);
			buffer[r++] = c;  r &= (N - 1);
		} else {
			if ((i = getbit(EI,infile)) == EOF) break;
			if ((j = getbit(EJ,infile)) == EOF) break;
			for (k = 0; k <= j + 1; k++) {
				c = buffer[(i + k) & (N - 1)];
				fputc(c, outfile);
				buffer[r++] = c;  r &= (N - 1);
			}
		}
	}
}

int main(int argc, char *argv[]) {
	int enc;
	char *s;

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
	if (enc) lzss_encode(infile,outfile);  else lzss_decode(infile,outfile);
	fclose(infile);  fclose(outfile);
	return 0;
}
