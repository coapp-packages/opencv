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
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
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

#include "cuda_shared.hpp"
#include "transform.hpp"

using namespace cv::gpu;

#ifndef CV_PI
#define CV_PI   3.1415926535897932384626433832795f
#endif

//////////////////////////////////////////////////////////////////////////////////////
// Cart <-> Polar

namespace cv { namespace gpu { namespace mathfunc
{
    struct Nothing
    {
        static __device__ void calc(int, int, float, float, float*, size_t, float)
        {
        }
    };
    struct Magnitude
    {
        static __device__ void calc(int x, int y, float x_data, float y_data, float* dst, size_t dst_step, float)
        {
            dst[y * dst_step + x] = sqrtf(x_data * x_data + y_data * y_data);
        }
    };
    struct MagnitudeSqr
    {
        static __device__ void calc(int x, int y, float x_data, float y_data, float* dst, size_t dst_step, float)
        {
            dst[y * dst_step + x] = x_data * x_data + y_data * y_data;
        }
    };
    struct Atan2
    {
        static __device__ void calc(int x, int y, float x_data, float y_data, float* dst, size_t dst_step, float scale)
        {
            dst[y * dst_step + x] = scale * atan2f(y_data, x_data);
        }
    };
    template <typename Mag, typename Angle>
    __global__ void cartToPolar(const float* xptr, size_t x_step, const float* yptr, size_t y_step, 
                                float* mag, size_t mag_step, float* angle, size_t angle_step, float scale, int width, int height)
    {
		const int x = blockDim.x * blockIdx.x + threadIdx.x;
		const int y = blockDim.y * blockIdx.y + threadIdx.y;

        if (x < width && y < height)
        {
            float x_data = xptr[y * x_step + x];
            float y_data = yptr[y * y_step + x];

            Mag::calc(x, y, x_data, y_data, mag, mag_step, scale);
            Angle::calc(x, y, x_data, y_data, angle, angle_step, scale);
        }
    }

    struct NonEmptyMag
    {
        static __device__ float get(const float* mag, size_t mag_step, int x, int y)
        {
            return mag[y * mag_step + x];
        }
    };
    struct EmptyMag
    {
        static __device__ float get(const float*, size_t, int, int)
        {
            return 1.0f;
        }
    };
    template <typename Mag>
    __global__ void polarToCart(const float* mag, size_t mag_step, const float* angle, size_t angle_step, float scale,
        float* xptr, size_t x_step, float* yptr, size_t y_step, int width, int height)
    {
		const int x = blockDim.x * blockIdx.x + threadIdx.x;
		const int y = blockDim.y * blockIdx.y + threadIdx.y;

        if (x < width && y < height)
        {
            float mag_data = Mag::get(mag, mag_step, x, y);
            float angle_data = angle[y * angle_step + x];
            float sin_a, cos_a;

            sincosf(scale * angle_data, &sin_a, &cos_a);

            xptr[y * x_step + x] = mag_data * cos_a;
            yptr[y * y_step + x] = mag_data * sin_a;
        }
    }

    template <typename Mag, typename Angle>
    void cartToPolar_caller(const DevMem2Df& x, const DevMem2Df& y, const DevMem2Df& mag, const DevMem2Df& angle, bool angleInDegrees, cudaStream_t stream)
    {
        dim3 threads(16, 16, 1);
        dim3 grid(1, 1, 1);

        grid.x = divUp(x.cols, threads.x);
        grid.y = divUp(x.rows, threads.y);
        
        const float scale = angleInDegrees ? (float)(180.0f / CV_PI) : 1.f;

        cartToPolar<Mag, Angle><<<grid, threads, 0, stream>>>(
            x.data, x.step/x.elemSize(), y.data, y.step/y.elemSize(), 
            mag.data, mag.step/mag.elemSize(), angle.data, angle.step/angle.elemSize(), scale, x.cols, x.rows);

        if (stream == 0)
            cudaSafeCall( cudaThreadSynchronize() );
    }

    void cartToPolar_gpu(const DevMem2Df& x, const DevMem2Df& y, const DevMem2Df& mag, bool magSqr, const DevMem2Df& angle, bool angleInDegrees, cudaStream_t stream)
    {
        typedef void (*caller_t)(const DevMem2Df& x, const DevMem2Df& y, const DevMem2Df& mag, const DevMem2Df& angle, bool angleInDegrees, cudaStream_t stream);
        static const caller_t callers[2][2][2] = 
        {
            {
                {
                    cartToPolar_caller<Magnitude, Atan2>,
                    cartToPolar_caller<Magnitude, Nothing>
                },
                {
                    cartToPolar_caller<MagnitudeSqr, Atan2>,
                    cartToPolar_caller<MagnitudeSqr, Nothing>,
                }
            },
            {
                {
                    cartToPolar_caller<Nothing, Atan2>,
                    cartToPolar_caller<Nothing, Nothing>
                },
                {
                    cartToPolar_caller<Nothing, Atan2>,
                    cartToPolar_caller<Nothing, Nothing>,
                }
            }
        };

        callers[mag.data == 0][magSqr][angle.data == 0](x, y, mag, angle, angleInDegrees, stream);
    }

    template <typename Mag>
    void polarToCart_caller(const DevMem2Df& mag, const DevMem2Df& angle, const DevMem2Df& x, const DevMem2Df& y, bool angleInDegrees, cudaStream_t stream)
    {
        dim3 threads(16, 16, 1);
        dim3 grid(1, 1, 1);

        grid.x = divUp(mag.cols, threads.x);
        grid.y = divUp(mag.rows, threads.y);
        
        const float scale = angleInDegrees ? (float)(CV_PI / 180.0f) : 1.0f;

        polarToCart<Mag><<<grid, threads, 0, stream>>>(mag.data, mag.step/mag.elemSize(), 
            angle.data, angle.step/angle.elemSize(), scale, x.data, x.step/x.elemSize(), y.data, y.step/y.elemSize(), mag.cols, mag.rows);

        if (stream == 0)
            cudaSafeCall( cudaThreadSynchronize() );
    }

    void polarToCart_gpu(const DevMem2Df& mag, const DevMem2Df& angle, const DevMem2Df& x, const DevMem2Df& y, bool angleInDegrees, cudaStream_t stream)
    {
        typedef void (*caller_t)(const DevMem2Df& mag, const DevMem2Df& angle, const DevMem2Df& x, const DevMem2Df& y, bool angleInDegrees, cudaStream_t stream);
        static const caller_t callers[2] = 
        {
            polarToCart_caller<NonEmptyMag>,
            polarToCart_caller<EmptyMag>
        };

        callers[mag.data == 0](mag, angle, x, y, angleInDegrees, stream);
    }

//////////////////////////////////////////////////////////////////////////////////////
// Compare

    template <typename T1, typename T2>
    struct NotEqual
    {
        __device__ uchar operator()(const T1& src1, const T2& src2, int, int)
        {
            return static_cast<uchar>(static_cast<int>(src1 != src2) * 255);
        }
    };

    template <typename T1, typename T2>
    inline void compare_ne(const DevMem2D& src1, const DevMem2D& src2, const DevMem2D& dst)
    {
        NotEqual<T1, T2> op;
        transform(static_cast< DevMem2D_<T1> >(src1), static_cast< DevMem2D_<T2> >(src2), dst, op, 0);
    }

    void compare_ne_8uc4(const DevMem2D& src1, const DevMem2D& src2, const DevMem2D& dst)
    {
        compare_ne<uint, uint>(src1, src2, dst);
    }
    void compare_ne_32f(const DevMem2D& src1, const DevMem2D& src2, const DevMem2D& dst)
    {
        compare_ne<float, float>(src1, src2, dst);
    }


//////////////////////////////////////////////////////////////////////////////
// Per-element bit-wise logical matrix operations


    __global__ void bitwise_not_kernel(int cols, int rows, const PtrStep src, PtrStep dst)
    {
        const int x = blockDim.x * blockIdx.x + threadIdx.x;
        const int y = blockDim.y * blockIdx.y + threadIdx.y;

        if (x < cols && y < rows)
        {
            dst.ptr(y)[x] = ~src.ptr(y)[x];
        }
    }


    void bitwise_not_caller(const DevMem2D src, int elemSize, PtrStep dst, cudaStream_t stream)
    {
        dim3 threads(16, 16, 1);
        dim3 grid(divUp(src.cols * elemSize, threads.x), divUp(src.rows, threads.y), 1);

        bitwise_not_kernel<<<grid, threads, 0, stream>>>(src.cols * elemSize, src.rows, src, dst);

        if (stream == 0)
            cudaSafeCall(cudaThreadSynchronize());
    }


    __global__ void bitwise_or_kernel(int cols, int rows, const PtrStep src1, const PtrStep src2, PtrStep dst)
    {
        const int x = blockDim.x * blockIdx.x + threadIdx.x;
        const int y = blockDim.y * blockIdx.y + threadIdx.y;

        if (x < cols && y < rows)
        {
            dst.ptr(y)[x] = src1.ptr(y)[x] | src2.ptr(y)[x];
        }
    }


    void bitwise_or_caller(int cols, int rows, const PtrStep src1, const PtrStep src2, int elemSize, PtrStep dst, cudaStream_t stream)
    {
        dim3 threads(16, 16, 1);
        dim3 grid(divUp(cols * elemSize, threads.x), divUp(rows, threads.y), 1);

        bitwise_or_kernel<<<grid, threads, 0, stream>>>(cols * elemSize, rows, src1, src2, dst);

        if (stream == 0)
            cudaSafeCall(cudaThreadSynchronize());
    }


    __global__ void bitwise_and_kernel(int cols, int rows, const PtrStep src1, const PtrStep src2, PtrStep dst)
    {
        const int x = blockDim.x * blockIdx.x + threadIdx.x;
        const int y = blockDim.y * blockIdx.y + threadIdx.y;

        if (x < cols && y < rows)
        {
            dst.ptr(y)[x] = src1.ptr(y)[x] & src2.ptr(y)[x];
        }
    }


    void bitwise_and_caller(int cols, int rows, const PtrStep src1, const PtrStep src2, int elemSize, PtrStep dst, cudaStream_t stream)
    {
        dim3 threads(16, 16, 1);
        dim3 grid(divUp(cols * elemSize, threads.x), divUp(rows, threads.y), 1);

        bitwise_and_kernel<<<grid, threads, 0, stream>>>(cols * elemSize, rows, src1, src2, dst);

        if (stream == 0)
            cudaSafeCall(cudaThreadSynchronize());
    }



    __global__ void bitwise_xor_kernel(int cols, int rows, const PtrStep src1, const PtrStep src2, PtrStep dst)
    {
        const int x = blockDim.x * blockIdx.x + threadIdx.x;
        const int y = blockDim.y * blockIdx.y + threadIdx.y;

        if (x < cols && y < rows)
        {
            dst.ptr(y)[x] = src1.ptr(y)[x] ^ src2.ptr(y)[x];
        }
    }


    void bitwise_xor_caller(int cols, int rows, const PtrStep src1, const PtrStep src2, int elemSize, PtrStep dst, cudaStream_t stream)
    {
        dim3 threads(16, 16, 1);
        dim3 grid(divUp(cols * elemSize, threads.x), divUp(rows, threads.y), 1);

        bitwise_xor_kernel<<<grid, threads, 0, stream>>>(cols * elemSize, rows, src1, src2, dst);

        if (stream == 0)
            cudaSafeCall(cudaThreadSynchronize());
    }
}}}
