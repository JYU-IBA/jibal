#ifndef _JIBAL_BOOTSTRAP_H_
#define _JIBAL_BOOTSTRAP_H_
#include <stdio.h>
#include <jibal_config.h>
#include <jibal_units.h>
char read_user_response(const char *question);
int bootstrap_write_user_config(const jibal_units *units, jibal_config *config);
void bootstrap_make_blanks(const char *user_dir, const char *filename);
#endif /* _JIBAL_BOOTSTRAP_H_ */
