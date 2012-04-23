#include <stdio.h>
#include "cyield.h"

/*
 * Range(max): Simple Example using the yield keyword in C
 * Function declaration is a bit odd, though it could've been worse.
*/
CYIELD_FUNC(Range)
    for (int i = 0; i < arg0->ul; i++)
        CYIELD(i);
CYIELD_FUNC_END(0);

/*
 * EnumDirectory(path): Shows a slightly more realistic scenario.
 * See this example as a comparison between:
 * - returning an array containing every filename as string
 * - returning every time a filename has been found
 * For larger amounts of files in a directory, or slower filesystems (for
 * example one over `sshfs'), it might take quite some time before the entire
 * directory has been enumerated, whereas a better user experience could be
 * gained if files show up (in a listing, etc.) as they are found.
*/

#ifdef __linux__

#include <dirent.h>

CYIELD_FUNC(EnumDirectory)
    DIR *dir = opendir(arg0->psz);
    if(dir != NULL) {
        struct dirent *f;
        while ((f = readdir(dir)) != NULL) {
            CYIELD(f->d_name);
        }
        closedir(dir);
    }
CYIELD_FUNC_END(0);

#else

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

#endif

int main(int argc, char *argv[])
{
    int times = 42;

#ifdef __linux__
    const char *dir = ".";
#else
    const char *dir = "*.*";
#endif

    CYIELD_FOREACH1(int, x, Range, times)
        printf("x: %d\n", x);
    CYIELD_FOREACH_NEXT();

    CYIELD_FOREACH1(const char *, filename, EnumDirectory, dir)
        printf("file: %s\n", filename);
    CYIELD_FOREACH_NEXT();

    CYIELD_FOREACH1(int, x, Range, 5)
        CYIELD_FOREACH1(int, y, Range, 10)
            printf("x: %d, y: %d\n", x, y);
        CYIELD_FOREACH_NEXT();
    CYIELD_FOREACH_NEXT();
}
