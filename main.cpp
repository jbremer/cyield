#include <stdio.h>
#include "cyield.h"

/*
 * Range(max): Simple Example using the yield keyword in C
 * Function declaration is a bit odd though, but it could've been worse.
*/
CYIELD_FUNC(Range)
    for (int i = 0; i < arg0->ul; i++)
        CYIELD(i);
CYIELD_FUNC_END(0);

/*
 * EnumDirectory(path): Shows a slightly more realistic scenario using yield.
*/
CYIELD_FUNC(EnumDirectory)
    WIN32_FIND_DATA FindData;
    HANDLE hEnum = FindFirstFile(arg0->psz, &FindData);
    if(hEnum != INVALID_HANDLE_VALUE) {
        do {
            CYIELD(FindData.cFileName);
        } while (FindNextFile(hEnum, &FindData));
        FindClose(hEnum);
    }
CYIELD_FUNC_END(0);

int main(int argc, char *argv[])
{
    int times = 42;
    const char *dir = "*.*";

    CYIELD_FOREACH1(int, x, Range, times)
        printf("x: %d\n", x);
    CYIELD_FOREACH_NEXT();

    CYIELD_FOREACH1(const char *, filename, EnumDirectory, dir)
        printf("file: %s\n", filename);
    CYIELD_FOREACH_NEXT();
}
