#include <mruby.h>
#include <mruby/irep.h>

int LLVMFuzzerTestOneInput(uint8_t *Data, size_t size) {

    // There are 22 bytes in RiteVM binary header
    if (size < 22) return 0;
    mrb_state *mrb = mrb_open();
    if (!mrb)
        return 0;
    mrb_load_irep(mrb, Data);
    mrb_close(mrb);
    return 0;
}
