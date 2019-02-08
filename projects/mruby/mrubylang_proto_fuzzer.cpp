#include <string>
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
	mrb_load_string(mrb, code);
	mrb_close(mrb);
	return 0;
}

DEFINE_PROTO_FUZZER(const Function &function) {
	auto s = ProtoToRb(function);
	(void)FuzzRB((const uint8_t*)s.data(), s.size());
}