#include "misc_logger_update_signal.h"
#include "logger.h"

void misc_logger_update_signal_exec(
    const misc_logger_update_signal_inputs_t *i,
    misc_logger_update_signal_outputs_t *o
)
{
    // TODO add string parameters in c-atom types
    Logger_UpdateSignal("nav", "wx", i->input);
    o->output = 0;
}

