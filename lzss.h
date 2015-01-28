#define LZSS_INPUT_IS_MEMORY 1
#define LZSS_INPUT_IS_FILE 2
#define LZSS_OUTPUT_IS_MEMORY 4
#define LZSS_OUTPUT_IS_FILE 8
#define LZSS_PREDICT_SIZE 16

#define DEFAULT_EI 11  /* typically 10..13 */
#define DEFAULT_EJ  4  /* typically 4..5 */
#define DEFAULT_P   1  /* If match length <= P then output one character */

#if !defined(_FILE_OFFSET_BITS)
#	define _FILE_OFFSET_BITS 64
#endif

#if (defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS != 64)
#	undefine _FILE_OFFSET_BITS
#	define _FILE_OFFSET_BITS 64
#endif

#include <stdint.h>

struct lzss_settings {
	uint8_t EI;
	uint8_t EJ;
	uint8_t P;
};

struct lzss_t {
	char * ptr;
	size_t size;
	size_t offset;
};

void lzss_decode_ff(FILE * infile, FILE * outfile, struct lzss_settings * settings);
void lzss_decode_fm(FILE * infile, struct lzss_t * result, struct lzss_settings * settings);
void lzss_decode_mf(struct lzss_t * input, FILE * outfile, struct lzss_settings * settings);
void lzss_decode_mm(struct lzss_t * input, struct lzss_t * result, struct lzss_settings * settings);

uint64_t lzss_encode_ff(FILE * infile, FILE * outfile, struct lzss_settings * settings);
void lzss_encode_fm(FILE * infile, struct lzss_t * result, struct lzss_settings * settings);
void lzss_encode_mf(struct lzss_t * input, FILE * outfile, struct lzss_settings * settings);
void lzss_encode_mm(struct lzss_t * input, struct lzss_t * result, struct lzss_settings * settings);

uint64_t lzss_predict_decomp_size_f(FILE * infile);
uint64_t lzss_predict_decomp_size_m(struct lzss_t * input);
uint64_t lzss_predict_comp_size_f(FILE * infile);
uint64_t lzss_predict_comp_size_m(struct lzss_t * input);
