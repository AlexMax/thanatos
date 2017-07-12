#include "duktape.h"

namespace script
{

void Init()
{
    duk_context *duk = duk_create_heap_default();
    duk_destroy_heap(duk);

    return;
}

}
