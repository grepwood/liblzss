#define LZSS_INPUT_IS_MEMORY 1
#define LZSS_INPUT_IS_FILE 2
#define LZSS_OUTPUT_IS_MEMORY 4
#define LZSS_OUTPUT_IS_FILE 8
#define LZSS_PREDICT_SIZE 16

#define EI 11  /* typically 10..13 */
#define EJ  4  /* typically 4..5 */
#define P   1  /* If match length <= P then output one character */
#define N (1 << EI)  /* buffer size */
#define F ((1 << EJ) + P)  /* lookahead buffer size */

#if !defined(_FILE_OFFSET_BITS)
#	define _FILE_OFFSET_BITS 64
#endif

#if (defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS != 64)
#	undefine _FILE_OFFSET_BITS
#	define _FILE_OFFSET_BITS 64
#endif

#include <stdint.h>

struct lzss_t {
	char * ptr;
	size_t size;
	size_t offset;
};

void lzss_decode_ff(FILE * infile, FILE * outfile);
void lzss_decode_fm(FILE * infile, struct lzss_t * result);
void lzss_decode_mf(struct lzss_t * input, FILE * outfile);
void lzss_decode_mm(struct lzss_t * input, struct lzss_t * result);

void lzss_encode_ff(FILE * infile, FILE * outfile);
void lzss_encode_fm(FILE * infile, struct lzss_t * result);
void lzss_encode_mf(struct lzss_t * input, FILE * outfile);
void lzss_encode_mm(struct lzss_t * input, struct lzss_t * result);

uint64_t lzss_predict_decomp_size_f(FILE * infile);
uint64_t lzss_predict_decomp_size_m(struct lzss_t * input);
