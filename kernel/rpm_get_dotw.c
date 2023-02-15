//
// Compute day of the week.
// Sunday is 0.
//
#include <rpm/api.h>

int rpm_get_dotw(int year, int month, int day)
{
    // Solution by Tomohiko Sakamoto.
    static const int offset[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };

    year -= (month < 3);
    return (year + year/4 - year/100 + year/400 + offset[month - 1] + day) % 7;
}
