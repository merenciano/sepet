#include <sepet/sepet.h>

int main(int argc, char **argv)
{
    spt_context ctx = spt_mkctx();
    return ctx.out_error;
}
