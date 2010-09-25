/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include <iostream>
#include <cmath>
#include <limits>
#include "gputest.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;
using namespace std;
using namespace gpu;

class CV_GpuArithmTest : public CvTest
{
public:
    CV_GpuArithmTest(const char* test_name, const char* test_funcs) : CvTest(test_name, test_funcs) {}
    virtual ~CV_GpuArithmTest() {}

protected:
    void run(int);
    
    int test(int type);

    virtual int test(const Mat& mat1, const Mat& mat2) = 0;

    int CheckNorm(const Mat& m1, const Mat& m2);
    int CheckNorm(const Scalar& s1, const Scalar& s2);
    int CheckNorm(double d1, double d2);
};

int CV_GpuArithmTest::test(int type)
{
    cv::Size sz(200, 200);
    cv::Mat mat1(sz, type), mat2(sz, type);
    cv::RNG rng(*ts->get_rng());
    rng.fill(mat1, cv::RNG::UNIFORM, cv::Scalar::all(10), cv::Scalar::all(100));
    rng.fill(mat2, cv::RNG::UNIFORM, cv::Scalar::all(10), cv::Scalar::all(100));

    return test(mat1, mat2);
}

int CV_GpuArithmTest::CheckNorm(const Mat& m1, const Mat& m2)
{
    double ret = norm(m1, m2, NORM_INF);

    if (ret < std::numeric_limits<double>::epsilon())
        return CvTS::OK;

    ts->printf(CvTS::LOG, "\nNorm: %f\n", ret);
    return CvTS::FAIL_GENERIC;
}

int CV_GpuArithmTest::CheckNorm(const Scalar& s1, const Scalar& s2)
{
    double ret0 = CheckNorm(s1[0], s2[0]), ret1 = CheckNorm(s1[1], s2[1]), ret2 = CheckNorm(s1[2], s2[2]), ret3 = CheckNorm(s1[3], s2[3]);

    return (ret0 == CvTS::OK && ret1 == CvTS::OK && ret2 == CvTS::OK && ret3 == CvTS::OK) ? CvTS::OK : CvTS::FAIL_GENERIC;
}

int CV_GpuArithmTest::CheckNorm(double d1, double d2)
{
    double ret = ::fabs(d1 - d2);

    if (ret < std::numeric_limits<double>::epsilon())
        return CvTS::OK;

    ts->printf(CvTS::LOG, "\nNorm: %f\n", ret);
    return CvTS::FAIL_GENERIC;
}

void CV_GpuArithmTest::run( int )
{
    int testResult = CvTS::OK;
    try
    {
        const int types[] = {CV_8UC1, CV_8UC3, CV_8UC4, CV_32FC1};
        const char* type_names[] = {"CV_8UC1", "CV_8UC3", "CV_8UC4", "CV_32FC1"};
        const int type_count = sizeof(types)/sizeof(types[0]);

        //run tests
        for (int t = 0; t < type_count; ++t)
        {
            ts->printf(CvTS::LOG, "========Start test %s========\n", type_names[t]);

            if (CvTS::OK == test(types[t]))
                ts->printf(CvTS::LOG, "SUCCESS\n");
            else
            {
                ts->printf(CvTS::LOG, "FAIL\n");
                testResult = CvTS::FAIL_MISMATCH;
            }    
        }

        ///!!! author, please remove commented code if loop above is equivalent.


        /*ts->printf(CvTS::LOG, "\n========Start test 8UC1========\n");
        if (test(CV_8UC1) == CvTS::OK)
            ts->printf(CvTS::LOG, "\nSUCCESS\n");
        else
        {
            ts->printf(CvTS::LOG, "\nFAIL\n");
            testResult = CvTS::FAIL_GENERIC;
        }

        ts->printf(CvTS::LOG, "\n========Start test 8UC3========\n");
        if (test(CV_8UC3) == CvTS::OK)
            ts->printf(CvTS::LOG, "\nSUCCESS\n");
        else
        {
            ts->printf(CvTS::LOG, "\nFAIL\n");
            testResult = CvTS::FAIL_GENERIC;
        }

        ts->printf(CvTS::LOG, "\n========Start test 8UC4========\n");
        if (test(CV_8UC4) == CvTS::OK)
            ts->printf(CvTS::LOG, "\nSUCCESS\n");
        else
        {
            ts->printf(CvTS::LOG, "\nFAIL\n");
            testResult = CvTS::FAIL_GENERIC;
        }

        ts->printf(CvTS::LOG, "\n========Start test 32FC1========\n");
        if (test(CV_32FC1) == CvTS::OK)
            ts->printf(CvTS::LOG, "\nSUCCESS\n");
        else
        {
            ts->printf(CvTS::LOG, "\nFAIL\n");
            testResult = CvTS::FAIL_GENERIC;
        }*/
    }
    catch(const cv::Exception& e)
    {
        if (!check_and_treat_gpu_exception(e, ts))
            throw;
        return;
    }

    ts->set_failed_test_info(testResult);
}

////////////////////////////////////////////////////////////////////////////////
// Add

struct CV_GpuNppImageAddTest : public CV_GpuArithmTest
{
    CV_GpuNppImageAddTest() : CV_GpuArithmTest( "GPU-NppImageAdd", "add" ) {}

	virtual int test(const Mat& mat1, const Mat& mat2)
    {
        if (mat1.type() != CV_8UC1 && mat1.type() != CV_8UC4 && mat1.type() != CV_32FC1)
        {
            ts->printf(CvTS::LOG, "\nUnsupported type\n");
            return CvTS::OK;
        }

        cv::Mat cpuRes;
        cv::add(mat1, mat2, cpuRes);

        GpuMat gpu1(mat1);
        GpuMat gpu2(mat2);
        GpuMat gpuRes;
        cv::gpu::add(gpu1, gpu2, gpuRes);

        return CheckNorm(cpuRes, gpuRes);
    }
};

////////////////////////////////////////////////////////////////////////////////
// Sub
struct CV_GpuNppImageSubtractTest : public CV_GpuArithmTest
{
    CV_GpuNppImageSubtractTest() : CV_GpuArithmTest( "GPU-NppImageSubtract", "subtract" ) {}

    int test( const Mat& mat1, const Mat& mat2 )
    {
        if (mat1.type() != CV_8UC1 && mat1.type() != CV_8UC4 && mat1.type() != CV_32FC1)
        {
            ts->printf(CvTS::LOG, "\nUnsupported type\n");
            return CvTS::OK;
        }

        cv::Mat cpuRes;
        cv::subtract(mat1, mat2, cpuRes);

        GpuMat gpu1(mat1);
        GpuMat gpu2(mat2);
        GpuMat gpuRes;
        cv::gpu::subtract(gpu1, gpu2, gpuRes);    

        return CheckNorm(cpuRes, gpuRes);
    }
};

////////////////////////////////////////////////////////////////////////////////
// multiply
struct CV_GpuNppImageMultiplyTest : public CV_GpuArithmTest
{
    CV_GpuNppImageMultiplyTest() : CV_GpuArithmTest( "GPU-NppImageMultiply", "multiply" ) {}

    int CV_GpuNppImageMultiplyTest::test( const Mat& mat1, const Mat& mat2 )
    {
        if (mat1.type() != CV_8UC1 && mat1.type() != CV_8UC4 && mat1.type() != CV_32FC1)
        {
            ts->printf(CvTS::LOG, "\nUnsupported type\n");
            return CvTS::OK;
        }

	    cv::Mat cpuRes;
	    cv::multiply(mat1, mat2, cpuRes);

	    GpuMat gpu1(mat1);
	    GpuMat gpu2(mat2);
	    GpuMat gpuRes;
	    cv::gpu::multiply(gpu1, gpu2, gpuRes);

	    return CheckNorm(cpuRes, gpuRes);
    }
};

////////////////////////////////////////////////////////////////////////////////
// divide
struct CV_GpuNppImageDivideTest : public CV_GpuArithmTest
{
    CV_GpuNppImageDivideTest() : CV_GpuArithmTest( "GPU-NppImageDivide", "divide" ) {}

    int CV_GpuNppImageDivideTest::test( const Mat& mat1, const Mat& mat2 )
    {
        if (mat1.type() != CV_8UC1 && mat1.type() != CV_8UC4 && mat1.type() != CV_32FC1)
        {
            ts->printf(CvTS::LOG, "\nUnsupported type\n");
            return CvTS::OK;
        }

	    cv::Mat cpuRes;
	    cv::divide(mat1, mat2, cpuRes);

	    GpuMat gpu1(mat1);
	    GpuMat gpu2(mat2);
	    GpuMat gpuRes;
	    cv::gpu::divide(gpu1, gpu2, gpuRes);

	    return CheckNorm(cpuRes, gpuRes);
    }
};

////////////////////////////////////////////////////////////////////////////////
// transpose
struct CV_GpuNppImageTransposeTest : public CV_GpuArithmTest
{
    CV_GpuNppImageTransposeTest() : CV_GpuArithmTest( "GPU-NppImageTranspose", "transpose" ) {}

    int CV_GpuNppImageTransposeTest::test( const Mat& mat1, const Mat& )
    {
        if (mat1.type() != CV_8UC1)
        {
            ts->printf(CvTS::LOG, "\nUnsupported type\n");
            return CvTS::OK;
        }

        cv::Mat cpuRes;
        cv::transpose(mat1, cpuRes);

        GpuMat gpu1(mat1);
        GpuMat gpuRes;
        cv::gpu::transpose(gpu1, gpuRes);

        return CheckNorm(cpuRes, gpuRes);
    }
};

////////////////////////////////////////////////////////////////////////////////
// absdiff
struct CV_GpuNppImageAbsdiffTest : public CV_GpuArithmTest
{
    CV_GpuNppImageAbsdiffTest() : CV_GpuArithmTest( "GPU-NppImageAbsdiff", "absdiff" ) {}

    int CV_GpuNppImageAbsdiffTest::test( const Mat& mat1, const Mat& mat2 )
    {
        if (mat1.type() != CV_8UC1 && mat1.type() != CV_32FC1)
        {
            ts->printf(CvTS::LOG, "\nUnsupported type\n");
            return CvTS::OK;
        }

        cv::Mat cpuRes;
        cv::absdiff(mat1, mat2, cpuRes);

        GpuMat gpu1(mat1);
        GpuMat gpu2(mat2);
        GpuMat gpuRes;
        cv::gpu::absdiff(gpu1, gpu2, gpuRes);

        return CheckNorm(cpuRes, gpuRes);
    }
};

////////////////////////////////////////////////////////////////////////////////
// compare
struct CV_GpuNppImageCompareTest : public CV_GpuArithmTest
{
    CV_GpuNppImageCompareTest() : CV_GpuArithmTest( "GPU-NppImageCompare", "compare" ) {}

    int CV_GpuNppImageCompareTest::test( const Mat& mat1, const Mat& mat2 )
    {
        if (mat1.type() != CV_32FC1)
        {
            ts->printf(CvTS::LOG, "\nUnsupported type\n");
            return CvTS::OK;
        }

        int cmp_codes[] = {CMP_EQ, CMP_GT, CMP_GE, CMP_LT, CMP_LE, CMP_NE};
        const char* cmp_str[] = {"CMP_EQ", "CMP_GT", "CMP_GE", "CMP_LT", "CMP_LE", "CMP_NE"};
        int cmp_num = sizeof(cmp_codes) / sizeof(int);

        int test_res = CvTS::OK;

        for (int i = 0; i < cmp_num; ++i)
        {
            ts->printf(CvTS::LOG, "\nCompare operation: %s\n", cmp_str[i]);

            cv::Mat cpuRes;
            cv::compare(mat1, mat2, cpuRes, cmp_codes[i]);

            GpuMat gpu1(mat1);
            GpuMat gpu2(mat2);
            GpuMat gpuRes;
            cv::gpu::compare(gpu1, gpu2, gpuRes, cmp_codes[i]);

            if (CheckNorm(cpuRes, gpuRes) != CvTS::OK)
                test_res = CvTS::FAIL_GENERIC;
        }

        return test_res;
    }
};

////////////////////////////////////////////////////////////////////////////////
// meanStdDev
struct CV_GpuNppImageMeanStdDevTest : public CV_GpuArithmTest
{
    CV_GpuNppImageMeanStdDevTest() : CV_GpuArithmTest( "GPU-NppImageMeanStdDev", "meanStdDev" ) {}

    int CV_GpuNppImageMeanStdDevTest::test( const Mat& mat1, const Mat& )
    {
        if (mat1.type() != CV_8UC1)
        {
            ts->printf(CvTS::LOG, "\nUnsupported type\n");
            return CvTS::OK;
        }

        Scalar cpumean; 
        Scalar cpustddev;
        cv::meanStdDev(mat1, cpumean, cpustddev);

        GpuMat gpu1(mat1);
        Scalar gpumean; 
        Scalar gpustddev;
        cv::gpu::meanStdDev(gpu1, gpumean, gpustddev);

        int test_res = CvTS::OK;

        if (CheckNorm(cpumean, gpumean) != CvTS::OK)
        {
            ts->printf(CvTS::LOG, "\nMean FAILED\n");
            test_res = CvTS::FAIL_GENERIC;
        }

        if (CheckNorm(cpustddev, gpustddev) != CvTS::OK)
        {
            ts->printf(CvTS::LOG, "\nStdDev FAILED\n");
            test_res = CvTS::FAIL_GENERIC;
        }

        return test_res;
    }
};

////////////////////////////////////////////////////////////////////////////////
// norm
struct CV_GpuNppImageNormTest : public CV_GpuArithmTest
{
    CV_GpuNppImageNormTest() : CV_GpuArithmTest( "GPU-NppImageNorm", "norm" ) {}

    int CV_GpuNppImageNormTest::test( const Mat& mat1, const Mat& mat2 )
    {
        if (mat1.type() != CV_8UC1)
        {
            ts->printf(CvTS::LOG, "\nUnsupported type\n");
            return CvTS::OK;
        }

        int norms[] = {NORM_INF, NORM_L1, NORM_L2};
        const char* norms_str[] = {"NORM_INF", "NORM_L1", "NORM_L2"};
        int norms_num = sizeof(norms) / sizeof(int);

        int test_res = CvTS::OK;

        for (int i = 0; i < norms_num; ++i)
        {
            ts->printf(CvTS::LOG, "\nNorm type: %s\n", norms_str[i]);

            double cpu_norm = cv::norm(mat1, mat2, norms[i]);

            GpuMat gpu1(mat1);
            GpuMat gpu2(mat2);
            double gpu_norm = cv::gpu::norm(gpu1, gpu2, norms[i]);

            if (CheckNorm(cpu_norm, gpu_norm) != CvTS::OK)
                test_res = CvTS::FAIL_GENERIC;
        }

        return test_res;
    }
};

////////////////////////////////////////////////////////////////////////////////
// flip
struct CV_GpuNppImageFlipTest : public CV_GpuArithmTest
{
    CV_GpuNppImageFlipTest() : CV_GpuArithmTest( "GPU-NppImageFlip", "flip" ) {}

    int CV_GpuNppImageFlipTest::test( const Mat& mat1, const Mat& )
    {
        if (mat1.type() != CV_8UC1 && mat1.type() != CV_8UC4)
        {
            ts->printf(CvTS::LOG, "\nUnsupported type\n");
            return CvTS::OK;
        }

        int flip_codes[] = {0, 1, -1};
        const char* flip_axis[] = {"X", "Y", "Both"};
        int flip_codes_num = sizeof(flip_codes) / sizeof(int);

        int test_res = CvTS::OK;

        for (int i = 0; i < flip_codes_num; ++i)
        {
            ts->printf(CvTS::LOG, "\nFlip Axis: %s\n", flip_axis[i]);

            Mat cpu_res;
            cv::flip(mat1, cpu_res, flip_codes[i]);
            
            GpuMat gpu1(mat1);
            GpuMat gpu_res;
            cv::gpu::flip(gpu1, gpu_res, flip_codes[i]);

            if (CheckNorm(cpu_res, gpu_res) != CvTS::OK)
                test_res = CvTS::FAIL_GENERIC;
        }

        return test_res;
    }
};

////////////////////////////////////////////////////////////////////////////////
// sum
struct CV_GpuNppImageSumTest : public CV_GpuArithmTest
{
    CV_GpuNppImageSumTest() : CV_GpuArithmTest( "GPU-NppImageSum", "sum" ) {}

    int CV_GpuNppImageSumTest::test( const Mat& mat1, const Mat& )
    {
        if (mat1.type() != CV_8UC1 && mat1.type() != CV_8UC4)
        {
            ts->printf(CvTS::LOG, "\nUnsupported type\n");
            return CvTS::OK;
        }

        Scalar cpures = cv::sum(mat1);

        GpuMat gpu1(mat1);
        Scalar gpures = cv::gpu::sum(gpu1);

        return CheckNorm(cpures, gpures);
    }
};

////////////////////////////////////////////////////////////////////////////////
// minNax
struct CV_GpuNppImageMinNaxTest : public CV_GpuArithmTest
{
    CV_GpuNppImageMinNaxTest() : CV_GpuArithmTest( "GPU-NppImageMinNax", "minNax" ) {}

    int CV_GpuNppImageMinNaxTest::test( const Mat& mat1, const Mat& )
    {
        if (mat1.type() != CV_8UC1)
        {
            ts->printf(CvTS::LOG, "\nUnsupported type\n");
            return CvTS::OK;
        }

        double cpumin, cpumax;
        cv::minMaxLoc(mat1, &cpumin, &cpumax);

        GpuMat gpu1(mat1);
        double gpumin, gpumax;
        cv::gpu::minMax(gpu1, &gpumin, &gpumax);

        return (CheckNorm(cpumin, gpumin) == CvTS::OK && CheckNorm(cpumax, gpumax) == CvTS::OK) ? CvTS::OK : CvTS::FAIL_GENERIC;
    }
};

////////////////////////////////////////////////////////////////////////////////
// LUT
struct CV_GpuNppImageLUTTest : public CV_GpuArithmTest
{
    CV_GpuNppImageLUTTest() : CV_GpuArithmTest( "GPU-NppImageLUT", "LUT" ) {}

    int CV_GpuNppImageLUTTest::test( const Mat& mat1, const Mat& )
    {
        if (mat1.type() != CV_8UC1)
        {
            ts->printf(CvTS::LOG, "\nUnsupported type\n");
            return CvTS::OK;
        }

        cv::Mat lut(1, 256, CV_32SC1);
        cv::RNG rng(*ts->get_rng());
        rng.fill(lut, cv::RNG::UNIFORM, cv::Scalar::all(100), cv::Scalar::all(200));

        cv::Mat cpuRes;
        cv::LUT(mat1, lut, cpuRes);
        cpuRes.convertTo(cpuRes, CV_8U);

        cv::gpu::GpuMat gpuRes;
        cv::gpu::LUT(GpuMat(mat1), lut, gpuRes);

        return CheckNorm(cpuRes, gpuRes);
    }
};



// If we comment some tests, we may foget/miss to uncomment it after.
// Placing all test definitions in one place 
// makes us know about what tests are commented.

CV_GpuNppImageAddTest CV_GpuNppImageAdd_test;
CV_GpuNppImageSubtractTest CV_GpuNppImageSubtract_test;
CV_GpuNppImageMultiplyTest CV_GpuNppImageMultiply_test;
CV_GpuNppImageDivideTest CV_GpuNppImageDivide_test;
CV_GpuNppImageTransposeTest CV_GpuNppImageTranspose_test;
CV_GpuNppImageAbsdiffTest CV_GpuNppImageAbsdiff_test;
CV_GpuNppImageCompareTest CV_GpuNppImageCompare_test;
CV_GpuNppImageMeanStdDevTest CV_GpuNppImageMeanStdDev_test;
CV_GpuNppImageNormTest CV_GpuNppImageNorm_test;
CV_GpuNppImageFlipTest CV_GpuNppImageFlip_test;
//CV_GpuNppImageSumTest CV_GpuNppImageSum_test;
CV_GpuNppImageMinNaxTest CV_GpuNppImageMinNax_test;
CV_GpuNppImageLUTTest CV_GpuNppImageLUT_test;