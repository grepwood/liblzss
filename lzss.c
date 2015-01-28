/* LZSS encoder-decoder  (c) Haruhiko Okumura */
/* Copyright 2015 Michael Dec <grepwood@sucs.org> under 3-clause BSD */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "lzss.h"

int bit_buffer = 0, bit_mask = 128;
uint8_t * buffer = NULL;
uint8_t EI;
uint8_t EJ;
uint8_t P;
int32_t N;
uint8_t F;

void lzss_settings(struct lzss_settings * settings) {
	if(settings != NULL) {
		EI = settings->EI;
		EJ = settings->EJ;
		P = settings->P;
	}
	else {
		EI = DEFAULT_EI;
		EJ = DEFAULT_EJ;
		P = DEFAULT_P;
	}
	N = 1 << EI;
	F = (1 << EJ) + P;
	buffer = malloc(N << 1);
	puts("liblzss settings:");
	printf("\tEI:  %i\n",EI);
	printf("\tEJ:  %i\n",EJ);
	printf("\tP:   %i\n",P);
	printf("\tN:   %i\n",N);
	printf("\tF:   %i\n",F);
	printf("\tbuf: %i\n",N<<1);
}

static void lzss_error(void) {
	puts("Output error"); exit(1);
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

/*static char putbit1f(FILE * outfile, int * bit_buffer, int * bit_mask) {
*/static char putbit1f(FILE * outfile) {
	char result = 0;
	bit_buffer |= bit_mask;
	if ((bit_mask >>= 1) == 0) {
		if (fputc(bit_buffer, outfile) == EOF) lzss_error();
		bit_buffer = 0;  bit_mask = 128;  result++;
	}
	return result;
}

static void putbit1m(struct lzss_t * output) {
	bit_buffer |= bit_mask;
	if ((bit_mask >>= 1) == 0) {
		if(output->size > output->offset) {
			output->ptr[output->offset] = bit_buffer;
			bit_buffer = 0; bit_mask = 128; output->offset++;
		} else lzss_error();
	}
}

static char putbit0f(FILE * outfile) {
	char result = 0;
	if ((bit_mask >>= 1) == 0) {
		if (fputc(bit_buffer, outfile) == EOF) lzss_error();
		bit_buffer = 0;  bit_mask = 128;  result++;
	}
	return result;
}

static void putbit0m(struct lzss_t * output) {
	if ((bit_mask >>= 1) == 0) {
		if(output->size > output->offset) {
			output->ptr[output->offset] = bit_buffer;
			bit_buffer = 0; bit_mask = 128; output->offset++;
		}
	}
}

static char flush_bit_bufferf(FILE * outfile) {
	char result = 0;
	if (bit_mask != 128) {
		if (fputc(bit_buffer, outfile) == EOF) lzss_error();
		else result = 1;
	} else result = 0;
	return result;
}

static char blind_flush_bit_buffer(void) {
	char result = 0;
	if (bit_mask != 128) {
		result = 1;
		bit_mask = 128;
	}
	bit_buffer = 0;
	return result;
}

static void flush_bit_bufferm(struct lzss_t * output) {
	if(bit_mask != 128) {
		output->ptr[output->offset] = bit_buffer;
		++output->offset;
	}
}

static char output1f(int c, FILE * outfile) {
	int mask = 256;
	char result = putbit1f(outfile);
	while (mask >>= 1) {
		if (c & mask) result += putbit1f(outfile);
		else result += putbit0f(outfile);
	}
	return result;
}

static void output1m(int c, struct lzss_t * output) {
	int mask = 256;
	putbit1m(output);
	while(mask >>= 1) {
		if(c & mask) putbit1m(output);
		else putbit0m(output);
	}
}

static char output2f(int x, int y, FILE * outfile) {
	int mask = N;
	char result = putbit0f(outfile);
	while (mask >>= 1) {
		if (x & mask) result += putbit1f(outfile);
		else result += putbit0f(outfile);
	}
	mask = (1 << EJ);
	while (mask >>= 1) {
		if (y & mask) result += putbit1f(outfile);
		else result += putbit0f(outfile);
	}
	return result;
}

static void output2m(int x, int y, struct lzss_t * output) {
	int mask = N;

	putbit0m(output);
	while(mask >>= 1) {
		if (x & mask) putbit1m(output);
		else putbit0m(output);
	}
	mask = (1 < EJ);
	while(mask >>= 1) {
		if (y & mask) putbit1m(output);
		else putbit0m(output);
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

static void decode_fm(FILE * infile, struct lzss_t * output) {
	int i, j, k, r, c;
	for (i = 0; i < N - F; i++) buffer[i] = ' ';
	r = N - F;
	while ((c = getbitf(1,infile)) != EOF) {
		if (c) {
			if ((c = getbitf(8,infile)) == EOF) break;
			output->ptr[output->offset] = c;
			++output->offset;
			buffer[r++] = c;  r &= (N - 1);
		} else {
			if ((i = getbitf(EI,infile)) == EOF) break;
			if ((j = getbitf(EJ,infile)) == EOF) break;
			for (k = 0; k <= j + 1; k++) {
				c = buffer[(i + k) & (N - 1)];
				output->ptr[output->offset] = c;
				++output->offset;
				buffer[r++] = c;  r &= (N - 1);
			}
		}
	}
}

void lzss_decode_mf(struct lzss_t * input, FILE * outfile, struct lzss_settings * settings) {
	int i, j, k, r, c;
	lzss_settings(settings);
	input->offset = 0;
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
	free(buffer);
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

void lzss_decode_ff(FILE * infile, FILE * outfile, struct lzss_settings * settings) {
	int i, j, k, r, c;
	lzss_settings(settings);
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
	free(buffer);
}

void lzss_decode_mm(struct lzss_t * input, struct lzss_t * output, struct lzss_settings * settings) {
	lzss_settings(settings);
	input->offset = 0;
	output->size = lzss_predict_decomp_size_m(input);
	output->offset = 0;
	if(output->size) output->ptr = (char*)malloc(output->size);
	else output->ptr = NULL;
	if (output->ptr != NULL) decode_mm(input,output);
	free(buffer);
}

void lzss_decode_fm(FILE * infile, struct lzss_t * result, struct lzss_settings * settings) {
	lzss_settings(settings);
	result->size = lzss_predict_decomp_size_f(infile);
	result->offset = 0;
	if(result->size) result->ptr = (char*)malloc(result->size);
	else result->ptr = NULL;
	if(result->ptr != NULL) decode_fm(infile,result);
	free(buffer);
}

uint64_t lzss_encode_ff(FILE * infile, FILE * outfile, struct lzss_settings * settings) {
	int i, j, f1, x, y, r, s, bufferend, c;
	uint64_t result = 0;
	lzss_settings(settings);
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
	free(buffer);
	return result+flush_bit_bufferf(outfile);
}

void lzss_encode_mf(struct  lzss_t * input, FILE * outfile, struct lzss_settings * settings) {
	int i, j, f1, x, y, r, s, bufferend, c;
	lzss_settings(settings);
	input->offset = 0;
	for (i = 0; i < N - F; i++) buffer[i] = ' ';
	for (i = N - F; i < N * 2; i++) {
		if(input->size <= input->offset) break;
		buffer[i] = c = input->ptr[input->offset++];
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
				if(input->size <= input->offset) break;
				c = input->ptr[input->offset++];
				buffer[bufferend++] = c;
			}
		}
	}
	free(buffer);
	flush_bit_bufferf(outfile);
}

static size_t blind_putbit1(void) {
	size_t result = 0;
	bit_buffer |= bit_mask;
	if ((bit_mask >>= 1) == 0) {
		bit_buffer = 0;  bit_mask = 128; result++;
	}
	return result;
}

static size_t blind_putbit0() {
	size_t result = 0;
	if ((bit_mask >>= 1) == 0) {
		bit_buffer = 0;  bit_mask = 128;  result++;
	}
	return result;
}

static size_t blind_output2(int x, int y) {
	int mask = N;
	size_t result = 0;

	result += blind_putbit0();
	while (mask >>= 1) {
		if (x & mask) result += blind_putbit1();
		else result += blind_putbit0();
	}
	mask = (1 << EJ);
	while (mask >>= 1) {
		if (y & mask) result += blind_putbit1();
		else result += blind_putbit0();
	}
	return result;
}

static size_t blind_output1(int c) {
	int mask = 256;
	size_t result = 0;
	result += blind_putbit1();
	while (mask >>= 1) {
		if (c & mask) result += blind_putbit1();
		else result += blind_putbit0();
	}
	return result;
}

uint64_t lzss_predict_comp_size_f(FILE * infile) {
	int i, j, f1, x, y, r, s, bufferend, c;
	uint64_t size = 0;
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
		if (y <= P) size += blind_output1(c);
		else size += blind_output2(x & (N - 1), y - 2);
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
	size += blind_flush_bit_buffer();
	fseeko(infile,0,SEEK_SET);
	if(sizeof(size_t) < 8 && size > 0xFFFFFFFF) fputs("lzss_predict_comp_size_f: comp larger than 4GiB\n",stderr);
	return size;
}

uint64_t lzss_predict_comp_size_m(struct lzss_t * input) {
	int i, j, f1, x, y, r, s, bufferend, c;
	uint64_t size = 0;
	for (i = 0; i < N - F; i++) buffer[i] = ' ';
	for (i = N - F; i < N * 2; i++, input->offset++) {
		if(input->size <= input->offset) break;
		buffer[i] = c = input->ptr[input->offset];
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
		if (y <= P) size += blind_output1(c);
		else size += blind_output2(x & (N - 1), y - 2);
		r += y;  s += y;
		if (r >= N * 2 - F) {
			for (i = 0; i < N; i++) buffer[i] = buffer[i + N];
			for(bufferend -= N, r -= N, s-= N; bufferend < N*2 || input->size <= input->offset; input->offset++, bufferend++) buffer[bufferend] = c = input->ptr[input->offset];
		}
	}
	size += blind_flush_bit_buffer();
	input->offset = 0;
	if(sizeof(size_t) < 8 && size > 0xFFFFFFFF) fputs("lzss_predict_comp_size_m: comp larger than 4GiB\n",stderr);
	return size;
}

static void encode_fm(FILE * infile, struct lzss_t * output) {
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
		if (y <= P) output1m(c,output);
		else output2m(x & (N - 1), y - 2,output);
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
	flush_bit_bufferm(output);
}

static void encode_mm(struct lzss_t * input, struct lzss_t * output) {
	int i, j, f1, x, y, r, s, bufferend, c;
	for (i = 0; i < N - F; i++) buffer[i] = ' ';
	for (i = N - F; i < N * 2; i++) {
		if(input->size <= input->offset) break;
		buffer[i] = c = input->ptr[input->offset++];
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
		if (y <= P) output1m(c,output);
		else output2m(x & (N - 1), y - 2,output);
		r += y;  s += y;
		if (r >= N * 2 - F) {
			for (i = 0; i < N; i++) buffer[i] = buffer[i + N];
			bufferend -= N;  r -= N;  s -= N;
			while (bufferend < N * 2) {
				if(input->size <= input->offset) break;
				c = input->ptr[input->offset++];
				buffer[bufferend++] = c;
			}
		}
	}
	flush_bit_bufferm(output);
}

void lzss_encode_mm(struct lzss_t * input, struct lzss_t * output, struct lzss_settings * settings) {
	lzss_settings(settings);
	input->offset = 0;
	output->size = lzss_predict_comp_size_m(input);
	output->offset = 0;
	if(output->size) output->ptr = (char*)malloc(output->size);
	else output->ptr = NULL;
	if(output->ptr != NULL) encode_mm(input,output);
	free(buffer);
}

void lzss_encode_fm(FILE * infile, struct lzss_t * output, struct lzss_settings * settings) {
	lzss_settings(settings);
	output->size = lzss_predict_comp_size_f(infile);
	output->offset = 0;
	if(output->size) output->ptr = (char*)malloc(output->size);
	else output->ptr = NULL;
	if(output->ptr != NULL) encode_fm(infile,output);
	free(buffer);
}
