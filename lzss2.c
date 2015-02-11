#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct lzss_prm {
	int c_nRingBufferSize;
	int c_nMatchLengthUpperLimit;
	int c_nThreshold;
};

struct lzss_pld {
	char operation;
	struct lzss_prm * options;
};

struct lzss_obj {
	uint8_t * ptr;
	size_t size;
	size_t off;
};

#define INPUT_MEMORY 1
#define OUTPUT_MEMORY 2
#define NO_PREDICT 4
#define DECOMPRESS 8
#define DEFAULT_PARAMS 16

void LZSSOperate(void * input, void * output, struct lzss_pld * mode) {
	if(mode != NULL) {
		if(mode->options == NULL) 
		switch(mode->operation & 24) {
			case 0: 
			case 1: 
			case 2: 
			case 3: 
			case 4: 
			case 5: 
			case 6: 
			case 7: 
			case 8: 
			case 9: 
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
		}
	} else fputs("liblzss: null mode structure\n",stderr);
	
}

void LZSSEncode(void * in, void * out, struct lzss_pld * mode) {
	puts("Not implemented yet.");
}

void LZSSDecode(void * in, void * out, struct lzss_pld * mode) {
	int local_nRingBufferSize        = 4096;
	int local_nMatchLengthUpperLimit =   18;
	int local_nThreshold             =    2;
	int64_t len;
	char * buffer = NULL;
	int32_t ibuf;
	int32_t c;
	int32_t i;
	int32_t j;
	int32_t k;
	int32_t r;
	uint32_t flags = 0;
/* Set parameters accordingly to passed structure or leave them be */
	if(!(mode->operation & DEFAULT_PARAMS)) {
		local_nRingBufferSize = mode->options->c_nRingBufferSize;
		local_nMatchLengthUpperLimit = mode->options->c_nMatchLengthUpperLimit;
		local_nThreshold = mode->options->c_nThreshold;
	}
/* r and buffer need to be properly initialized after parameters are set */
	buffer = malloc(local_nRingBufferSize + local_nMatchLengthUpperLimit - 1);
	r = local_nRingBufferSize - local_nMatchLengthUpperLimit;
/* Wiping buffer */
	memset(buffer,' ',i = local_nRingBufferSize - local_nMatchLengthUpperLimit);
/* Decompression */
	if(mode->operation & INPUT_MEMORY) {
		ibuf = (struct lzss_obj*)in->off;
		len = (struct lzss_obj*)in->size;
	else ibuf = 0;
	while ( ibuf < len ) {
		if (((flags >>= 1) & 256) == 0) {
			c = in[ibuf++];
			flags = c | 0xff00;/* uses higher byte cleverly to count eight */
		}
		if (flags & 1) {
			c = in[ibuf++];
			out[m_outindex++] = c;
			buffer[r++] = c;
			r &= (local_nRingBufferSize - 1);
		} else {
			if(
			i = in[ibuf++];
			j = in[ibuf++];
			i |= ((j & 0xf0) << 4);
			j = (j & 0x0f) + local_nThreshold;
			for (k = 0; k <= j; k++) {
				c = buffer[(i + k) & (local_nRingBufferSize - 1)];
				out[m_outindex++] = c;
				buffer[r++] = c;
				r &= (local_nRingBufferSize - 1);
			}
		}
	}
/* Standard cleanup */
	free(buffer);
/* Operation-specific cleanup */
}
