#ifndef SEPET_H
#define SEPET_H

#include <stdint.h>

typedef enum {
    SPT_LOCATION_NONE = 0,
    SPT_LOCATION_BUILDING,
    SPT_LOCATION_ROOM,
    SPT_LOCATION_AREA,
    SPT_LOCATION_CONTAINER
} SPT_LOCATION_;

typedef int32_t spt_loc_t; // SPT_LOCATION_
typedef uint64_t spt_entry_t;

spt_entry_t spt_add(const char *name, const char *building, const char *room, const char *area, const char *container, void *env);
void spt_update(spt_entry_t entry, spt_loc_t location, const char *value, void *env);
void spt_rm(spt_entry_t entry, void *env);

#endif // SEPET_H

