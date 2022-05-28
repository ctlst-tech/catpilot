#include "ph_logger_update_signal.h"

extern int Logger_UpdateSignal(char *frame_name, char *signal_name, double value);

void ph_logger_update_signal_exec(const ph_logger_update_signal_inputs_t *i, ph_logger_update_signal_outputs_t *o)
{
    // TODO add string parameters in c-atom types
    Logger_UpdateSignal("nav", "wx", i->input);
    o->output = 0;
}
