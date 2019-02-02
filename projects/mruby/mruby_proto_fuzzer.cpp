#include <string>
#include <sstream>
#include <fstream>

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

	/* This is a bit tricky
	 * First, we need to compute the checksum over the payload
	 * The payload comprises
	 *   - 4 byte bin_size
	 *   - 8 bytes of compiler name ("MATZ") and version ("0000")
	 *   - variable length data
	 *   - 4 bytes string that signals end of data ("END\0")
	 * Furthermore, bin_size is the sum of
	 *   - header size (= 22 bytes)
	 *   - size of variable length data
	 *   - end string size (=4 bytes)
	*/

	/* First, we construct the payload. Later we compute checksum
	 * Next, we compute the checksum of the payload
	 * Finally, we write the crc followed by the payload into the fuzzed input
	 */

	WriteInt(pay_str, data.size() + 22 + 4);
	pay_str.write("MATZ0000", 8);
	std::string end_str = "END";
	size_t end_str_len = end_str.size() + 1;
	pay_str.write(data.data(), data.size());
	pay_str.write(end_str.c_str(), end_str_len);

	std::stringstream crc_str;
	uint16_t crc = calc_crc_16_ccitt((const uint8_t *)pay_str.str().data(), pay_str.str().size(), 0);
	WriteShort(crc_str, crc);

	// Write CRC+ payload
	all.write(crc_str.str().data(), 2);
	all.write(pay_str.str().data(), pay_str.str().size());

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
