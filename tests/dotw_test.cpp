//
// Test fpm_get_dotw() - Day Of The Week function.
//
#include <gtest/gtest.h>
#include <fpm/api.h>

enum { SUN = 0, MON, TUE, WED, THU, FRI, SAT };

TEST(dotw, arbitrary_wednesday)
{
    // Wednesday, February 15, 2023 - when I wrote this test.
    int wday = fpm_get_dotw(2023, 2, 15);
    EXPECT_EQ(wday, WED);
}

TEST(dotw, space_x_endeavor)
{
    // Saturday, May 30, 2020 - Space-X launches astronauts to space station.
    int wday = fpm_get_dotw(2020, 5, 30);
    EXPECT_EQ(wday, SAT);
}

TEST(dotw, war_on_japan)
{
    // Monday, December 8, 1941 - U.S. and Britain declare war on Japan.
    int wday = fpm_get_dotw(1941, 12, 8);
    EXPECT_EQ(wday, MON);
}

TEST(dotw, emancipation_proclamation)
{
    // Thursday, January 1, 1863 - Abraham Lincoln signs the Emancipation Proclamation ending slavery.
    int wday = fpm_get_dotw(1863, 1, 1);
    EXPECT_EQ(wday, THU);
}

TEST(dotw, u_s_capital)
{
    // Friday, December 12, 1800 - Washington, D.C. is established as the U.S. capital.
    int wday = fpm_get_dotw(1800, 12, 12);
    EXPECT_EQ(wday, FRI);
}
