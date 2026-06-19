#ifndef HERMES_CHECKPOINT_IO_H
#define HERMES_CHECKPOINT_IO_H

#include "hermes_checkpoint.h"
#include <stddef.h>

int hermes_checkpoint_io_init(const char *storage_path);
int hermes_checkpoint_io_save(const hermes_checkpoint_t *cp);
int hermes_checkpoint_io_load(const char *app_id, hermes_checkpoint_t *out);
int hermes_checkpoint_io_delete(const char *app_id);

#endif
