#include <string.h>
#include <stdio.h>
#include <mruby.h>
#include <mruby/compile.h>

static size_t totalRuns = 0;
static size_t failedRuns = 0;

int LLVMFuzzerTestOneInput(uint8_t *Data, size_t size) {
    char *code = (char *)Data;
    if (size < 2 || Data[size-1] != '\0' || strlen(code) != size - 1) {
        return 0;
    }
    mrb_state *mrb = mrb_open();
    mrb_load_string(mrb, code);
    if (mrb->exc) {
		failedRuns++;
		fprintf(stdout, "--------------\n");
		fprintf(stdout, "Faulty program\n");
		fprintf(stdout, "%s\n", code);
		fprintf(stdout, "Error\n");
		mrb_print_error(mrb);
		fprintf(stdout, "--------------\n");
		fprintf(stderr, "Pass percentage: %lu\n", (100 - (failedRuns*100/totalRuns)));
    }
    mrb_close(mrb);
    return 0;
}
