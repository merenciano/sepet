#include <sepet.h>

struct entry_t {
    uint8_t building;
    uint8_t room;
    uint8_t area;
    uint8_t container;
    uint32_t item;
};

struct env_t {
    char buildings[32 * 256];
    char rooms[32 * 256];
    char areas[32 * 256];
    char containers[32 * 256];
    char *items;
    struct entry_t *entries;
    int32_t item_count;
    int32_t entry_count;
};

spt_entry_t
spt_add(const char* name, const char* building, const char* room, const char* area, const char* container, void *env)
{
    return 0;
}

void
spt_update(spt_entry_t entry, spt_loc_t location, const char* value, void *env)
{

}

void
spt_rm(spt_entry_t entry, void *env)
{

}

