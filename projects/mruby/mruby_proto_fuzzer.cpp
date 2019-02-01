#include <string>
#include <sstream>
#include <fstream>
#include <zlib.h>  // for crc32

#include <mruby.h>
#include <mruby/irep.h>

#include "libprotobuf-mutator/src/libfuzzer/libfuzzer_macro.h"
#include "mruby_bytecode.pb.h"

#include <limits.h>
#include <stdint.h>
#include <stddef.h>

static void WriteInt(std::stringstream &out, uint32_t x) {
	x = __builtin_bswap32(x);
	out.write((char *)&x, sizeof(x));
}

static void WriteShort(std::stringstream &out, uint16_t x) {
	x = __builtin_bswap16(x);
	out.write((char *)&x, sizeof(x));
}

static void WriteByte(std::stringstream &out, uint8_t x) {
	out.write((char *)&x, sizeof(x));
}

static std::string Compress(const std::string &s) {
	std::string out(s.size() + 100, '\0');
	size_t out_len = out.size();
	compress((uint8_t *)&out[0], &out_len, (uint8_t *)s.data(), s.size());
	out.resize(out_len);
	return out;
}

// Chunk is written as:
//  * 4-byte length
//  * 4-byte type
//  * the data itself
//  * 4-byte crc (of type and data)
static void WriteChunk(std::stringstream &out, const char *type,
                       const std::string &chunk, bool compress = false) {
	std::string compressed;
	const std::string *s = &chunk;
	if (compress) {
		compressed = Compress(chunk);
		s = &compressed;
	}
	uint32_t len = s->size();
	uint32_t crc = crc32(crc32(0, (const unsigned char *)type, 4),
	                     (const unsigned char *)s->data(), s->size());
	WriteInt(out, len);
	out.write(type, 4);
	out.write(s->data(), s->size());
	WriteInt(out, crc);
}

/* Calculate CRC (CRC-16-CCITT)
**
**  0000_0000_0000_0000_0000_0000_0000_0000
**          ^|------- CRC -------|- work --|
**        carry
*/
#define  CRC_16_CCITT       0x11021ul        /* x^16+x^12+x^5+1 */
#define  CRC_XOR_PATTERN    (CRC_16_CCITT << 8)
#define  CRC_CARRY_BIT      (0x01000000)

uint16_t
calc_crc_16_ccitt(const uint8_t *src, size_t nbytes, uint16_t crc)
{
	size_t ibyte;
	uint32_t ibit;
	uint32_t crcwk = crc << 8;

	for (ibyte = 0; ibyte < nbytes; ibyte++) {
		crcwk |= *src++;
		for (ibit = 0; ibit < CHAR_BIT; ibit++) {
			crcwk <<= 1;
			if (crcwk & CRC_CARRY_BIT) {
				crcwk ^= CRC_XOR_PATTERN;
			}
		}
	}
	return (uint16_t)(crcwk >> 8);
}


std::string ProtoToMrb(const MrbProto &mrb_proto) {

//      struct rite_binary_header {
//		uint8_t binary_ident[4];    /* Binary Identifier */
//		uint8_t binary_version[4];  /* Binary Format Version */
//		uint8_t binary_crc[2];      /* Binary CRC */
//		uint8_t binary_size[4];     /* Binary Size */
//		uint8_t compiler_name[4];   /* Compiler name */ /* MATZ */
//		uint8_t compiler_version[4]; /* 0000
//	  };

	std::stringstream all;
	const unsigned char header[] = {0x52, 0x49, 0x54, 0x45, 0x30, 0x30, 0x30, 0x36};
	all.write((const char*)header, sizeof(header));

	std::stringstream pay_str;
	auto &data = mrb_proto.data();

	// Write 2-byte CRC and 4-byte bin size followed by
	// meta-data (compiler name/version)
	/* We write int to a temp stringstream, we then write temp string stream
	 * to permanent string stream
	 */
	uint16_t crc = calc_crc_16_ccitt((const uint8_t *)data.data(), data.size(), 0);
	WriteShort(pay_str, crc);
    WriteInt(pay_str, data.size());
	// Write CRC+ bin_size into permanent stream
	all.write(pay_str.str().data(), 6);
	// Write compiler name/version into permanent stream
	all.write("MATZ0000", 8);
	// Write protobuf opaque byte stream
	all.write(data.data(), data.size());

	std::string res = all.str();
	if (const char *dump_path = getenv("PROTO_FUZZER_DUMP_PATH")) {
		// With libFuzzer binary run this to generate an MRB file x.mrb:
		// PROTO_FUZZER_DUMP_PATH=x.mrb ./a.out proto-input
		std::ofstream of(dump_path);
		of.write(res.data(), res.size());
	}
	return res;
}

int FuzzMRB(const uint8_t *Data, size_t size) {
	mrb_state *mrb = mrb_open();
	if (!mrb)
		return 0;
	mrb_load_irep(mrb, Data);
	mrb_close(mrb);
	return 0;
}

DEFINE_PROTO_FUZZER(const MrbProto &mrb_proto) {
	auto s = ProtoToMrb(mrb_proto);
	(void)FuzzMRB((const uint8_t*)s.data(), s.size());
}