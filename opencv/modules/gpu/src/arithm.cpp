/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
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
//     and/or other GpuMaterials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or bpied warranties, including, but not limited to, the bpied
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

#include "precomp.hpp"

using namespace cv;
using namespace cv::gpu;
using namespace std;

#if !defined (HAVE_CUDA)

void cv::gpu::add(const GpuMat&, const GpuMat&, GpuMat&) { throw_nogpu(); }
void cv::gpu::subtract(const GpuMat&, const GpuMat&, GpuMat&) { throw_nogpu(); }
void cv::gpu::multiply(const GpuMat&, const GpuMat&, GpuMat&) { throw_nogpu(); }
void cv::gpu::divide(const GpuMat&, const GpuMat&, GpuMat&) { throw_nogpu(); }
void cv::gpu::transpose(const GpuMat&, GpuMat&) { throw_nogpu(); }
void cv::gpu::absdiff(const GpuMat&, const GpuMat&, GpuMat&) { throw_nogpu(); }
void cv::gpu::compare(const GpuMat&, const GpuMat&, GpuMat&, int) { throw_nogpu(); }
void cv::gpu::meanStdDev(const GpuMat&, Scalar&, Scalar&) { throw_nogpu(); }
double cv::gpu::norm(const GpuMat&, int) { throw_nogpu(); return 0.0; }
double cv::gpu::norm(const GpuMat&, const GpuMat&, int) { throw_nogpu(); return 0.0; }
void cv::gpu::flip(const GpuMat&, GpuMat&, int) { throw_nogpu(); }
Scalar cv::gpu::sum(const GpuMat&) { throw_nogpu(); return Scalar(); }
void cv::gpu::minMax(const GpuMat&, double*, double*) { throw_nogpu(); }
void cv::gpu::LUT(const GpuMat& src, const Mat& lut, GpuMat& dst) { throw_nogpu(); }

#else /* !defined (HAVE_CUDA) */

////////////////////////////////////////////////////////////////////////
// add subtract multiply divide

namespace
{
	typedef NppStatus (*npp_arithm_8u_t)(const Npp8u* pSrc1, int nSrc1Step, const Npp8u* pSrc2, int nSrc2Step, Npp8u* pDst, int nDstStep, 
										 NppiSize oSizeROI, int nScaleFactor);
	typedef NppStatus (*npp_arithm_32f_t)(const Npp32f* pSrc1, int nSrc1Step, const Npp32f* pSrc2, int nSrc2Step, Npp32f* pDst, 
									      int nDstStep, NppiSize oSizeROI);

	void nppFuncCaller(const GpuMat& src1, const GpuMat& src2, GpuMat& dst, 
					   npp_arithm_8u_t npp_func_8uc1, npp_arithm_8u_t npp_func_8uc4, npp_arithm_32f_t npp_func_32fc1)
	{
        CV_DbgAssert(src1.size() == src2.size() && src1.type() == src2.type());

        CV_Assert(src1.type() == CV_8UC1 || src1.type() == CV_8UC4 || src1.type() == CV_32FC1);

        dst.create( src1.size(), src1.type() );

		NppiSize sz;
		sz.width  = src1.cols;
		sz.height = src1.rows;

        switch (src1.type())
        {
        case CV_8UC1:
			nppSafeCall( npp_func_8uc1(src1.ptr<Npp8u>(), src1.step, 
				src2.ptr<Npp8u>(), src2.step, 
				dst.ptr<Npp8u>(), dst.step, sz, 0) );
            break;
        case CV_8UC4:
			nppSafeCall( npp_func_8uc4(src1.ptr<Npp8u>(), src1.step, 
				src2.ptr<Npp8u>(), src2.step, 
				dst.ptr<Npp8u>(), dst.step, sz, 0) );
            break;
        case CV_32FC1:
			nppSafeCall( npp_func_32fc1(src1.ptr<Npp32f>(), src1.step,
				src2.ptr<Npp32f>(), src2.step,
				dst.ptr<Npp32f>(), dst.step, sz) );
            break;
        default:
            CV_Assert(!"Unsupported source type");
        }
	}
}

void cv::gpu::add(const GpuMat& src1, const GpuMat& src2, GpuMat& dst)
{
	nppFuncCaller(src1, src2, dst, nppiAdd_8u_C1RSfs, nppiAdd_8u_C4RSfs, nppiAdd_32f_C1R);
}

void cv::gpu::subtract(const GpuMat& src1, const GpuMat& src2, GpuMat& dst) 
{
	nppFuncCaller(src2, src1, dst, nppiSub_8u_C1RSfs, nppiSub_8u_C4RSfs, nppiSub_32f_C1R);
}

void cv::gpu::multiply(const GpuMat& src1, const GpuMat& src2, GpuMat& dst)
{
	nppFuncCaller(src1, src2, dst, nppiMul_8u_C1RSfs, nppiMul_8u_C4RSfs, nppiMul_32f_C1R);
}

void cv::gpu::divide(const GpuMat& src1, const GpuMat& src2, GpuMat& dst)
{
	nppFuncCaller(src2, src1, dst, nppiDiv_8u_C1RSfs, nppiDiv_8u_C4RSfs, nppiDiv_32f_C1R);
}

////////////////////////////////////////////////////////////////////////
// transpose

void cv::gpu::transpose(const GpuMat& src, GpuMat& dst)
{
    CV_Assert(src.type() == CV_8UC1);

    dst.create( src.cols, src.rows, src.type() );

    NppiSize sz;
    sz.width  = src.cols;
    sz.height = src.rows;

    nppSafeCall( nppiTranspose_8u_C1R(src.ptr<Npp8u>(), src.step, dst.ptr<Npp8u>(), dst.step, sz) );
}

////////////////////////////////////////////////////////////////////////
// absdiff

void cv::gpu::absdiff(const GpuMat& src1, const GpuMat& src2, GpuMat& dst)
{
	CV_DbgAssert(src1.size() == src2.size() && src1.type() == src2.type());

	CV_Assert(src1.type() == CV_8UC1 || src1.type() == CV_32FC1);

    dst.create( src1.size(), src1.type() );

    NppiSize sz;
    sz.width  = src1.cols;
    sz.height = src1.rows;

    if (src1.type() == CV_8UC1)
    {
        nppSafeCall( nppiAbsDiff_8u_C1R(src1.ptr<Npp8u>(), src1.step, 
            src2.ptr<Npp8u>(), src2.step, 
            dst.ptr<Npp8u>(), dst.step, sz) );
    }
    else
    {
        nppSafeCall( nppiAbsDiff_32f_C1R(src1.ptr<Npp32f>(), src1.step,
            src2.ptr<Npp32f>(), src2.step,
            dst.ptr<Npp32f>(), dst.step, sz) );
    }
}

////////////////////////////////////////////////////////////////////////
// compare

namespace cv { namespace gpu { namespace matrix_operations
{
    void compare_ne_8u(const DevMem2D& src1, const DevMem2D& src2, const DevMem2D& dst);
    void compare_ne_32f(const DevMem2D& src1, const DevMem2D& src2, const DevMem2D& dst);
}}}

void cv::gpu::compare(const GpuMat& src1, const GpuMat& src2, GpuMat& dst, int cmpop) 
{
    CV_DbgAssert(src1.size() == src2.size() && src1.type() == src2.type());

    CV_Assert(src1.type() == CV_8UC4 || src1.type() == CV_32FC1);

    dst.create( src1.size(), CV_8UC1 );

    static const NppCmpOp nppCmpOp[] = { NPP_CMP_EQ, NPP_CMP_GREATER, NPP_CMP_GREATER_EQ, NPP_CMP_LESS, NPP_CMP_LESS_EQ };

    NppiSize sz;
    sz.width  = src1.cols;
    sz.height = src1.rows;

    if (src1.type() == CV_8UC4)
    {
        if (cmpop != CMP_NE)
        {
            nppSafeCall( nppiCompare_8u_C4R(src1.ptr<Npp8u>(), src1.step, 
                src2.ptr<Npp8u>(), src2.step, 
                dst.ptr<Npp8u>(), dst.step, sz, nppCmpOp[cmpop]) );
        }
        else
        {
            matrix_operations::compare_ne_8u(src1, src2, dst);
        }
    }
    else
    {
        if (cmpop != CMP_NE)
        {
            nppSafeCall( nppiCompare_32f_C1R(src1.ptr<Npp32f>(), src1.step,
                src2.ptr<Npp32f>(), src2.step,
                dst.ptr<Npp8u>(), dst.step, sz, nppCmpOp[cmpop]) );
        }
        else
        {
            matrix_operations::compare_ne_32f(src1, src2, dst);
        }
    }
}

////////////////////////////////////////////////////////////////////////
// meanStdDev

void cv::gpu::meanStdDev(const GpuMat& src, Scalar& mean, Scalar& stddev) 
{
    CV_Assert(src.type() == CV_8UC1);

    NppiSize sz;
    sz.width  = src.cols;
    sz.height = src.rows;

    nppSafeCall( nppiMean_StdDev_8u_C1R(src.ptr<Npp8u>(), src.step, sz, mean.val, stddev.val) );
}

////////////////////////////////////////////////////////////////////////
// norm

double cv::gpu::norm(const GpuMat& src1, int normType) 
{
    return norm(src1, GpuMat(src1.size(), src1.type(), Scalar::all(0.0)), normType);
}

double cv::gpu::norm(const GpuMat& src1, const GpuMat& src2, int normType)
{
    CV_DbgAssert(src1.size() == src2.size() && src1.type() == src2.type());

    CV_Assert(src1.type() == CV_8UC1);
    CV_Assert(normType == NORM_INF || normType == NORM_L1 || normType == NORM_L2);

    typedef NppStatus (*npp_norm_diff_func_t)(const Npp8u* pSrc1, int nSrcStep1, const Npp8u* pSrc2, int nSrcStep2, 
        NppiSize oSizeROI, Npp64f* pRetVal);

    static const npp_norm_diff_func_t npp_norm_diff_func[] = {nppiNormDiff_Inf_8u_C1R, nppiNormDiff_L1_8u_C1R, nppiNormDiff_L2_8u_C1R};

    NppiSize sz;
    sz.width  = src1.cols;
    sz.height = src1.rows;

    int funcIdx = normType >> 1;
    Scalar retVal;

    nppSafeCall( npp_norm_diff_func[funcIdx](src1.ptr<Npp8u>(), src1.step, 
        src2.ptr<Npp8u>(), src2.step, 
        sz, retVal.val) );

    return retVal[0];
}

////////////////////////////////////////////////////////////////////////
// flip

void cv::gpu::flip(const GpuMat& src, GpuMat& dst, int flipCode)
{
    CV_Assert(src.type() == CV_8UC1 || src.type() == CV_8UC4);

    dst.create( src.size(), src.type() );

    NppiSize sz;
    sz.width  = src.cols;
    sz.height = src.rows;

    if (src.type() == CV_8UC1)
    {
        nppSafeCall( nppiMirror_8u_C1R(src.ptr<Npp8u>(), src.step, 
            dst.ptr<Npp8u>(), dst.step, sz, 
            (flipCode == 0 ? NPP_HORIZONTAL_AXIS : (flipCode > 0 ? NPP_VERTICAL_AXIS : NPP_BOTH_AXIS))) );
    }
    else
    {
        nppSafeCall( nppiMirror_8u_C4R(src.ptr<Npp8u>(), src.step, 
            dst.ptr<Npp8u>(), dst.step, sz, 
            (flipCode == 0 ? NPP_HORIZONTAL_AXIS : (flipCode > 0 ? NPP_VERTICAL_AXIS : NPP_BOTH_AXIS))) );
    }
}

////////////////////////////////////////////////////////////////////////
// sum

Scalar cv::gpu::sum(const GpuMat& src)
{
    CV_Assert(src.type() == CV_8UC1 || src.type() == CV_8UC4);
    
    Scalar res;

    NppiSize sz;
    sz.width  = src.cols;
    sz.height = src.rows;

    if (src.type() == CV_8UC1)
    {
        nppSafeCall( nppiSum_8u_C1R(src.ptr<Npp8u>(), src.step, sz, res.val) );
    }
    else
    {
        nppSafeCall( nppiSum_8u_C4R(src.ptr<Npp8u>(), src.step, sz, res.val) );
    }

    return res;
}

////////////////////////////////////////////////////////////////////////
// minMax

void cv::gpu::minMax(const GpuMat& src, double* minVal, double* maxVal) 
{
    CV_Assert(src.type() == CV_8UC1);

    NppiSize sz;
    sz.width  = src.cols;
    sz.height = src.rows;

    Npp8u min_res, max_res;

    nppSafeCall( nppiMinMax_8u_C1R(src.ptr<Npp8u>(), src.step, sz, &min_res, &max_res) );

    if (minVal)
        *minVal = min_res;

    if (maxVal)
        *maxVal = max_res;
}

////////////////////////////////////////////////////////////////////////
// LUT

void cv::gpu::LUT(const GpuMat& src, const Mat& lut, GpuMat& dst)
{
    class LevelsInit
    {
    public:
        Npp32s pLevels[256];

        LevelsInit()
        {            
            for (int i = 0; i < 256; ++i)
                pLevels[i] = i;
        }
    };
    static LevelsInit lvls;

    int cn = src.channels();

    CV_Assert(src.type() == CV_8UC1);
    CV_Assert(lut.depth() == CV_32SC1 && lut.rows * lut.cols == 256 && lut.isContinuous());

    dst.create(src.size(), src.type());

    NppiSize sz;
    sz.height = src.rows;
    sz.width = src.cols;

    nppSafeCall( nppiLUT_Linear_8u_C1R(src.ptr<Npp8u>(), src.step, dst.ptr<Npp8u>(), dst.step, sz, 
        lut.ptr<Npp32s>(), lvls.pLevels, 256) );
}

#endif /* !defined (HAVE_CUDA) */