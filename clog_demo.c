#include "clog.h"

int main()
{
    clog_init(LL_DBG);
    clog_dbg("This is a debug message with value: %d", 42);
    return 0;
}