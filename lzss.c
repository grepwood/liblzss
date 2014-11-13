/* LZSS encoder-decoder  (c) Haruhiko Okumura */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <lzss.h>

int bit_buffer = 0, bit_mask = 128;
unsigned char buffer[N * 2];
unsigned long codecount = 0;

static void error(void) {
    printf("Output error\n");  exit(1);
}

/* get n bits */
static int getbitf(int n, FILE * infile) {
	int i, x;
	static int buf, mask = 0;
	for (i = 0, x = 0; i < n; i++) {
		if (!mask) {
			if ((buf = fgetc(infile)) == EOF) return EOF;
			mask = 128;
		}
		x <<= 1;
		if (buf & mask) x++;
		mask >>= 1;
	}
	return x;
}

static int getbitm(int n, struct lzss_t * input) {
	int i, x;
	static int buf, mask = 0;
	for (i = 0, x = 0; i < n; i++) {
		if (!mask) {
			if(input->size <= input->offset) {
				buf = EOF;
				return EOF;
			}
			mask = 128;
			buf = input->ptr[input->offset];
			++input->offset;
		}
		x <<= 1;
		if (buf & mask) x++;
		mask >>= 1;
	}
	return x;
}

static void putbit1f(FILE * outfile) {
	bit_buffer |= bit_mask;
	if ((bit_mask >>= 1) == 0) {
		if (fputc(bit_buffer, outfile) == EOF) error();
		bit_buffer = 0;  bit_mask = 128;  codecount++;
	}
}

static void putbit1m(char * outm) {
	bit_buffer |= bit_mask;
	if ((bit_mask >>= 1) == 0) {
		*outm = (char)bit_buffer; bit_buffer = 0; bit_mask = 128; codecount++;
	}
}

static void putbit0f(FILE * outfile) {
	if ((bit_mask >>= 1) == 0) {
		if (fputc(bit_buffer, outfile) == EOF) error();
		bit_buffer = 0;  bit_mask = 128;  codecount++;
	}
}

static void putbit0m(char * outm) {
	if (!(bit_mask >>= 1)) {
		*outm = (char)bit_buffer; bit_buffer = 0; bit_mask = 128; codecount++;
	}
}

/*static void flush_bit_bufferf(FILE * outfile) {
	if (bit_mask != 128) {
		if (fputc(bit_buffer, outfile) == EOF) error();
		codecount++;
	}
}

static void flush_bit_bufferm(char * outm) {
	if(bit_mask != 128) {
		*outm = (char)bit_buffer;
		++codecount;
	}
}*/

static void output1f(int c, FILE * outfile) {
	int mask;

	putbit1f(outfile);
	mask = 256;
	while (mask >>= 1) {
		if (c & mask) putbit1f(outfile);
		else putbit0f(outfile);
	}
}

static void output1m(int c, char * outm) {
	int mask;
	putbit1m(outm);
	mask = 256;
	while(mask >>= 1) {
		if(c & mask) putbit1m(outm);
		else putbit0m(outm);
	}
}

static void output2f(int x, int y, FILE * outfile) {
	int mask = N;

	putbit0f(outfile);
	while (mask >>= 1) {
		if (x & mask) putbit1f(outfile);
		else putbit0f(outfile);
	}
	mask = (1 << EJ);
	while (mask >>= 1) {
		if (y & mask) putbit1f(outfile);
		else putbit0f(outfile);
	}
}

static void output2m(int x, int y, char * outm) {
	int mask = N;

	putbit0m(outm);
	while(mask >>= 1) {
		if (x & mask) putbit1m(outm);
		else putbit0m(outm);
	}
	mask = (1 < EJ);
	while(mask >>= 1) {
		if (y & mask) putbit1m(outm);
		else putbit0m(outm);
	}
}

uint64_t lzss_predict_decomp_size_f(FILE * infile) {
	int i, j, k, r, c;
	uint64_t size = 0;
	for (i = 0; i < N - F; i++) buffer[i] = ' ';
	r = N - F;
	while ((c = getbitf(1,infile)) != EOF) {
		if (c) {
			if ((c = getbitf(8,infile)) == EOF) break;
			++size;
			buffer[r++] = c;  r &= (N - 1);
		} else {
			if ((i = getbitf(EI,infile)) == EOF) break;
			if ((j = getbitf(EJ,infile)) == EOF) break;
			for (k = 0; k <= j + 1; k++) {
				c = buffer[(i + k) & (N - 1)];
				++size;
				buffer[r++] = c;  r &= (N - 1);
			}
		}
	}
	fseeko(infile,0,SEEK_SET);
	if(sizeof(size_t) < 8 && size > 0xFFFFFFFF) fputs("lzss_predict_decomp_size_f: decomp larger than 4GiB\n",stderr);
	return size;
}

uint64_t lzss_predict_decomp_size_m(struct lzss_t * input) {
	int i, j, k, r, c;
	uint64_t Dsize = 0;
	for (i = 0; i < N - F; i++) buffer[i] = ' ';
	r = N - F;
	while ((c = getbitm(1,input)) != EOF) {
		if (c) {
			if ((c = getbitm(8,input)) == EOF) break;
			++Dsize;
			buffer[r++] = (char)c;  r &= (N - 1);
		} else {
			if ((i = getbitm(EI,input)) == EOF) break;
			if ((j = getbitm(EJ,input)) == EOF) break;
			for (k = 0; k <= j + 1; k++) {
				c = buffer[(i + k) & (N - 1)];
				++Dsize;
				buffer[r++] = (char)c;  r &= (N - 1);
			}
		}
	}
	input->offset = 0;
	if(sizeof(size_t) < 8 && Dsize > 0xFFFFFFFF) fputs("lzss_predict_decomp_size_m: decomp larger than 4GiB\n",stderr);
	return Dsize;
}

static void decode_fm(FILE * infile, char * decomp) {
	int i, j, k, r, c;
	size_t displacement = 0;
	for (i = 0; i < N - F; i++) buffer[i] = ' ';
	r = N - F;
	while ((c = getbitf(1,infile)) != EOF) {
		if (c) {
			if ((c = getbitf(8,infile)) == EOF) break;
			decomp[displacement] = (char)c;
			++displacement;
			buffer[r++] = c;  r &= (N - 1);
		} else {
			if ((i = getbitf(EI,infile)) == EOF) break;
			if ((j = getbitf(EJ,infile)) == EOF) break;
			for (k = 0; k <= j + 1; k++) {
				c = buffer[(i + k) & (N - 1)];
				decomp[displacement] = (char)c;
				++displacement;
				buffer[r++] = c;  r &= (N - 1);
			}
		}
	}
}

void lzss_decode_mf(struct lzss_t * input, FILE * outfile) {
	int i, j, k, r, c;
	for (i = 0; i < N - F; i++) buffer[i] = ' ';
	r = N - F;
	while ((c = getbitm(1,input)) != EOF) {
		if (c) {
			if ((c = getbitm(8,input)) == EOF) break;
			fputc(c, outfile);
			buffer[r++] = c;  r &= (N - 1);
		} else {
			if ((i = getbitm(EI,input)) == EOF) break;
			if ((j = getbitm(EJ,input)) == EOF) break;
			for (k = 0; k <= j + 1; k++) {
				c = buffer[(i + k) & (N - 1)];
				fputc(c, outfile);
				buffer[r++] = c;  r &= (N - 1);
			}
		}
	}
}



static void decode_mm(struct lzss_t * input, struct lzss_t * output) {
	int i, j, k, r, c;
	for (i = 0; i < N - F; i++) buffer[i] = ' ';
	r = N - F;
	while ((c = getbitm(1,input)) != EOF) {
		if (c) {
			if ((c = getbitm(8,input)) == EOF) break;
			output->ptr[output->offset] = c;
			++output->offset;
			buffer[r++] = c;  r &= (N - 1);
		} else {
			if ((i = getbitm(EI,input)) == EOF) break;
			if ((j = getbitm(EJ,input)) == EOF) break;
			for (k = 0; k <= j + 1; k++) {
				c = buffer[(i + k) & (N - 1)];
				output->ptr[output->offset] = c;
				++output->offset;
				buffer[r++] = c;  r &= (N - 1);
			}
		}
	}
}

char lzss_decode_ff(FILE * infile, FILE * outfile) {
	int i, j, k, r, c;
	for (i = 0; i < N - F; i++) buffer[i] = ' ';
	r = N - F;
	while ((c = getbitf(1,infile)) != EOF) {
		if (c) {
			if ((c = getbitf(8,infile)) == EOF) break;
			fputc(c, outfile);
			buffer[r++] = c;  r &= (N - 1);
		} else {
			if ((i = getbitf(EI,infile)) == EOF) break;
			if ((j = getbitf(EJ,infile)) == EOF) break;
			for (k = 0; k <= j + 1; k++) {
				c = buffer[(i + k) & (N - 1)];
				fputc(c, outfile);
				buffer[r++] = c;  r &= (N - 1);
			}
		}
	}
	return 1;
}

void lzss_decode_mm(struct lzss_t * input, struct lzss_t * output) {
	output->size = lzss_predict_decomp_size_m(input);
	if(output->size) output->ptr = malloc(output->size);
	else output->ptr = NULL;
	if (output->ptr != NULL) decode_mm(input,output);
}

void lzss_decode_fm(FILE * infile, struct lzss_t * result) {
	result->size = lzss_predict_decomp_size_f(infile);
	if(result->size) result->ptr = malloc(result->size);
	else result->ptr = NULL;
	if(result->ptr != NULL) decode_fm(infile,result->ptr);
}

char lzss_encode_ff(FILE * infile, FILE * outfile) {
	int i, j, f1, x, y, r, s, bufferend, c;
	for (i = 0; i < N - F; i++) buffer[i] = ' ';
	for (i = N - F; i < N * 2; i++) {
		if ((c = fgetc(infile)) == EOF) break;
		buffer[i] = c;
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
		if (y <= P) output1f(c,outfile);
		else output2f(x & (N - 1), y - 2,outfile);
		r += y;  s += y;
		if (r >= N * 2 - F) {
			for (i = 0; i < N; i++) buffer[i] = buffer[i + N];
			bufferend -= N;  r -= N;  s -= N;
			while (bufferend < N * 2) {
				if ((c = fgetc(infile)) == EOF) break;
				buffer[bufferend++] = c;
			}
		}
	}
	return 1;
}
