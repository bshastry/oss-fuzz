#include <string>
#include <fstream>

#include <mruby.h>
#include <mruby/compile.h>

#include "libprotobuf-mutator/src/libfuzzer/libfuzzer_macro.h"
#include "ruby.pb.h"
#include "proto_to_ruby.h"

using namespace ruby_fuzzer;

int FuzzRB(const uint8_t *Data, size_t size) {
	mrb_state *mrb = mrb_open();
	if (!mrb)
		return 0;

	char *code = (char *)malloc(size+1);
	if (!code)
		return 0;
	memcpy(code, Data, size);
	code[size] = '\0';

	mrb_load_string(mrb, code);
	mrb_close(mrb);

	if (const char *dump_path = getenv("PROTO_FUZZER_DUMP_PATH")) {
		// With libFuzzer binary run this to generate an RB file x.rb:
		// PROTO_FUZZER_DUMP_PATH=x.rb ./a.out proto-input
		std::ofstream of(dump_path);
		of.write(code, size);
	}

	return 0;
}

DEFINE_PROTO_FUZZER(const Function &function) {
	auto s = FunctionToString(function);
	(void)FuzzRB((const uint8_t*)s.data(), s.size());
}