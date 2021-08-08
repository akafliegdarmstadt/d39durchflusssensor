#include <ztest.h>
#include <nmea.h>

void test_nmeaflow_buildmsg(void)
{
	const char expected[] = "$PFLO,010.0*16\n";
	char buf[NMEA_LENGTH] = "";

	nmeaflow_buildmsg(buf, 10.0);

	zassert_mem_equal(expected, buf, NMEA_LENGTH, "nmeaflow message");

}

void test_main(void)
{
	ztest_test_suite(common,
			 ztest_unit_test(test_nmeaflow_buildmsg)
			 );
	ztest_run_test_suite(common);
}
