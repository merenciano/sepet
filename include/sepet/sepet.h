#ifndef __SEPET_H__
#define __SEPET_H__

#include <stdint.h>

#define SPT_INVALID_ID        (-1)        /* Error code, app state invalid. */
#define SPT_UNSPECIFIED_ID    (0)         /* Explicit value for unstored or currently unavailable items. */
#define SPT_DEFAULT_ID        (1)         /* Unspecified but valid as storage, e.g. subsection value for tiny containers. */
#define SPT_NAME_SIZE         (32)        /* Maximum number of characters per name. */
#define SPT_MSG_SIZE          (2048)      /* Output error message buffer size. */
#define SPT_MAX_FIELDS        (256)       /* Maximum number of unique storage fields for each type. */
#define SPT_MAX_ITEMS         (1U << 13)  /* Maximum number of unique items. */

/* Error callback signature. Will be called in case of error if provided (see: spt_context::err_callback). */
typedef void (*ErrCb)(void *usrdata, int errcode, const char *errmsg);

/* Error code definitions. Do not expect this enum to be used as a type (codes are stored in 'int' variables when used). */
enum spt_error_codes {
    SPT_SUCCESS         = 1,    /* Last API call returned successfuly. */
    SPT_NOOP            = 0,    /* Last API call was ignored, skipped or did not change anything for benign reasons. */
    SPT_ERROR_CTX       = -100, /* Invalid context usually caused by NULL ptr. */
    SPT_ERROR_BOUNDS    = -110, /* Identifier value outside valid range. */
    SPT_ERROR_MEMORY    = -120, /* Memory limit reached for the requested field. Consider increasing the configured sizes. */
    SPT_ERROR_STATE     = -130, /* Incompatibilities between state and calls, like trying to rename disabled items. */
    SPT_ERROR_NOT_FOUND = -140, /* Could not find the specified element in the entry list. */
};

/* Item and storage field identifiers. Do not expect this enum to be used as a type (codes are stored in 'int' variables when used). */
enum spt_fields {
    SPT_FIELD_BUILDING = 0,
    SPT_FIELD_ROOM,
    SPT_FIELD_CONTAINER,
    SPT_FIELD_SUBSECTION,
    SPT_FIELD_ITEM,
    SPT_FIELD_COUNT
};

/* char[] encapsulation for cleaner code: pointer arithmetic and operator '=' */
typedef struct spt_name {
    char buf[SPT_NAME_SIZE];
} spt_name;

/* Stored item ID. It encodes the item identifier and its stored location. */
typedef struct spt_entry {
    uint8_t     building;   /* Building ID. */
    uint8_t     room;       /* Room ID. */
    uint8_t     container;  /* Container ID. */
    uint8_t     subsection; /* Subsection ID. */
    uint32_t    item;       /* Item ID. */
} spt_entry;

/* Application context (or environment, instance, ...), it contains all the application state. */
typedef struct spt_context {
    /* Persistent data (inventory state). */
    spt_name    building_names  [SPT_MAX_FIELDS];   /* User-defined building aliases, used to describe large contiguous spaces, e.g. "Home", "Parents'", "Workplace", ... */
    spt_name    room_names      [SPT_MAX_FIELDS];   /* User-defined room aliases, used to delimit area units, e.g. "Bedroom", "Garage", "Attic", ... */
    spt_name    container_names [SPT_MAX_FIELDS];   /* User-defined container aliases for listing the storage units of the room, e.g. "Desk", "Shelves", "Suitcase", ... */
    spt_name    subsec_names    [SPT_MAX_FIELDS];   /* User-defined subsection aliases, wildcard to refer to compartments and other final locations, e.g. "TopShelf", "Drawer_3", "OutsidePocket", ... */
    spt_name    item_names      [SPT_MAX_ITEMS];    /* List of user-defined item aliases. */
    spt_entry   entries         [SPT_MAX_ITEMS];    /* References to every single stored item and its location info. */

    /* Runtime utils. */
    int         out_error;          /* Out error code (from: enum spt_error_codes). This value is overwritten on every API call. */
    char        out_err_msg[SPT_MSG_SIZE];  /* Out error message. Null-terminated string containing more specific details of the error ocurred, in contrast with out_error which is a generic code. This buffer written only when errors occur. */
    ErrCb       err_callback;       /* User-defined callback function. If not NULL */
    void       *usr_data;           /* User-defined pointer to data. It will not be used by this library except for providing it in user callbacks. */
} spt_context;

/**
 * @brief Initialize / reset a spt instance with default values.
 * @note Any previous data will be erased.
 * @note Read ctx->out_error code (see: enum spt_error_codes) for error checking after this call.
 * @param ctx SPT instance.
 */
spt_context *spt_reset(
    spt_context *ctx);

/**
 * @brief Restore context state from a previous shapshot.
 * Since this lib is so simple, all the data fits in a statically allocated
 * struct that can be mapped to memory in a single block.
 * The current configuration allows for ~10k items and ~1k storage location
 * identifiers, every one of them having storage for the user-defined naming
 * string, has a total of ~350KB.
 * @note Read ctx->out_error code (see: enum spt_error_codes) for error
 * checking after this call.
 * @param ctx SPT instance to load the data on.
 * @param blob Chunk of binary data formatted like spt_context struct.
 */
void spt_load(
    spt_context *ctx,
    void        *blob);

/** 
 * @brief Adds a new item or storage field to the database.
 * @note Read ctx->out_error code (see: enum spt_error_codes) for error
 * checking after this call.
 * @param ctx SPT instance.
 * @param field Field identifier (see: enum spt_fields).
 * @param alias User-defined name for the new element.
 * @return Generated identifier.
 */
uint32_t spt_add(
    spt_context *ctx,
    int          field,
    const char  *name);

/** 
 * @brief Rename an existing element.
 * Does not cause invalidations or side effects to worry about since the
 * internal ID remains unchanged.
 * @note Read ctx->out_error code (see: enum spt_error_codes) for error
 * checking after this call.
 * @param ctx SPT instance.
 * @param field Field identifier (see: enum spt_fields).
 * @param element Identifier of the renaming target.
 * @param new_alias New name.
 */
void spt_rename(
    spt_context *ctx,
    int          field,
    uint32_t     element,
    const char  *new_name);

/**
 * @brief Stores an item in a specified location.
 * @note If the item was already stored, it is extracted from its previous
 * location before the insertion.
 * @note Read ctx->out_error code (see: enum spt_error_codes) for error
 * checking after this call.
 * @param ctx SPT instance.
 * @param item Identifier of the item being inserted.
 * @param building Identifier of the desired building.
 * @param room Identifier of the desired room.
 * @param container Identifier of the desired container.
 * @param subsection Identifier of the desired subsection.
 * @return Resulting entry.
 */
spt_entry spt_insert(
    spt_context *ctx,
    uint32_t     item,
    uint8_t      building,
    uint8_t      room,
    uint8_t      container,
    uint8_t      subsection);

/**
 * @brief Extract an item from its storage location.
 * The item is left in an unspecified state.
 * @note Read ctx->out_error code (see: enum spt_error_codes) for error
 * checking after this call.
 * @param ctx SPT instance.
 * @param entry Stored item to retrieve.
 * @return Item ID.
 */
uint32_t spt_extract(
    spt_context *ctx,
    spt_entry    entry);

/**
 * @brief Delete the specified element from the database.
 * Reset the user-defined name and the generated ID.
 * Any dangling references (like contained items if the element was a storage
 * field or active entry if it was a stored item) will be left to an
 * unspecified state.
 * @note Read ctx->out_error code (see: enum spt_error_codes) for error
 * checking after this call.
 * @param ctx SPT instance.
 * @param field Field identifier from 'spt_fields'.
 * @param element Item or storage to delete.
 */
void spt_delete(
    spt_context *ctx,
    int          field,
    uint32_t     element);

/**
 * @brief SPT instance generator.
 * This spt_context instances can be created and managed by anyone.
 * The only reason this function exist is for QoL when interfacing with other
 * programming languages or frameworks.
 * @return Generated instance default-initialized.
 * TODO: More in-depth checks regarding the possible copy of the instance.
 * The memory location of the instance is not the same but it's hard to tell
 * how is the OS copying the memory. I'm not able to spot any memory movement
 * when looking at the release disassembly. Maybe it's implemented using
 * virtual memory pager tricks? Since it is moving ~400KB of sequencial
 * data from a temp object to a lvalue...
 */
static inline spt_context spt_mkctx() { return *spt_reset(&(spt_context){}); }

#endif // __SEPET_H__
