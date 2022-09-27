#include "misc_logger_update_frame.h"
#include "logger.h"

void misc_logger_update_frame_exec(misc_logger_update_frame_outputs_t *o)
{
    // TODO add string parameters in c-atom types
    Logger_Start("nav", 2, 10);
    o->output = 0;
}

