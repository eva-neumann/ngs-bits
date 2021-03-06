#include "TestFramework.h"

TEST_CLASS(BedLowCoverage_Test)
{
Q_OBJECT
private slots:
	
    void roi_cov20()
	{
		EXECUTE("BedLowCoverage", "-in " + TESTDATA("../cppNGS-TEST/data_in/panel.bed") + " -bam " + TESTDATA("../cppNGS-TEST/data_in/panel.bam") + " -out out/BedLowCoverage_test01_out.bed -cutoff 20");
		COMPARE_FILES("out/BedLowCoverage_test01_out.bed", TESTDATA("data_out/BedLowCoverage_test01_out.bed"));
	}

    void wgs_cov20()
    {
        EXECUTE("BedLowCoverage", "-wgs -bam " + TESTDATA("../cppNGS-TEST/data_in/panel.bam") + " -out out/BedLowCoverage_test02_out.bed -cutoff 20");
        COMPARE_FILES("out/BedLowCoverage_test02_out.bed", TESTDATA("data_out/BedLowCoverage_test02_out.bed"));
    }

};
