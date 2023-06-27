/* SPDX-License-Identifier: BSD-3-Clause */

#ifndef _HELPERS_H
#define _HELPERS_H

#include "../util/parser/parser.h"


int redirect_out(simple_command_t *s, int flags);
int redirect_err(simple_command_t *s, int flags);
int redirect_in(simple_command_t *s);
int redirects(simple_command_t *s);
int redirect_output_cd(simple_command_t *s);


#endif /* _HELPERS_H */
