#define LZSS_INPUT_IS_MEMORY 1
#define LZSS_INPUT_IS_FILE 2
#define LZSS_OUTPUT_IS_MEMORY 4
#define LZSS_OUTPUT_IS_FILE 8
#define LZSS_PREDICT_SIZE 16

#if !defined(_FILE_OFFSET_BITS)
#	define _FILE_OFFSET_BITS 64
#endif

#if (defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS != 64)
#	undefine _FILE_OFFSET_BITS
#	define _FILE_OFFSET_BITS 64
#endif

struct lzss_t {
	char * ptr;
	size_t size;
	size_t offset;
};

#include <stdint.h>

char lzss_decode_ff(FILE * infile, FILE * outfile);
void lzss_decode_fm(FILE * infile, struct lzss_t * result);
void lzss_decode_mf(struct lzss_t * input, FILE * outfile);
void lzss_decode_mm(struct lzss_t * input, struct lzss_t * result);

char lzss_encode_ff(FILE * infile, FILE * outfile);

uint64_t lzss_predict_decomp_size_f(FILE * infile);
uint64_t lzss_predict_decomp_size_m(struct lzss_t * input);
