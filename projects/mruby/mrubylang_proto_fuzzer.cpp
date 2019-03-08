#include <string>
#include <iostream>
#include <fstream>

#include <mruby.h>
#include <mruby/compile.h>

#include "libprotobuf-mutator/src/libfuzzer/libfuzzer_macro.h"
#include "ruby.pb.h"
#include "proto_to_ruby.h"

using namespace ruby_fuzzer;
using namespace std;

static size_t totalRuns = 0;
static size_t failedRuns = 0;

int FuzzRB(const uint8_t *Data, size_t size) {
	mrb_value v;
	mrb_state *mrb = mrb_open();
	if (!mrb)
		return 0;

	char *code = (char *)malloc(size+1);
	if (!code)
		return 0;
	memcpy(code, Data, size);
	code[size] = '\0';

	if (const char *dump_path = getenv("PROTO_FUZZER_DUMP_PATH")) {
		// With libFuzzer binary run this to generate an RB file x.rb:
		// PROTO_FUZZER_DUMP_PATH=x.rb ./a.out proto-input
		std::ofstream of(dump_path);
		of.write(code, size);
	}

	totalRuns++;
	v = mrb_load_string(mrb, code);
	if (mrb->exc) {
		failedRuns++;
		cout << "--------------" << endl;
		cout << "Faulty program" << endl;
		cout << code << endl;
		cout << "Error" << endl;
		mrb_print_error(mrb);
		cout << "--------------" << endl;
		cerr << "Pass percentage: " << (100 - (failedRuns*100/totalRuns)) << endl;
	}
	mrb_close(mrb);

	free(code);
	return 0;
}

DEFINE_PROTO_FUZZER(const Function &function) {
	auto s = FunctionToString(function);
	(void)FuzzRB((const uint8_t*)s.data(), s.size());
}