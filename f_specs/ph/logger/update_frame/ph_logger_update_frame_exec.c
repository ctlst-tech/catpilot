#include "ph_logger_update_frame.h"

extern int Logger_Start(char *frame_name, int priority, int period);

void ph_logger_update_frame_exec(ph_logger_update_frame_outputs_t *o)
{
    // TODO add string parameters in c-atom types
    Logger_Start("nav", 2, 10);
    o->output = 0;
}
