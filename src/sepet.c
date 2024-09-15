#include "sepet.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

/* Empty macro argument placeholder. */
#define _SPT_ARG_PH

/* Max index value depending on field type. */
#define _SPT_MAX_IDX(FLD) \
    ((FLD) == SPT_FIELD_ITEM ? SPT_MAX_ITEMS : SPT_MAX_FIELDS)

/* Macro capturing the error handling boilerplate code. */
#define _SPT_ERR(CTX, ERR, FMT, ...) do {                                      \
    (CTX)->out_error = (ERR);                                                  \
    snprintf((CTX)->out_err_msg, SPT_MSG_SIZE, FMT, __VA_ARGS__);       \
    if ((CTX)->err_callback) {                                                 \
        (CTX)->err_callback((CTX)->usr_data,                                   \
                (CTX)->out_error, (CTX)->out_err_msg);                         \
    }} while (0)

/* Field arg value check. */
#define _SPT_CHECK_FIELD(CTX, FLD, RET) do {                                   \
    if ((FLD) < 0 || (FLD) >= SPT_FIELD_COUNT) {                               \
        _SPT_ERR((CTX), SPT_ERROR_BOUNDS,                                      \
                "Field (%d) out of bounds. Range: [0, %d)",                    \
                (FLD), SPT_FIELD_COUNT);                                       \
        return RET;                                                            \
    }} while (0)

/* Element index arg value check, */
#define _SPT_CHECK_ELEM(CTX, FLD, ELEM, RET) do {                              \
    if ((ELEM) <= SPT_DEFAULT_ID || (ELEM) >= _SPT_MAX_IDX((FLD))) {                         \
        _SPT_ERR((CTX), SPT_ERROR_BOUNDS,                                      \
                "Element (%d) out of bounds. Range: [0, %d)",                  \
                (ELEM), _SPT_MAX_IDX((FLD)));                                  \
        return RET;                                                            \
    }} while (0)

/* Arg context checking. Done at the beginning of every function. */
#define _SPT_CHECK_CTX(CTX, RET) do {if (!(CTX)) {return RET;}} while (0)

/* 'enum spt_fields' to string (enum values and this indices have to match). */
static const char* SPT_FIELD_NAMES[SPT_FIELD_COUNT] = {
    "Building",
    "Room",
    "Container",
    "Subsection",
    "Item"
};

/* Check if name is empty string, i.e. buf[0] == '\0'. */
static int _spt_empty(spt_name *n) { return !*n->buf; }

/* Clears contents of a name. The cleared name is guaranteed to be empty. */
static void _spt_clear(spt_name *n) { *n->buf = 0; }

spt_context *
spt_reset(spt_context *ctx)
{
    _SPT_CHECK_CTX(ctx, ctx);
    /* Name arrays rely on first char being 0 on unused ones. */
    memset(ctx, 0, sizeof(*ctx));

    ctx->building_names [SPT_UNSPECIFIED_ID] = (spt_name){"Unspecified"};
    ctx->room_names     [SPT_UNSPECIFIED_ID] = (spt_name){"Unspecified"};
    ctx->container_names[SPT_UNSPECIFIED_ID] = (spt_name){"Unspecified"};
    ctx->subsec_names   [SPT_UNSPECIFIED_ID] = (spt_name){"Unspecified"};
    ctx->item_names     [SPT_UNSPECIFIED_ID] = (spt_name){"Unspecified"};
    ctx->building_names [SPT_DEFAULT_ID]     = (spt_name){"Default"};
    ctx->room_names     [SPT_DEFAULT_ID]     = (spt_name){"Default"};
    ctx->container_names[SPT_DEFAULT_ID]     = (spt_name){"Default"};
    ctx->subsec_names   [SPT_DEFAULT_ID]     = (spt_name){"Default"};
    ctx->item_names     [SPT_DEFAULT_ID]     = (spt_name){"Default"};

    ctx->out_error = SPT_SUCCESS;
    return ctx;
}

uint32_t
spt_add(spt_context *ctx, int field, const char *alias)
{
    /* Context and field checks. */
    _SPT_CHECK_CTX(ctx, SPT_INVALID_ID);
    _SPT_CHECK_FIELD(ctx, field, SPT_INVALID_ID);

    const int maxidx = field == SPT_FIELD_ITEM ? SPT_MAX_ITEMS : SPT_MAX_FIELDS;
    /* Get a char ptr pointing at the first empty alias
     * element and assume it's really unused. */
    spt_name *name = ctx->building_names + field * SPT_MAX_FIELDS;
    int idx = 0;
    for (; idx < maxidx && !_spt_empty(name + idx); ++idx);

    /* Check if field's names buffer is full. */
    if (idx >= maxidx) {
        _SPT_ERR(ctx, SPT_ERROR_MEMORY,
                "%s aliases array reached its limit of %d elements.",
                SPT_FIELD_NAMES[field], maxidx);
        return SPT_INVALID_ID;
    }

    /* Set name data and return the element index. */
    if (alias && *alias) {
        /* Alias has data: copy it into name. */
        strncpy(name[idx].buf, alias, SPT_NAME_SIZE);
    } else {
        /* Alias is NULL or empty: set the internal index as ascii. */
        snprintf(name[idx].buf, SPT_NAME_SIZE, "%d", idx);
    }

    ctx->out_error = SPT_SUCCESS;
    return idx;
}

spt_entry
spt_insert(spt_context *ctx, uint32_t item, uint8_t building, uint8_t room,
           uint8_t container, uint8_t subsec)
{
    _SPT_CHECK_CTX(ctx, (spt_entry){0});
    _SPT_CHECK_ELEM(ctx, SPT_FIELD_ITEM, item, (spt_entry){-1L});

    spt_entry e = {.building = building, .room = room, .container = container,
                   .subsection = subsec, .item = item};
    int i = 0;
    for (;i < SPT_MAX_ITEMS && ctx->entries[i].item; ++i);
    ctx->entries[i] = e;
    ctx->out_error = SPT_SUCCESS;

    return e;
}

uint32_t
spt_extract(spt_context *ctx, spt_entry entry)
{
    _SPT_CHECK_CTX(ctx, -1);
    _SPT_CHECK_ELEM(ctx, SPT_FIELD_ITEM, entry.item, -1);

    for (int i = 0; i < SPT_MAX_ITEMS; ++i) {
        if (ctx->entries[i].item == entry.item) {
            ctx->entries[i].building = SPT_UNSPECIFIED_ID;
            ctx->entries[i].room = SPT_UNSPECIFIED_ID;
            ctx->entries[i].container = SPT_UNSPECIFIED_ID;
            ctx->entries[i].subsection = SPT_UNSPECIFIED_ID;
            ctx->out_error = SPT_SUCCESS;
            return entry.item;
        }
    }

    _SPT_ERR(ctx, SPT_ERROR_NOT_FOUND, "ItemID: %d.\n"
            "The specified item could not be found in the entry list.",
            entry.item);
    return -1;
}

void
spt_rename(spt_context *ctx, int field, uint32_t element, const char *new_name)
{
    _SPT_CHECK_CTX(ctx, _SPT_ARG_PH);
    _SPT_CHECK_FIELD(ctx, field, _SPT_ARG_PH);
    spt_name *name = ctx->building_names + field * SPT_MAX_FIELDS;

    /* Check if field:element is active i.e., created. */
    if (_spt_empty(name + element)) {
        _SPT_ERR(ctx, SPT_ERROR_STATE,
                "Trying to rename an uninit element, field:%d, id:%d, name().",
                field, element);
        return;
    }

    if (new_name && *new_name) {
        /* New name has data: copy it into name. */
        strncpy(name[element].buf, new_name, SPT_NAME_SIZE);
    } else {
        /* New name is NULL or empty: set the internal index as ascii. */
        snprintf(name[element].buf, SPT_NAME_SIZE, "%d", element);
    }

    ctx->out_error = SPT_SUCCESS;
}

void
spt_delete(spt_context *ctx, int field, uint32_t element)
{
    _SPT_CHECK_CTX(ctx, _SPT_ARG_PH);
    _SPT_CHECK_FIELD(ctx, field, _SPT_ARG_PH);
    _SPT_CHECK_ELEM(ctx, field, element, _SPT_ARG_PH);

    spt_name *name = ctx->building_names + field * SPT_MAX_FIELDS;
    /* TODO: Remove this useless branch. */
    if (_spt_empty(name + element)) {
        ctx->out_error = SPT_NOOP;
        return;
    }

    _spt_clear(name + element);
    ctx->out_error = SPT_SUCCESS;

    /* TODO: entries pass extracting the ones who reference the deleted elem */
}
