#include <stdint.h>

struct lzss_prm {
	int32_t c_nRingBufferSize;
	int32_t c_nMatchLengthUpperLimit;
	int32_t c_nThreshold;
};

struct lzss_pld {
	char operation;
	struct lzss_prm * options;
};

struct lzss_obj {
	char * ptr;
	size_t size;
	size_t off;
};

#define INPUT_MEMORY 1
#define OUTPUT_MEMORY 2
#define NO_PREDICT 4
#define DECOMPRESS 8
#define DEFAULT_PARAMS 16

uint64_t LZSS_Decode_Buffer_Predict(struct lzss_obj * in, struct lzss_pld * payload);
void LZSS_Decode_Buffer(struct lzss_obj	* in, void * out, struct lzss_pld * payload);
void LZSS_Decode_Stream(FILE * in, void * out, struct lzss_pld * payload);

void LZSS_Encode_Stream(void * in, void * out, void * payload);
