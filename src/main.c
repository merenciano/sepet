#include <sepet/sepet.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    spt_context ctx = spt_mkctx();

    spt_add(&ctx, SPT_FIELD_BUILDING,   "Casa");
    spt_add(&ctx, SPT_FIELD_ROOM,       "Dormitorio");
    spt_add(&ctx, SPT_FIELD_ROOM,       "Cocina");
    spt_add(&ctx, SPT_FIELD_CONTAINER,  "Mesita");
    spt_add(&ctx, SPT_FIELD_CONTAINER,  "MesaAuxiliar");
    uint8_t subsec = spt_add(&ctx, SPT_FIELD_SUBSECTION, "Cagarro");
    spt_add(&ctx, SPT_FIELD_SUBSECTION, "Estante1");
    spt_add(&ctx, SPT_FIELD_SUBSECTION, "Estante2");
    spt_add(&ctx, SPT_FIELD_SUBSECTION, "Superficie");
    spt_add(&ctx, SPT_FIELD_ITEM,  "Condones");
    spt_add(&ctx, SPT_FIELD_ITEM,  "LlavesDenia");
    spt_add(&ctx, SPT_FIELD_ITEM,  "EBook");
    spt_add(&ctx, SPT_FIELD_ITEM,  "Pasaporte");
    spt_add(&ctx, SPT_FIELD_ITEM,  "Microondas");
    spt_add(&ctx, SPT_FIELD_ITEM,  "OllaPresion");
    spt_add(&ctx, SPT_FIELD_ITEM,  "Cafetera");

    spt_rename(&ctx, SPT_FIELD_SUBSECTION, subsec, "Cajon1");

    printf("\n--- User-defined aliases ---\n");
    printf("\nBuildings:\n");
    for (spt_name *name = ctx.building_names; name->buf[0]; ++name) {
        printf("\t%ld: %s\n", name - ctx.building_names, (char*)name);
    }

    printf("\nRooms:\n");
    for (spt_name *name = ctx.room_names; name->buf[0]; ++name) {
        printf("\t%ld: %s\n", name - ctx.room_names, (char*)name);
    }

    printf("\nContainers:\n");
    for (spt_name *name = ctx.container_names; name->buf[0]; ++name) {
        printf("\t%ld: %s\n", name - ctx.container_names, (char*)name);
    }

    printf("\nSubsections:\n");
    for (spt_name *name = ctx.subsec_names; name->buf[0]; ++name) {
        printf("\t%ld: %s\n", name - ctx.subsec_names, (char*)name);
    }

    printf("\nItems:\n");
    for (spt_name *name = ctx.item_names; name->buf[0]; ++name) {
        printf("\t%ld: %s\n", name - ctx.item_names, (char*)name);
    }

    printf("--------------------------------\n");

    spt_entry passport = spt_insert(
            &ctx,
            spt_get_id(&ctx, SPT_FIELD_ITEM, "Pasaporte"),
            spt_get_id(&ctx, SPT_FIELD_BUILDING, "Casa"),
            spt_get_id(&ctx, SPT_FIELD_ROOM, "Dormitorio"),
            spt_get_id(&ctx, SPT_FIELD_CONTAINER, "Mesita"),
            spt_get_id(&ctx, SPT_FIELD_SUBSECTION, "Cajon1"));

    spt_insert(&ctx,
               spt_get_id(&ctx, SPT_FIELD_ITEM, "Microondas"),
               spt_get_id(&ctx, SPT_FIELD_BUILDING, "Casa"),
               spt_get_id(&ctx, SPT_FIELD_ROOM, "Cocina"),
               spt_get_id(&ctx, SPT_FIELD_CONTAINER, "MesaAuxiliar"),
               spt_get_id(&ctx, SPT_FIELD_SUBSECTION, "Estante1"));

    spt_insert(&ctx,
               spt_get_id(&ctx, SPT_FIELD_ITEM, "Cafetera"),
               spt_get_id(&ctx, SPT_FIELD_BUILDING, "Casa"),
               spt_get_id(&ctx, SPT_FIELD_ROOM, "Cocina"),
               spt_get_id(&ctx, SPT_FIELD_CONTAINER, "MesaAuxiliar"),
               spt_get_id(&ctx, SPT_FIELD_SUBSECTION, "Superficie"));

    printf("\n--- Inventory entries ---\n");
    for (spt_entry *entry = ctx.entries; *(uint64_t*)entry; ++entry) {
       printf("\t%ld: b: %d - r: %d - c: %d - s: %d - i: %d\n",
             entry - ctx.entries, entry->building, entry->room,
             entry->container, entry->subsection, entry->item);
    }

    /* Extracting passport from inventory. */
    spt_extract(&ctx, passport);

    printf("\n--- Inventory entries ---\n");
    for (spt_entry *entry = ctx.entries; *(uint64_t*)entry; ++entry) {
        if (!entry->building) continue;
        printf("\t%ld: b: %d - r: %d - c: %d - s: %d - i: %d\n",
               entry - ctx.entries, entry->building, entry->room,
               entry->container, entry->subsection, entry->item);
    }

    spt_delete(&ctx, SPT_FIELD_ROOM, spt_get_id(&ctx, SPT_FIELD_ROOM, "Dormitorio"));

    printf("\n--- Inventory entries ---\n");
    for (spt_entry *entry = ctx.entries; *(uint64_t*)entry; ++entry) {
        if (!entry->building) continue;
        printf("\t%ld: b: %d - r: %d - c: %d - s: %d - i: %d\n",
               entry - ctx.entries, entry->building, entry->room,
               entry->container, entry->subsection, entry->item);
    }

    printf("\nRooms:\n");
    for (spt_name *name = ctx.room_names; name->buf[0]; ++name) {
        printf("\t%ld: %s\n", name - ctx.room_names, (char*)name);
    }

    return ctx.out_error;
}
