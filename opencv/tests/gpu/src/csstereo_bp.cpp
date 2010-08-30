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

#include "gputest.hpp"
#include <iostream>
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/gpu/gpu.hpp>

class CV_GpuCSStereoBP : public CvTest
{
    public:
        CV_GpuCSStereoBP();
    protected:
        void run(int);
};

CV_GpuCSStereoBP::CV_GpuCSStereoBP(): CvTest( "GPU-CSStereoBP", "ConstantSpaceStereoBP" ){}

void CV_GpuCSStereoBP::run(int )
{
        cv::Mat img_l = cv::imread(std::string(ts->get_data_path()) + "csstereobp/aloe-L.png");
        cv::Mat img_r = cv::imread(std::string(ts->get_data_path()) + "csstereobp/aloe-R.png");
        cv::Mat img_template = cv::imread(std::string(ts->get_data_path()) + "csstereobp/aloe-disp.png", 0);

        cv::gpu::GpuMat disp;
        cv::gpu::StereoConstantSpaceBP bpm(128, 16, 4, 4);

        bpm(cv::gpu::GpuMat(img_l), cv::gpu::GpuMat(img_r), disp);

        //cv::imwrite(std::string(ts->get_data_path()) + "csstereobp/aloe-disp.png", disp);

        disp.convertTo(disp, img_template.type());

        double norm = cv::norm(disp, img_template, cv::NORM_INF);
		if (norm >= 0.5) std::cout << "ConstantSpaceStereoBP norm = " << norm << std::endl;
        ts->set_failed_test_info((norm < 0.5) ? CvTS::OK : CvTS::FAIL_GENERIC);
}


CV_GpuCSStereoBP CV_GpuCSStereoBP_test;
