#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "lzss.h"

uint64_t LZSS_Decode_Buffer_Predict(struct lzss_obj * in, struct lzss_pld * payload) {
	int32_t c_nRingBufferSize;
	int32_t c_nMatchLengthUpperLimit;
	int32_t c_nThreshold;
	uint64_t result = 0;
	char * buffer = NULL;
	int32_t c;
	int32_t i;
	int32_t j;
	int32_t k;
	int32_t r;

	uint32_t flags = 0;
/* Reset offset */
	in->off = 0;
/* Assign defaults if options are null */
	if(payload->options == NULL) {
		c_nRingBufferSize			= 4096;
		c_nMatchLengthUpperLimit	= 18;
		c_nThreshold				= 2;
	} else {
		c_nRingBufferSize			= payload->options->c_nRingBufferSize;
		c_nMatchLengthUpperLimit	= payload->options->c_nMatchLengthUpperLimit;
		c_nThreshold				= payload->options->c_nThreshold;
	}
	r = c_nRingBufferSize - c_nMatchLengthUpperLimit;
	buffer = malloc(c_nRingBufferSize + c_nMatchLengthUpperLimit - 1);
	memset(buffer,' ',r);

	while ( in->off < in->size ) {
		flags >>= 1;
		if (!(flags & 256)) {
			c = in->ptr[in->off++];
			flags = c | 0xff00;/* uses higher byte cleverly to count eight */
		}

		if (flags & 1) {
			c = in->ptr[in->off++];
			++result;

			buffer[r++] = c;
			r &= (c_nRingBufferSize - 1);
		} else {
			i = in->ptr[in->off++];
			j = in->ptr[in->off++];

			i |= ((j & 0xf0) << 4);
			j = (j & 0x0f) + c_nThreshold;

			for (k = 0; k <= j; k++) {
				c = buffer[(i + k) & (c_nRingBufferSize - 1)];

				++result;

				buffer[r++] = c;
				r &= (c_nRingBufferSize - 1);
			}
		}
	}
	free(buffer);
	in->off = 0;
	return result;
}

void LZSS_Decode_Buffer(struct lzss_obj	* in, void * out, struct lzss_pld * payload) {
	int32_t c_nRingBufferSize;
	int32_t c_nMatchLengthUpperLimit;
	int32_t c_nThreshold;

	uint64_t * outindex = NULL;
	char * buffer = NULL;
	int32_t c;
	int32_t i;
	int32_t j;
	int32_t k;
	int32_t r;

	uint32_t flags = 0;
/* Reset offset */
	in->off = 0;
/* Assign defaults if options are null */
	if(payload->options == NULL) {
		c_nRingBufferSize			= 4096;
		c_nMatchLengthUpperLimit	= 18;
		c_nThreshold				= 2;
	} else {
		c_nRingBufferSize			= payload->options->c_nRingBufferSize;
		c_nMatchLengthUpperLimit	= payload->options->c_nMatchLengthUpperLimit;
		c_nThreshold				= payload->options->c_nThreshold;
	}
	if(payload->operation & OUTPUT_MEMORY) outindex = &((struct lzss_obj *)out)->off;
	else outindex = malloc(8);
	*outindex = 0;
	r = c_nRingBufferSize - c_nMatchLengthUpperLimit;
	buffer = malloc(c_nRingBufferSize + c_nMatchLengthUpperLimit - 1);
	memset(buffer,' ',r);

	while ( in->off < in->size ) {
		flags >>= 1;
		if (!(flags & 256)) {
			c = in->ptr[in->off++];
			flags = c | 0xff00;/* uses higher byte cleverly to count eight */
		}

		if (flags & 1) {
			c = in->ptr[in->off++];
			if(payload->operation & OUTPUT_MEMORY) ((struct lzss_obj *)out)->ptr[*outindex++] = c;
			else {
				fputc(c,out);
				(*outindex)++;
			}

			buffer[r++] = c;
			r &= (c_nRingBufferSize - 1);
		} else {
			i = in->ptr[in->off++];
			j = in->ptr[in->off++];

			i |= ((j & 0xf0) << 4);
			j = (j & 0x0f) + c_nThreshold;

			for (k = 0; k <= j; k++) {
				c = buffer[(i + k) & (c_nRingBufferSize - 1)];

				if(payload->operation & OUTPUT_MEMORY) ((struct lzss_obj *)out)->ptr[*outindex++] = c;
				else {
					fputc(c,out);
					(*outindex)++;
				}

				buffer[r++] = c;
				r &= (c_nRingBufferSize - 1);
			}
		}
	}
	free(buffer);
	if(!(payload->operation & OUTPUT_MEMORY)) free(outindex);
	in->off = 0;
}

void LZSS_Decode_Stream(FILE * in, void * out, struct lzss_pld * payload) {
	int32_t c_nRingBufferSize;
	int32_t c_nMatchLengthUpperLimit;
	int32_t c_nThreshold;

	uint64_t ibuf = 0;
	uint64_t * outindex = NULL;
	char * buffer = NULL;
	int32_t c;
	int32_t i;
	int32_t j;
	int32_t k;
	int32_t r;

	uint32_t flags = 0;
/* Assign defaults if options are null */
	if(payload->options == NULL) {
		c_nRingBufferSize			= 4096;
		c_nMatchLengthUpperLimit	= 18;
		c_nThreshold				= 2;
	} else {
		c_nRingBufferSize			= payload->options->c_nRingBufferSize;
		c_nMatchLengthUpperLimit	= payload->options->c_nMatchLengthUpperLimit;
		c_nThreshold				= payload->options->c_nThreshold;
	}
	if(payload->operation & OUTPUT_MEMORY) outindex = &((struct lzss_obj *)out)->off;
	else outindex = malloc(8);
	*outindex = 0;
	r = c_nRingBufferSize - c_nMatchLengthUpperLimit;
	buffer = malloc(c_nRingBufferSize + c_nMatchLengthUpperLimit - 1);
	memset(buffer,' ',r);

	while (!feof(in)) {
		flags >>= 1;
		if (!(flags & 256)) {
			c = fgetc(in);
			++ibuf;
			flags = c | 0xff00;/* uses higher byte cleverly to count eight */
		}

		if (flags & 1) {
			c = fgetc(in);
			++ibuf;
			if(payload->operation & OUTPUT_MEMORY) ((struct lzss_obj *)out)->ptr[*outindex++] = c;
			else {
				fputc(c,out);
				(*outindex)++;
			}

			buffer[r++] = c;
			r &= (c_nRingBufferSize - 1);
		} else {
			i = fgetc(in); ++ibuf;
			j = fgetc(in); ++ibuf;

			i |= ((j & 0xf0) << 4);
			j = (j & 0x0f) + c_nThreshold;

			for (k = 0; k <= j; k++) {
				c = buffer[(i + k) & (c_nRingBufferSize - 1)];

				if(payload->operation & OUTPUT_MEMORY) ((struct lzss_obj *)out)->ptr[*outindex++] = c;
				else {
					fputc(c,out);
					(*outindex)++;
				}

				buffer[r++] = c;
				r &= (c_nRingBufferSize - 1);
			}
		}
	}
	free(buffer);
	if(!(payload->operation & OUTPUT_MEMORY)) free(outindex);
}

void LZSS_Encode_Stream(void * in, void * out, void * payload) {
	puts("Not implemented yet");
}
