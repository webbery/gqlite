#include "gqlite.h"

int main() {
    gqlite* handle;
    assert(ECode_Success == gqlite_open(&handle, "example"));
    assert(ECode_Success == gqlite_close(&handle));

    return 0;
}