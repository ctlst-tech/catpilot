#include "cube_misc_cli_logger.h"

void CLI_write_stream_string(const char *str);


void cube_misc_cli_logger_exec(const cube_misc_cli_logger_inputs_t *i){
    static char str2post[256];
#define VAR_FMT "%+.3lf "
    snprintf(str2post, 255,
             VAR_FMT
             VAR_FMT VAR_FMT VAR_FMT " || " VAR_FMT VAR_FMT VAR_FMT " || " \
             VAR_FMT VAR_FMT VAR_FMT " || " VAR_FMT VAR_FMT VAR_FMT "\n\r",
             xTaskGetTickCount() / 1000.f,
             i->v1, i->v2, i->v3, i->v4, i->v5, i->v6,
             i->v7, i->v8, i->v9, i->v10, i->v11, i->v12
    );

    CLI_write_stream_string(str2post);
}
