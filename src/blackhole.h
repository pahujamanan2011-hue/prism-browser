#ifndef PRISM_BLACKHOLE_H
#define PRISM_BLACKHOLE_H

#include <stdbool.h>

bool blackhole_init(void);
void blackhole_cleanup(void);
bool blackhole_is_blocked(const char* url);
int blackhole_get_block_count(void);
bool blackhole_add_domain(const char* domain);
bool blackhole_remove_domain(const char* domain);
void blackhole_refresh(void);

#endif /* PRISM_BLACKHOLE_H */