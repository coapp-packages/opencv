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

#include "test_precomp.hpp"

#include <fstream>
#include <iterator>
#include <numeric>

using namespace cv;
using namespace std;
using namespace gpu;

class CV_GpuMatOpConvertToTest : public cvtest::BaseTest
{
    public:
        CV_GpuMatOpConvertToTest() {}
        ~CV_GpuMatOpConvertToTest() {}

    protected:
        void run(int);
};

void CV_GpuMatOpConvertToTest::run(int /* start_from */)
{
    const Size img_size(67, 35);

    const char* types_str[] = {"CV_8U", "CV_8S", "CV_16U", "CV_16S", "CV_32S", "CV_32F", "CV_64F"};

    bool passed = true;
    int lastType = CV_32F;

    if (TargetArchs::builtWith(NATIVE_DOUBLE) && DeviceInfo().supports(NATIVE_DOUBLE))
        lastType = CV_64F;

    for (int i = 0; i <= lastType && passed; ++i)
    {
        for (int j = 0; j <= lastType && passed; ++j)
        {
            for (int c = 1; c < 5 && passed; ++c)
            {
                const int src_type = CV_MAKETYPE(i, c);
                const int dst_type = j;

                cv::RNG& rng = ts->get_rng();

                Mat cpumatsrc(img_size, src_type);
                rng.fill(cpumatsrc, RNG::UNIFORM, Scalar::all(0), Scalar::all(300));

                GpuMat gpumatsrc(cpumatsrc);
                Mat cpumatdst;
                GpuMat gpumatdst;

                cpumatsrc.convertTo(cpumatdst, dst_type, 0.5, 3.0);
                gpumatsrc.convertTo(gpumatdst, dst_type, 0.5, 3.0);

                double r = norm(cpumatdst, (Mat)gpumatdst, NORM_INF);
                if (r > 1)
                {
                    ts->printf(cvtest::TS::LOG,
                               "\nFAILED: SRC_TYPE=%sC%d DST_TYPE=%s NORM = %f\n",
                               types_str[i], c, types_str[j], r);
                    passed = false;
                }
            }
        }
    }

    ts->set_failed_test_info(passed ? cvtest::TS::OK : cvtest::TS::FAIL_GENERIC);
}

TEST(GpuMat_convertTo, accuracy) { CV_GpuMatOpConvertToTest test; test.safe_run(); }
