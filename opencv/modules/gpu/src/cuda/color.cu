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

#include "internal_shared.hpp"
#include "opencv2/gpu/device/saturate_cast.hpp"
#include "opencv2/gpu/device/vecmath.hpp"
#include "opencv2/gpu/device/limits_gpu.hpp"
#include "opencv2/gpu/device/transform.hpp"

using namespace cv::gpu;
using namespace cv::gpu::device;

#ifndef CV_DESCALE
#define CV_DESCALE(x, n) (((x) + (1 << ((n)-1))) >> (n))
#endif

namespace cv { namespace gpu { namespace color
{
    template<typename T> struct ColorChannel
    {
        typedef float worktype_f;
        static __device__ __forceinline__ T max() { return numeric_limits_gpu<T>::max(); }
        static __device__ __forceinline__ T half() { return (T)(max()/2 + 1); }
    };
    template<> struct ColorChannel<float>
    {
        typedef float worktype_f;
        static __device__ __forceinline__ float max() { return 1.f; }
        static __device__ __forceinline__ float half() { return 0.5f; }
    };

    template <typename T>
    __device__ __forceinline__ void setAlpha(typename TypeVec<T, 3>::vec_t& vec, T val)
    {
    }
    template <typename T>
    __device__ __forceinline__ void setAlpha(typename TypeVec<T, 4>::vec_t& vec, T val)
    {
        vec.w = val;
    }
    template <typename T>
    __device__ __forceinline__ T getAlpha(const typename TypeVec<T, 3>::vec_t& vec)
    {
        return ColorChannel<T>::max();
    }
    template <typename T>
    __device__ __forceinline__ T getAlpha(const typename TypeVec<T, 4>::vec_t& vec)
    {
        return vec.w;
    }

    template <typename Cvt>
    void callConvert(const DevMem2D& src, const DevMem2D& dst, const Cvt& cvt, cudaStream_t stream)
    {
        typedef typename Cvt::src_t src_t;
        typedef typename Cvt::dst_t dst_t;

        transform((DevMem2D_<src_t>)src, (DevMem2D_<dst_t>)dst, cvt, stream);
    }

////////////////// Various 3/4-channel to 3/4-channel RGB transformations /////////////////

    template <typename T, int SRCCN, int DSTCN>
    struct RGB2RGB
    {
        typedef typename TypeVec<T, SRCCN>::vec_t src_t;
        typedef typename TypeVec<T, DSTCN>::vec_t dst_t;

        explicit RGB2RGB(int bidx) : bidx(bidx) {}

        __device__ __forceinline__ dst_t operator()(const src_t& src) const
        {
            dst_t dst;

            dst.x = (&src.x)[bidx];
            dst.y = src.y;
            dst.z = (&src.x)[bidx ^ 2];
            setAlpha(dst, getAlpha<T>(src));

            return dst;
        }

    private:
        int bidx;
    };

    template <typename T, int SRCCN, int DSTCN>
    void RGB2RGB_caller(const DevMem2D& src, const DevMem2D& dst, int bidx, cudaStream_t stream)
    {
        RGB2RGB<T, SRCCN, DSTCN> cvt(bidx);
        callConvert(src, dst, cvt, stream);
    }

    void RGB2RGB_gpu_8u(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, int bidx, cudaStream_t stream)
    {
        typedef void (*RGB2RGB_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, cudaStream_t stream);
        static const RGB2RGB_caller_t RGB2RGB_callers[2][2] =
        {
            {RGB2RGB_caller<uchar, 3, 3>, RGB2RGB_caller<uchar, 3, 4>},
            {RGB2RGB_caller<uchar, 4, 3>, RGB2RGB_caller<uchar, 4, 4>}
        };

        RGB2RGB_callers[srccn-3][dstcn-3](src, dst, bidx, stream);
    }

    void RGB2RGB_gpu_16u(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, int bidx, cudaStream_t stream)
    {
        typedef void (*RGB2RGB_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, cudaStream_t stream);
        static const RGB2RGB_caller_t RGB2RGB_callers[2][2] =
        {
            {RGB2RGB_caller<ushort, 3, 3>, RGB2RGB_caller<ushort, 3, 4>},
            {RGB2RGB_caller<ushort, 4, 3>, RGB2RGB_caller<ushort, 4, 4>}
        };

        RGB2RGB_callers[srccn-3][dstcn-3](src, dst, bidx, stream);
    }

    void RGB2RGB_gpu_32f(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, int bidx, cudaStream_t stream)
    {
        typedef void (*RGB2RGB_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, cudaStream_t stream);
        static const RGB2RGB_caller_t RGB2RGB_callers[2][2] =
        {
            {RGB2RGB_caller<float, 3, 3>, RGB2RGB_caller<float, 3, 4>},
            {RGB2RGB_caller<float, 4, 3>, RGB2RGB_caller<float, 4, 4>}
        };

        RGB2RGB_callers[srccn-3][dstcn-3](src, dst, bidx, stream);
    }

/////////// Transforming 16-bit (565 or 555) RGB to/from 24/32-bit (888[8]) RGB //////////

    template <int GREEN_BITS> struct RGB5x52RGBConverter;
    template <> struct RGB5x52RGBConverter<5>
    {
        template <typename D>
        static __device__ __forceinline__ void cvt(uint src, D& dst, int bidx)
        {
            (&dst.x)[bidx] = (uchar)(src << 3);
            dst.y = (uchar)((src >> 2) & ~7);
            (&dst.x)[bidx ^ 2] = (uchar)((src >> 7) & ~7);
            setAlpha(dst, (uchar)(src & 0x8000 ? 255 : 0));
        }
    };
    template <> struct RGB5x52RGBConverter<6>
    {
        template <typename D>
        static __device__ __forceinline__ void cvt(uint src, D& dst, int bidx)
        {
            (&dst.x)[bidx] = (uchar)(src << 3);
            dst.y = (uchar)((src >> 3) & ~3);
            (&dst.x)[bidx ^ 2] = (uchar)((src >> 8) & ~7);
            setAlpha(dst, (uchar)(255));
        }
    };

    template <int GREEN_BITS, int DSTCN> struct RGB5x52RGB
    {
        typedef ushort src_t;
        typedef typename TypeVec<uchar, DSTCN>::vec_t dst_t;

        explicit RGB5x52RGB(int bidx) : bidx(bidx) {}

        __device__ __forceinline__ dst_t operator()(ushort src) const
        {
            dst_t dst;
            RGB5x52RGBConverter<GREEN_BITS>::cvt((uint)src, dst, bidx);
            return dst;
        }

    private:
        int bidx;
    };

    template <int GREEN_BITS> struct RGB2RGB5x5Converter;
    template<> struct RGB2RGB5x5Converter<6>
    {
        template <typename T>
        static __device__ __forceinline__ ushort cvt(const T& src, int bidx)
        {
            return (ushort)(((&src.x)[bidx] >> 3) | ((src.y & ~3) << 3) | (((&src.x)[bidx^2] & ~7) << 8));
        }
    };
    template<> struct RGB2RGB5x5Converter<5>
    {
        static __device__ __forceinline__ ushort cvt(const uchar3& src, int bidx)
        {
            return (ushort)(((&src.x)[bidx] >> 3) | ((src.y & ~7) << 2) | (((&src.x)[bidx^2] & ~7) << 7));
        }
        static __device__ __forceinline__ ushort cvt(const uchar4& src, int bidx)
        {
            return (ushort)(((&src.x)[bidx] >> 3) | ((src.y & ~7) << 2) | (((&src.x)[bidx^2] & ~7) << 7) | (src.w ? 0x8000 : 0));
        }
    };

    template<int SRCCN, int GREEN_BITS> struct RGB2RGB5x5
    {
        typedef typename TypeVec<uchar, SRCCN>::vec_t src_t;
        typedef ushort dst_t;

        explicit RGB2RGB5x5(int bidx) : bidx(bidx) {}

        __device__ __forceinline__ ushort operator()(const src_t& src)
        {
            return RGB2RGB5x5Converter<GREEN_BITS>::cvt(src, bidx);
        }

    private:
        int bidx;
    };

    template <int GREEN_BITS, int DSTCN>
    void RGB5x52RGB_caller(const DevMem2D& src, const DevMem2D& dst, int bidx, cudaStream_t stream)
    {
        RGB5x52RGB<GREEN_BITS, DSTCN> cvt(bidx);
        callConvert(src, dst, cvt, stream);
    }

    void RGB5x52RGB_gpu(const DevMem2D& src, int green_bits, const DevMem2D& dst, int dstcn, int bidx, cudaStream_t stream)
    {
        typedef void (*RGB5x52RGB_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, cudaStream_t stream);
        static const RGB5x52RGB_caller_t RGB5x52RGB_callers[2][2] =
        {
            {RGB5x52RGB_caller<5, 3>, RGB5x52RGB_caller<5, 4>},
            {RGB5x52RGB_caller<6, 3>, RGB5x52RGB_caller<6, 4>}
        };

        RGB5x52RGB_callers[green_bits - 5][dstcn - 3](src, dst, bidx, stream);
    }

    template <int SRCCN, int GREEN_BITS>
    void RGB2RGB5x5_caller(const DevMem2D& src, const DevMem2D& dst, int bidx, cudaStream_t stream)
    {
        RGB2RGB5x5<SRCCN, GREEN_BITS> cvt(bidx);
        callConvert(src, dst, cvt, stream);
    }

    void RGB2RGB5x5_gpu(const DevMem2D& src, int srccn, const DevMem2D& dst, int green_bits, int bidx, cudaStream_t stream)
    {
        typedef void (*RGB2RGB5x5_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, cudaStream_t stream);
        static const RGB2RGB5x5_caller_t RGB2RGB5x5_callers[2][2] =
        {
            {RGB2RGB5x5_caller<3, 5>, RGB2RGB5x5_caller<3, 6>},
            {RGB2RGB5x5_caller<4, 5>, RGB2RGB5x5_caller<4, 6>}
        };

        RGB2RGB5x5_callers[srccn - 3][green_bits - 5](src, dst, bidx, stream);
    }

///////////////////////////////// Grayscale to Color ////////////////////////////////

    template <int DSTCN, typename T> struct Gray2RGB
    {
        typedef T src_t;
        typedef typename TypeVec<T, DSTCN>::vec_t dst_t;

        __device__ __forceinline__ dst_t operator()(const T& src) const
        {
            dst_t dst;

            dst.z = dst.y = dst.x = src;
            setAlpha(dst, ColorChannel<T>::max());

            return dst;
        }
    };

    template <int GREEN_BITS> struct Gray2RGB5x5Converter;
    template<> struct Gray2RGB5x5Converter<6>
    {
        static __device__ __forceinline__ ushort cvt(uint t)
        {
            return (ushort)((t >> 3) | ((t & ~3) << 3) | ((t & ~7) << 8));
        }
    };
    template<> struct Gray2RGB5x5Converter<5>
    {
        static __device__ __forceinline__ ushort cvt(uint t)
        {
            t >>= 3;
            return (ushort)(t | (t << 5) | (t << 10));
        }
    };

    template<int GREEN_BITS> struct Gray2RGB5x5
    {
        typedef uchar src_t;
        typedef ushort dst_t;

        __device__ __forceinline__ ushort operator()(uchar src) const
        {
            return Gray2RGB5x5Converter<GREEN_BITS>::cvt((uint)src);
        }
    };

    template <typename T, int DSTCN>
    void Gray2RGB_caller(const DevMem2D& src, const DevMem2D& dst, cudaStream_t stream)
    {
        Gray2RGB<DSTCN, T> cvt;
        callConvert(src, dst, cvt, stream);
    }

    void Gray2RGB_gpu_8u(const DevMem2D& src, const DevMem2D& dst, int dstcn, cudaStream_t stream)
    {
        typedef void (*Gray2RGB_caller_t)(const DevMem2D& src, const DevMem2D& dst, cudaStream_t stream);
        static const Gray2RGB_caller_t Gray2RGB_callers[] = {Gray2RGB_caller<uchar, 3>, Gray2RGB_caller<uchar, 4>};

        Gray2RGB_callers[dstcn - 3](src, dst, stream);
    }

    void Gray2RGB_gpu_16u(const DevMem2D& src, const DevMem2D& dst, int dstcn, cudaStream_t stream)
    {
        typedef void (*Gray2RGB_caller_t)(const DevMem2D& src, const DevMem2D& dst, cudaStream_t stream);
        static const Gray2RGB_caller_t Gray2RGB_callers[] = {Gray2RGB_caller<ushort, 3>, Gray2RGB_caller<ushort, 4>};

        Gray2RGB_callers[dstcn - 3](src, dst, stream);
    }

    void Gray2RGB_gpu_32f(const DevMem2D& src, const DevMem2D& dst, int dstcn, cudaStream_t stream)
    {
        typedef void (*Gray2RGB_caller_t)(const DevMem2D& src, const DevMem2D& dst, cudaStream_t stream);
        static const Gray2RGB_caller_t Gray2RGB_callers[] = {Gray2RGB_caller<float, 3>, Gray2RGB_caller<float, 4>};

        Gray2RGB_callers[dstcn - 3](src, dst, stream);
    }

    template <int GREEN_BITS>
    void Gray2RGB5x5_caller(const DevMem2D& src, const DevMem2D& dst, cudaStream_t stream)
    {
        Gray2RGB5x5<GREEN_BITS> cvt;
        callConvert(src, dst, cvt, stream);
    }

    void Gray2RGB5x5_gpu(const DevMem2D& src, const DevMem2D& dst, int green_bits, cudaStream_t stream)
    {
        typedef void (*Gray2RGB5x5_caller_t)(const DevMem2D& src, const DevMem2D& dst, cudaStream_t stream);
        static const Gray2RGB5x5_caller_t Gray2RGB5x5_callers[2] =
        {
            Gray2RGB5x5_caller<5>, Gray2RGB5x5_caller<6>
        };

        Gray2RGB5x5_callers[green_bits - 5](src, dst, stream);
    }

///////////////////////////////// Color to Grayscale ////////////////////////////////

    #undef R2Y
    #undef G2Y
    #undef B2Y

    enum
    {
        yuv_shift  = 14,
        xyz_shift  = 12,
        R2Y        = 4899,
        G2Y        = 9617,
        B2Y        = 1868,
        BLOCK_SIZE = 256
    };

    template <int GREEN_BITS> struct RGB5x52GrayConverter;
    template<> struct RGB5x52GrayConverter<6>
    {
        static __device__ __forceinline__ uchar cvt(uint t)
        {
            return (uchar)CV_DESCALE(((t << 3) & 0xf8) * B2Y + ((t >> 3) & 0xfc) * G2Y + ((t >> 8) & 0xf8) * R2Y, yuv_shift);
        }
    };
    template<> struct RGB5x52GrayConverter<5>
    {
        static __device__ __forceinline__ uchar cvt(uint t)
        {
            return (uchar)CV_DESCALE(((t << 3) & 0xf8) * B2Y + ((t >> 2) & 0xf8) * G2Y + ((t >> 7) & 0xf8) * R2Y, yuv_shift);
        }
    };

    template<int GREEN_BITS> struct RGB5x52Gray
    {
        typedef ushort src_t;
        typedef uchar dst_t;

        __device__ __forceinline__ uchar operator()(ushort src) const
        {
            return RGB5x52GrayConverter<GREEN_BITS>::cvt((uint)src);
        }
    };

    template <typename T>
    __device__ __forceinline__ T RGB2GrayConvert(const T* src, int bidx)
    {
        return (T)CV_DESCALE((unsigned)(src[bidx] * B2Y + src[1] * G2Y + src[bidx^2] * R2Y), yuv_shift);
    }
     __device__ __forceinline__ float RGB2GrayConvert(const float* src, int bidx)
    {
        const float cr = 0.299f;
        const float cg = 0.587f;
        const float cb = 0.114f;

        return src[bidx] * cb + src[1] * cg + src[bidx^2] * cr;
    }

    template <int SRCCN, typename T> struct RGB2Gray
    {
        typedef typename TypeVec<T, SRCCN>::vec_t src_t;
        typedef T dst_t;

        explicit RGB2Gray(int bidx) : bidx(bidx) {}

        __device__ __forceinline__ T operator()(const src_t& src)
        {
            return RGB2GrayConvert(&src.x, bidx);
        }

    private:
        int bidx;
    };

    template <typename T, int SRCCN>
    void RGB2Gray_caller(const DevMem2D& src, const DevMem2D& dst, int bidx, cudaStream_t stream)
    {
        RGB2Gray<SRCCN, T> cvt(bidx);
        callConvert(src, dst, cvt, stream);
    }

    void RGB2Gray_gpu_8u(const DevMem2D& src, int srccn, const DevMem2D& dst, int bidx, cudaStream_t stream)
    {
        typedef void (*RGB2Gray_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, cudaStream_t stream);
        RGB2Gray_caller_t RGB2Gray_callers[] = {RGB2Gray_caller<uchar, 3>, RGB2Gray_caller<uchar, 4>};

        RGB2Gray_callers[srccn - 3](src, dst, bidx, stream);
    }

    void RGB2Gray_gpu_16u(const DevMem2D& src, int srccn, const DevMem2D& dst, int bidx, cudaStream_t stream)
    {
        typedef void (*RGB2Gray_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, cudaStream_t stream);
        RGB2Gray_caller_t RGB2Gray_callers[] = {RGB2Gray_caller<ushort, 3>, RGB2Gray_caller<ushort, 4>};

        RGB2Gray_callers[srccn - 3](src, dst, bidx, stream);
    }

    void RGB2Gray_gpu_32f(const DevMem2D& src, int srccn, const DevMem2D& dst, int bidx, cudaStream_t stream)
    {
        typedef void (*RGB2Gray_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, cudaStream_t stream);
        RGB2Gray_caller_t RGB2Gray_callers[] = {RGB2Gray_caller<float, 3>, RGB2Gray_caller<float, 4>};

        RGB2Gray_callers[srccn - 3](src, dst, bidx, stream);
    }

    template <int GREEN_BITS>
    void RGB5x52Gray_caller(const DevMem2D& src, const DevMem2D& dst, cudaStream_t stream)
    {
        RGB5x52Gray<GREEN_BITS> cvt;
        callConvert(src, dst, cvt, stream);
    }

    void RGB5x52Gray_gpu(const DevMem2D& src, int green_bits, const DevMem2D& dst, cudaStream_t stream)
    {
        typedef void (*RGB5x52Gray_caller_t)(const DevMem2D& src, const DevMem2D& dst, cudaStream_t stream);
        static const RGB5x52Gray_caller_t RGB5x52Gray_callers[2] =
        {
            RGB5x52Gray_caller<5>, RGB5x52Gray_caller<6>
        };

        RGB5x52Gray_callers[green_bits - 5](src, dst, stream);
    }

///////////////////////////////////// RGB <-> YCrCb //////////////////////////////////////

    __constant__ int cYCrCbCoeffs_i[5];
    __constant__ float cYCrCbCoeffs_f[5];

    template <typename T, typename D>
    __device__ __forceinline__ void RGB2YCrCbConvert(const T* src, D& dst, int bidx)
    {
        const int delta = ColorChannel<T>::half() * (1 << yuv_shift);

        const int Y = CV_DESCALE(src[0] * cYCrCbCoeffs_i[0] + src[1] * cYCrCbCoeffs_i[1] + src[2] * cYCrCbCoeffs_i[2], yuv_shift);
        const int Cr = CV_DESCALE((src[bidx^2] - Y) * cYCrCbCoeffs_i[3] + delta, yuv_shift);
        const int Cb = CV_DESCALE((src[bidx] - Y) * cYCrCbCoeffs_i[4] + delta, yuv_shift);

        dst.x = saturate_cast<T>(Y);
        dst.y = saturate_cast<T>(Cr);
        dst.z = saturate_cast<T>(Cb);
    }
    template <typename D>
    static __device__ __forceinline__ void RGB2YCrCbConvert(const float* src, D& dst, int bidx)
    {
        dst.x = src[0] * cYCrCbCoeffs_f[0] + src[1] * cYCrCbCoeffs_f[1] + src[2] * cYCrCbCoeffs_f[2];
        dst.y = (src[bidx^2] - dst.x) * cYCrCbCoeffs_f[3] + ColorChannel<float>::half();
        dst.z = (src[bidx] - dst.x) * cYCrCbCoeffs_f[4] + ColorChannel<float>::half();
    }

    template<typename T> struct RGB2YCrCbBase
    {
        typedef int coeff_t;

        explicit RGB2YCrCbBase(const coeff_t coeffs[5])
        {
            cudaSafeCall( cudaMemcpyToSymbol(cYCrCbCoeffs_i, coeffs, 5 * sizeof(int)) );
        }
    };
    template<> struct RGB2YCrCbBase<float>
    {
        typedef float coeff_t;

        explicit RGB2YCrCbBase(const coeff_t coeffs[5])
        {
            cudaSafeCall( cudaMemcpyToSymbol(cYCrCbCoeffs_f, coeffs, 5 * sizeof(float)) );
        }
    };
    template <int SRCCN, int DSTCN, typename T> struct RGB2YCrCb : RGB2YCrCbBase<T>
    {
        typedef typename RGB2YCrCbBase<T>::coeff_t coeff_t;
        typedef typename TypeVec<T, SRCCN>::vec_t src_t;
        typedef typename TypeVec<T, DSTCN>::vec_t dst_t;

        RGB2YCrCb(int bidx, const coeff_t coeffs[5]) : RGB2YCrCbBase<T>(coeffs), bidx(bidx) {}

        __device__ __forceinline__ dst_t operator()(const src_t& src) const
        {
            dst_t dst;
            RGB2YCrCbConvert(&src.x, dst, bidx);
            return dst;
        }

    private:
        int bidx;
    };

    template <typename T, typename D>
    __device__ __forceinline__ void YCrCb2RGBConvert(const T& src, D* dst, int bidx)
    {
        const int b = src.x + CV_DESCALE((src.z - ColorChannel<D>::half()) * cYCrCbCoeffs_i[3], yuv_shift);
        const int g = src.x + CV_DESCALE((src.z - ColorChannel<D>::half()) * cYCrCbCoeffs_i[2] + (src.y - ColorChannel<D>::half()) * cYCrCbCoeffs_i[1], yuv_shift);
        const int r = src.x + CV_DESCALE((src.y - ColorChannel<D>::half()) * cYCrCbCoeffs_i[0], yuv_shift);

        dst[bidx] = saturate_cast<D>(b);
        dst[1] = saturate_cast<D>(g);
        dst[bidx^2] = saturate_cast<D>(r);
    }
    template <typename T>
    __device__ __forceinline__ void YCrCb2RGBConvert(const T& src, float* dst, int bidx)
    {
        dst[bidx] = src.x + (src.z - ColorChannel<float>::half()) * cYCrCbCoeffs_f[3];
        dst[1] = src.x + (src.z - ColorChannel<float>::half()) * cYCrCbCoeffs_f[2] + (src.y - ColorChannel<float>::half()) * cYCrCbCoeffs_f[1];
        dst[bidx^2] = src.x + (src.y - ColorChannel<float>::half()) * cYCrCbCoeffs_f[0];
    }

    template<typename T> struct YCrCb2RGBBase
    {
        typedef int coeff_t;

        explicit YCrCb2RGBBase(const coeff_t coeffs[4])
        {
            cudaSafeCall( cudaMemcpyToSymbol(cYCrCbCoeffs_i, coeffs, 4 * sizeof(int)) );
        }
    };
    template<> struct YCrCb2RGBBase<float>
    {
        typedef float coeff_t;

        explicit YCrCb2RGBBase(const coeff_t coeffs[4])
        {
            cudaSafeCall( cudaMemcpyToSymbol(cYCrCbCoeffs_f, coeffs, 4 * sizeof(float)) );
        }
    };
    template <int SRCCN, int DSTCN, typename T> struct YCrCb2RGB : YCrCb2RGBBase<T>
    {
        typedef typename YCrCb2RGBBase<T>::coeff_t coeff_t;
        typedef typename TypeVec<T, SRCCN>::vec_t src_t;
        typedef typename TypeVec<T, DSTCN>::vec_t dst_t;

        YCrCb2RGB(int bidx, const coeff_t coeffs[4]) : YCrCb2RGBBase<T>(coeffs), bidx(bidx) {}

        __device__ __forceinline__ dst_t operator()(const src_t& src) const
        {
            dst_t dst;

            YCrCb2RGBConvert(src, &dst.x, bidx);
            setAlpha(dst, ColorChannel<T>::max());

            return dst;
        }

    private:
        int bidx;
    };

    template <typename T, int SRCCN, int DSTCN>
    void RGB2YCrCb_caller(const DevMem2D& src, const DevMem2D& dst, int bidx, const void* coeffs, cudaStream_t stream)
    {
        typedef typename RGB2YCrCb<SRCCN, DSTCN, T>::coeff_t coeff_t;
        RGB2YCrCb<SRCCN, DSTCN, T> cvt(bidx, (const coeff_t*)coeffs);
        callConvert(src, dst, cvt, stream);
    }

    void RGB2YCrCb_gpu_8u(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, int bidx, const void* coeffs, cudaStream_t stream)
    {
        typedef void (*RGB2YCrCb_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, const void* coeffs, cudaStream_t stream);
        static const RGB2YCrCb_caller_t RGB2YCrCb_callers[2][2] =
        {
            {RGB2YCrCb_caller<uchar, 3, 3>, RGB2YCrCb_caller<uchar, 3, 4>},
            {RGB2YCrCb_caller<uchar, 4, 3>, RGB2YCrCb_caller<uchar, 4, 4>}
        };

        RGB2YCrCb_callers[srccn-3][dstcn-3](src, dst, bidx, coeffs, stream);
    }

    void RGB2YCrCb_gpu_16u(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, int bidx, const void* coeffs, cudaStream_t stream)
    {
        typedef void (*RGB2YCrCb_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, const void* coeffs, cudaStream_t stream);
        static const RGB2YCrCb_caller_t RGB2YCrCb_callers[2][2] =
        {
            {RGB2YCrCb_caller<ushort, 3, 3>, RGB2YCrCb_caller<ushort, 3, 4>},
            {RGB2YCrCb_caller<ushort, 4, 3>, RGB2YCrCb_caller<ushort, 4, 4>}
        };

        RGB2YCrCb_callers[srccn-3][dstcn-3](src, dst, bidx, coeffs, stream);
    }

    void RGB2YCrCb_gpu_32f(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, int bidx, const void* coeffs, cudaStream_t stream)
    {
        typedef void (*RGB2YCrCb_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, const void* coeffs, cudaStream_t stream);
        static const RGB2YCrCb_caller_t RGB2YCrCb_callers[2][2] =
        {
            {RGB2YCrCb_caller<float, 3, 3>, RGB2YCrCb_caller<float, 3, 4>},
            {RGB2YCrCb_caller<float, 4, 3>, RGB2YCrCb_caller<float, 4, 4>}
        };

        RGB2YCrCb_callers[srccn-3][dstcn-3](src, dst, bidx, coeffs, stream);
    }

    template <typename T, int SRCCN, int DSTCN>
    void YCrCb2RGB_caller(const DevMem2D& src, const DevMem2D& dst, int bidx, const void* coeffs, cudaStream_t stream)
    {
        typedef typename YCrCb2RGB<SRCCN, DSTCN, T>::coeff_t coeff_t;
        YCrCb2RGB<SRCCN, DSTCN, T> cvt(bidx, (const coeff_t*)coeffs);
        callConvert(src, dst, cvt, stream);
    }

    void YCrCb2RGB_gpu_8u(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, int bidx, const void* coeffs, cudaStream_t stream)
    {
        typedef void (*YCrCb2RGB_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, const void* coeffs, cudaStream_t stream);
        static const YCrCb2RGB_caller_t YCrCb2RGB_callers[2][2] =
        {
            {YCrCb2RGB_caller<uchar, 3, 3>, YCrCb2RGB_caller<uchar, 3, 4>},
            {YCrCb2RGB_caller<uchar, 4, 3>, YCrCb2RGB_caller<uchar, 4, 4>}
        };

        YCrCb2RGB_callers[srccn-3][dstcn-3](src, dst, bidx, coeffs, stream);
    }

    void YCrCb2RGB_gpu_16u(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, int bidx, const void* coeffs, cudaStream_t stream)
    {
        typedef void (*YCrCb2RGB_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, const void* coeffs, cudaStream_t stream);
        static const YCrCb2RGB_caller_t YCrCb2RGB_callers[2][2] =
        {
            {YCrCb2RGB_caller<ushort, 3, 3>, YCrCb2RGB_caller<ushort, 3, 4>},
            {YCrCb2RGB_caller<ushort, 4, 3>, YCrCb2RGB_caller<ushort, 4, 4>}
        };

        YCrCb2RGB_callers[srccn-3][dstcn-3](src, dst, bidx, coeffs, stream);
    }

    void YCrCb2RGB_gpu_32f(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, int bidx, const void* coeffs, cudaStream_t stream)
    {
        typedef void (*YCrCb2RGB_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, const void* coeffs, cudaStream_t stream);
        static const YCrCb2RGB_caller_t YCrCb2RGB_callers[2][2] =
        {
            {YCrCb2RGB_caller<float, 3, 3>, YCrCb2RGB_caller<float, 3, 4>},
            {YCrCb2RGB_caller<float, 4, 3>, YCrCb2RGB_caller<float, 4, 4>}
        };

        YCrCb2RGB_callers[srccn-3][dstcn-3](src, dst, bidx, coeffs, stream);
    }

////////////////////////////////////// RGB <-> XYZ ///////////////////////////////////////

    __constant__ int cXYZ_D65i[9];
    __constant__ float cXYZ_D65f[9];

    template <typename T, typename D>
    __device__ __forceinline__ void RGB2XYZConvert(const T* src, D& dst)
    {
        dst.x = saturate_cast<T>(CV_DESCALE(src[0] * cXYZ_D65i[0] + src[1] * cXYZ_D65i[1] + src[2] * cXYZ_D65i[2], xyz_shift));
        dst.y = saturate_cast<T>(CV_DESCALE(src[0] * cXYZ_D65i[3] + src[1] * cXYZ_D65i[4] + src[2] * cXYZ_D65i[5], xyz_shift));
        dst.z = saturate_cast<T>(CV_DESCALE(src[0] * cXYZ_D65i[6] + src[1] * cXYZ_D65i[7] + src[2] * cXYZ_D65i[8], xyz_shift));
    }
    template <typename D>
    __device__ __forceinline__ void RGB2XYZConvert(const float* src, D& dst)
    {
        dst.x = src[0] * cXYZ_D65f[0] + src[1] * cXYZ_D65f[1] + src[2] * cXYZ_D65f[2];
        dst.y = src[0] * cXYZ_D65f[3] + src[1] * cXYZ_D65f[4] + src[2] * cXYZ_D65f[5];
        dst.z = src[0] * cXYZ_D65f[6] + src[1] * cXYZ_D65f[7] + src[2] * cXYZ_D65f[8];
    }

    template <typename T> struct RGB2XYZBase
    {
        typedef int coeff_t;

        explicit RGB2XYZBase(const coeff_t coeffs[9])
        {
            cudaSafeCall( cudaMemcpyToSymbol(cXYZ_D65i, coeffs, 9 * sizeof(int)) );
        }
    };
    template <> struct RGB2XYZBase<float>
    {
        typedef float coeff_t;

        explicit RGB2XYZBase(const coeff_t coeffs[9])
        {
            cudaSafeCall( cudaMemcpyToSymbol(cXYZ_D65f, coeffs, 9 * sizeof(float)) );
        }
    };
    template <int SRCCN, int DSTCN, typename T> struct RGB2XYZ : RGB2XYZBase<T>
    {
        typedef typename RGB2XYZBase<T>::coeff_t coeff_t;
        typedef typename TypeVec<T, SRCCN>::vec_t src_t;
        typedef typename TypeVec<T, DSTCN>::vec_t dst_t;

        explicit RGB2XYZ(const coeff_t coeffs[9]) : RGB2XYZBase<T>(coeffs) {}

        __device__ __forceinline__ dst_t operator()(const src_t& src) const
        {
            dst_t dst;
            RGB2XYZConvert(&src.x, dst);
            return dst;
        }
    };

    template <typename T, typename D>
    __device__ __forceinline__ void XYZ2RGBConvert(const T& src, D* dst)
    {
        dst[0] = saturate_cast<D>(CV_DESCALE(src.x * cXYZ_D65i[0] + src.y * cXYZ_D65i[1] + src.z * cXYZ_D65i[2], xyz_shift));
            dst[1] = saturate_cast<D>(CV_DESCALE(src.x * cXYZ_D65i[3] + src.y * cXYZ_D65i[4] + src.z * cXYZ_D65i[5], xyz_shift));
            dst[2] = saturate_cast<D>(CV_DESCALE(src.x * cXYZ_D65i[6] + src.y * cXYZ_D65i[7] + src.z * cXYZ_D65i[8], xyz_shift));
    }
    template <typename T>
    __device__ __forceinline__ void XYZ2RGBConvert(const T& src, float* dst)
    {
        dst[0] = src.x * cXYZ_D65f[0] + src.y * cXYZ_D65f[1] + src.z * cXYZ_D65f[2];
            dst[1] = src.x * cXYZ_D65f[3] + src.y * cXYZ_D65f[4] + src.z * cXYZ_D65f[5];
            dst[2] = src.x * cXYZ_D65f[6] + src.y * cXYZ_D65f[7] + src.z * cXYZ_D65f[8];
    }

    template <typename T> struct XYZ2RGBBase
    {
        typedef int coeff_t;

        explicit XYZ2RGBBase(const coeff_t coeffs[9])
        {
            cudaSafeCall( cudaMemcpyToSymbol(cXYZ_D65i, coeffs, 9 * sizeof(int)) );
        }
    };
    template <> struct XYZ2RGBBase<float>
    {
        typedef float coeff_t;

        explicit XYZ2RGBBase(const coeff_t coeffs[9])
        {
            cudaSafeCall( cudaMemcpyToSymbol(cXYZ_D65f, coeffs, 9 * sizeof(float)) );
        }
    };
    template <int SRCCN, int DSTCN, typename T> struct XYZ2RGB : XYZ2RGBBase<T>
    {
        typedef typename RGB2XYZBase<T>::coeff_t coeff_t;
        typedef typename TypeVec<T, SRCCN>::vec_t src_t;
        typedef typename TypeVec<T, DSTCN>::vec_t dst_t;

        explicit XYZ2RGB(const coeff_t coeffs[9]) : XYZ2RGBBase<T>(coeffs) {}

        __device__ __forceinline__ dst_t operator()(const src_t& src) const
        {
            dst_t dst;
            XYZ2RGBConvert(src, &dst.x);
            setAlpha(dst, ColorChannel<T>::max());
            return dst;
        }
    };

    template <typename T, int SRCCN, int DSTCN>
    void RGB2XYZ_caller(const DevMem2D& src, const DevMem2D& dst, const void* coeffs, cudaStream_t stream)
    {
        typedef typename RGB2XYZ<SRCCN, DSTCN, T>::coeff_t coeff_t;
        RGB2XYZ<SRCCN, DSTCN, T> cvt((const coeff_t*)coeffs);
        callConvert(src, dst, cvt, stream);
    }

    void RGB2XYZ_gpu_8u(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, const void* coeffs, cudaStream_t stream)
    {
        typedef void (*RGB2XYZ_caller_t)(const DevMem2D& src, const DevMem2D& dst, const void* coeffs, cudaStream_t stream);
        static const RGB2XYZ_caller_t RGB2XYZ_callers[2][2] =
        {
            {RGB2XYZ_caller<uchar, 3, 3>, RGB2XYZ_caller<uchar, 3, 4>},
            {RGB2XYZ_caller<uchar, 4, 3>, RGB2XYZ_caller<uchar, 4, 4>}
        };

        RGB2XYZ_callers[srccn-3][dstcn-3](src, dst, coeffs, stream);
    }

    void RGB2XYZ_gpu_16u(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, const void* coeffs, cudaStream_t stream)
    {
        typedef void (*RGB2XYZ_caller_t)(const DevMem2D& src, const DevMem2D& dst, const void* coeffs, cudaStream_t stream);
        static const RGB2XYZ_caller_t RGB2XYZ_callers[2][2] =
        {
            {RGB2XYZ_caller<ushort, 3, 3>, RGB2XYZ_caller<ushort, 3, 4>},
            {RGB2XYZ_caller<ushort, 4, 3>, RGB2XYZ_caller<ushort, 4, 4>}
        };

        RGB2XYZ_callers[srccn-3][dstcn-3](src, dst, coeffs, stream);
    }

    void RGB2XYZ_gpu_32f(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, const void* coeffs, cudaStream_t stream)
    {
        typedef void (*RGB2XYZ_caller_t)(const DevMem2D& src, const DevMem2D& dst, const void* coeffs, cudaStream_t stream);
        static const RGB2XYZ_caller_t RGB2XYZ_callers[2][2] =
        {
            {RGB2XYZ_caller<float, 3, 3>, RGB2XYZ_caller<float, 3, 4>},
            {RGB2XYZ_caller<float, 4, 3>, RGB2XYZ_caller<float, 4, 4>}
        };

        RGB2XYZ_callers[srccn-3][dstcn-3](src, dst, coeffs, stream);
    }

    template <typename T, int SRCCN, int DSTCN>
    void XYZ2RGB_caller(const DevMem2D& src, const DevMem2D& dst, const void* coeffs, cudaStream_t stream)
    {
        typedef typename XYZ2RGB<SRCCN, DSTCN, T>::coeff_t coeff_t;
        XYZ2RGB<SRCCN, DSTCN, T> cvt((const coeff_t*)coeffs);
        callConvert(src, dst, cvt, stream);
    }

    void XYZ2RGB_gpu_8u(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, const void* coeffs, cudaStream_t stream)
    {
        typedef void (*XYZ2RGB_caller_t)(const DevMem2D& src, const DevMem2D& dst, const void* coeffs, cudaStream_t stream);
        static const XYZ2RGB_caller_t XYZ2RGB_callers[2][2] =
        {
            {XYZ2RGB_caller<uchar, 3, 3>, XYZ2RGB_caller<uchar, 3, 4>},
            {XYZ2RGB_caller<uchar, 4, 3>, XYZ2RGB_caller<uchar, 4, 4>}
        };

        XYZ2RGB_callers[srccn-3][dstcn-3](src, dst, coeffs, stream);
    }

    void XYZ2RGB_gpu_16u(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, const void* coeffs, cudaStream_t stream)
    {
        typedef void (*XYZ2RGB_caller_t)(const DevMem2D& src, const DevMem2D& dst, const void* coeffs, cudaStream_t stream);
        static const XYZ2RGB_caller_t XYZ2RGB_callers[2][2] =
        {
            {XYZ2RGB_caller<ushort, 3, 3>, XYZ2RGB_caller<ushort, 3, 4>},
            {XYZ2RGB_caller<ushort, 4, 3>, XYZ2RGB_caller<ushort, 4, 4>}
        };

        XYZ2RGB_callers[srccn-3][dstcn-3](src, dst, coeffs, stream);
    }

    void XYZ2RGB_gpu_32f(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, const void* coeffs, cudaStream_t stream)
    {
        typedef void (*XYZ2RGB_caller_t)(const DevMem2D& src, const DevMem2D& dst, const void* coeffs, cudaStream_t stream);
        static const XYZ2RGB_caller_t XYZ2RGB_callers[2][2] =
        {
            {XYZ2RGB_caller<float, 3, 3>, XYZ2RGB_caller<float, 3, 4>},
            {XYZ2RGB_caller<float, 4, 3>, XYZ2RGB_caller<float, 4, 4>}
        };

        XYZ2RGB_callers[srccn-3][dstcn-3](src, dst, coeffs, stream);
    }

////////////////////////////////////// RGB <-> HSV ///////////////////////////////////////

    __constant__ int cHsvDivTable   [256] = {0, 1044480, 522240, 348160, 261120, 208896, 174080, 149211, 130560, 116053, 104448, 94953, 87040, 80345, 74606, 69632, 65280, 61440, 58027, 54973, 52224, 49737, 47476, 45412, 43520, 41779, 40172, 38684, 37303, 36017, 34816, 33693, 32640, 31651, 30720, 29842, 29013, 28229, 27486, 26782, 26112, 25475, 24869, 24290, 23738, 23211, 22706, 22223, 21760, 21316, 20890, 20480, 20086, 19707, 19342, 18991, 18651, 18324, 18008, 17703, 17408, 17123, 16846, 16579, 16320, 16069, 15825, 15589, 15360, 15137, 14921, 14711, 14507, 14308, 14115, 13926, 13743, 13565, 13391, 13221, 13056, 12895, 12738, 12584, 12434, 12288, 12145, 12006, 11869, 11736, 11605, 11478, 11353, 11231, 11111, 10995, 10880, 10768, 10658, 10550, 10445, 10341, 10240, 10141, 10043, 9947, 9854, 9761, 9671, 9582, 9495, 9410, 9326, 9243, 9162, 9082, 9004, 8927, 8852, 8777, 8704, 8632, 8561, 8492, 8423, 8356, 8290, 8224, 8160, 8097, 8034, 7973, 7913, 7853, 7795, 7737, 7680, 7624, 7569, 7514, 7461, 7408, 7355, 7304, 7253, 7203, 7154, 7105, 7057, 7010, 6963, 6917, 6872, 6827, 6782, 6739, 6695, 6653, 6611, 6569, 6528, 6487, 6447, 6408, 6369, 6330, 6292, 6254, 6217, 6180, 6144, 6108, 6073, 6037, 6003, 5968, 5935, 5901, 5868, 5835, 5803, 5771, 5739, 5708, 5677, 5646, 5615, 5585, 5556, 5526, 5497, 5468, 5440, 5412, 5384, 5356, 5329, 5302, 5275, 5249, 5222, 5196, 5171, 5145, 5120, 5095, 5070, 5046, 5022, 4998, 4974, 4950, 4927, 4904, 4881, 4858, 4836, 4813, 4791, 4769, 4748, 4726, 4705, 4684, 4663, 4642, 4622, 4601, 4581, 4561, 4541, 4522, 4502, 4483, 4464, 4445, 4426, 4407, 4389, 4370, 4352, 4334, 4316, 4298, 4281, 4263, 4246, 4229, 4212, 4195, 4178, 4161, 4145, 4128, 4112, 4096};
    __constant__ int cHsvDivTable180[256] = {0, 122880, 61440, 40960, 30720, 24576, 20480, 17554, 15360, 13653, 12288, 11171, 10240, 9452, 8777, 8192, 7680, 7228, 6827, 6467, 6144, 5851, 5585, 5343, 5120, 4915, 4726, 4551, 4389, 4237, 4096, 3964, 3840, 3724, 3614, 3511, 3413, 3321, 3234, 3151, 3072, 2997, 2926, 2858, 2793, 2731, 2671, 2614, 2560, 2508, 2458, 2409, 2363, 2318, 2276, 2234, 2194, 2156, 2119, 2083, 2048, 2014, 1982, 1950, 1920, 1890, 1862, 1834, 1807, 1781, 1755, 1731, 1707, 1683, 1661, 1638, 1617, 1596, 1575, 1555, 1536, 1517, 1499, 1480, 1463, 1446, 1429, 1412, 1396, 1381, 1365, 1350, 1336, 1321, 1307, 1293, 1280, 1267, 1254, 1241, 1229, 1217, 1205, 1193, 1182, 1170, 1159, 1148, 1138, 1127, 1117, 1107, 1097, 1087, 1078, 1069, 1059, 1050, 1041, 1033, 1024, 1016, 1007, 999, 991, 983, 975, 968, 960, 953, 945, 938, 931, 924, 917, 910, 904, 897, 890, 884, 878, 871, 865, 859, 853, 847, 842, 836, 830, 825, 819, 814, 808, 803, 798, 793, 788, 783, 778, 773, 768, 763, 759, 754, 749, 745, 740, 736, 731, 727, 723, 719, 714, 710, 706, 702, 698, 694, 690, 686, 683, 679, 675, 671, 668, 664, 661, 657, 654, 650, 647, 643, 640, 637, 633, 630, 627, 624, 621, 617, 614, 611, 608, 605, 602, 599, 597, 594, 591, 588, 585, 582, 580, 577, 574, 572, 569, 566, 564, 561, 559, 556, 554, 551, 549, 546, 544, 541, 539, 537, 534, 532, 530, 527, 525, 523, 521, 518, 516, 514, 512, 510, 508, 506, 504, 502, 500, 497, 495, 493, 492, 490, 488, 486, 484, 482};
    __constant__ int cHsvDivTable256[256] = {0, 174763, 87381, 58254, 43691, 34953, 29127, 24966, 21845, 19418, 17476, 15888, 14564, 13443, 12483, 11651, 10923, 10280, 9709, 9198, 8738, 8322, 7944, 7598, 7282, 6991, 6722, 6473, 6242, 6026, 5825, 5638, 5461, 5296, 5140, 4993, 4855, 4723, 4599, 4481, 4369, 4263, 4161, 4064, 3972, 3884, 3799, 3718, 3641, 3567, 3495, 3427, 3361, 3297, 3236, 3178, 3121, 3066, 3013, 2962, 2913, 2865, 2819, 2774, 2731, 2689, 2648, 2608, 2570, 2533, 2497, 2461, 2427, 2394, 2362, 2330, 2300, 2270, 2241, 2212, 2185, 2158, 2131, 2106, 2081, 2056, 2032, 2009, 1986, 1964, 1942, 1920, 1900, 1879, 1859, 1840, 1820, 1802, 1783, 1765, 1748, 1730, 1713, 1697, 1680, 1664, 1649, 1633, 1618, 1603, 1589, 1574, 1560, 1547, 1533, 1520, 1507, 1494, 1481, 1469, 1456, 1444, 1432, 1421, 1409, 1398, 1387, 1376, 1365, 1355, 1344, 1334, 1324, 1314, 1304, 1295, 1285, 1276, 1266, 1257, 1248, 1239, 1231, 1222, 1214, 1205, 1197, 1189, 1181, 1173, 1165, 1157, 1150, 1142, 1135, 1128, 1120, 1113, 1106, 1099, 1092, 1085, 1079, 1072, 1066, 1059, 1053, 1046, 1040, 1034, 1028, 1022, 1016, 1010, 1004, 999, 993, 987, 982, 976, 971, 966, 960, 955, 950, 945, 940, 935, 930, 925, 920, 915, 910, 906, 901, 896, 892, 887, 883, 878, 874, 869, 865, 861, 857, 853, 848, 844, 840, 836, 832, 828, 824, 820, 817, 813, 809, 805, 802, 798, 794, 791, 787, 784, 780, 777, 773, 770, 767, 763, 760, 757, 753, 750, 747, 744, 741, 737, 734, 731, 728, 725, 722, 719, 716, 713, 710, 708, 705, 702, 699, 696, 694, 691, 688, 685};

    template <typename D> __device__ void RGB2HSVConvert(const uchar* src, D& dst, int bidx, int hr)
    {
        const int hsv_shift = 12;
        const int* hdiv_table = hr == 180 ? cHsvDivTable180 : cHsvDivTable256;

        int b = src[bidx], g = src[1], r = src[bidx^2];
        int h, s, v = b;
        int vmin = b, diff;
        int vr, vg;

        v = max(v, g);
        v = max(v, r);
        vmin = min(vmin, g);
        vmin = min(vmin, r);

        diff = v - vmin;
        vr = v == r ? -1 : 0;
        vg = v == g ? -1 : 0;

        s = (diff * cHsvDivTable[v] + (1 << (hsv_shift-1))) >> hsv_shift;
        h = (vr & (g - b)) + (~vr & ((vg & (b - r + 2 * diff)) + ((~vg) & (r - g + 4 * diff))));
        h = (h * hdiv_table[diff] + (1 << (hsv_shift-1))) >> hsv_shift;
        h += h < 0 ? hr : 0;

        dst.x = saturate_cast<uchar>(h);
        dst.y = (uchar)s;
        dst.z = (uchar)v;
    }
    template<typename D> __device__ void RGB2HSVConvert(const float* src, D& dst, int bidx, int hr)
    {
        const float hscale = hr * (1.f / 360.f);

        float b = src[bidx], g = src[1], r = src[bidx^2];
        float h, s, v;

        float vmin, diff;

        v = vmin = r;
        v = fmax(v, g);
        v = fmax(v, b);
        vmin = fmin(vmin, g);
        vmin = fmin(vmin, b);

        diff = v - vmin;
        s = diff / (float)(fabs(v) + numeric_limits_gpu<float>::epsilon());
        diff = (float)(60. / (diff + numeric_limits_gpu<float>::epsilon()));

        if (v == r)
            h = (g - b) * diff;
        else if (v == g)
            h = (b - r) * diff + 120.f;
        else
            h = (r - g) * diff + 240.f;

        if (h < 0) h += 360.f;

        dst.x = h * hscale;
        dst.y = s;
        dst.z = v;
    }

    template <int SRCCN, int DSTCN, typename T> struct RGB2HSV
    {
        typedef typename TypeVec<T, SRCCN>::vec_t src_t;
        typedef typename TypeVec<T, DSTCN>::vec_t dst_t;

        RGB2HSV(int bidx, int hr) : bidx(bidx), hr(hr) {}

        __device__ __forceinline__ dst_t operator()(const src_t& src) const
        {
            dst_t dst;
            RGB2HSVConvert(&src.x, dst, bidx, hr);
            return dst;
        }

    private:
        int bidx;
        int hr;
    };

    __constant__ int cHsvSectorData[6][3] =
    {
        {1,3,0}, {1,0,2}, {3,0,1}, {0,2,1}, {0,1,3}, {2,1,0}
    };

    template <typename T> __device__ void HSV2RGBConvert(const T& src, float* dst, int bidx, int hr)
    {
        const float hscale = 6.f / hr;

        float h = src.x, s = src.y, v = src.z;
        float b, g, r;

        if( s == 0 )
            b = g = r = v;
        else
        {
            float tab[4];
            int sector;
            h *= hscale;
            if( h < 0 )
                do h += 6; while( h < 0 );
            else if( h >= 6 )
                do h -= 6; while( h >= 6 );
            sector = __float2int_rd(h);
            h -= sector;

            tab[0] = v;
            tab[1] = v*(1.f - s);
            tab[2] = v*(1.f - s*h);
            tab[3] = v*(1.f - s*(1.f - h));

            b = tab[cHsvSectorData[sector][0]];
            g = tab[cHsvSectorData[sector][1]];
            r = tab[cHsvSectorData[sector][2]];
        }

        dst[bidx] = b;
        dst[1] = g;
        dst[bidx^2] = r;
    }
    template <typename T> __device__ void HSV2RGBConvert(const T& src, uchar* dst, int bidx, int hr)
    {
        float3 buf;

        buf.x = src.x;
        buf.y = src.y * (1.f/255.f);
        buf.z = src.z * (1.f/255.f);

        HSV2RGBConvert(buf, &buf.x, bidx, hr);

        dst[0] = saturate_cast<uchar>(buf.x * 255.f);
        dst[1] = saturate_cast<uchar>(buf.y * 255.f);
        dst[2] = saturate_cast<uchar>(buf.z * 255.f);
    }

    template <int SRCCN, int DSTCN, typename T> struct HSV2RGB
    {
        typedef typename TypeVec<T, SRCCN>::vec_t src_t;
        typedef typename TypeVec<T, DSTCN>::vec_t dst_t;

        HSV2RGB(int bidx, int hr) : bidx(bidx), hr(hr) {}

        __device__ __forceinline__ dst_t operator()(const src_t& src) const
        {
            dst_t dst;
            HSV2RGBConvert(src, &dst.x, bidx, hr);
            setAlpha(dst, ColorChannel<T>::max());
            return dst;
        }

    private:
        int bidx;
        int hr;
    };

    template <typename T, int SRCCN, int DSTCN>
    void RGB2HSV_caller(const DevMem2D& src, const DevMem2D& dst, int bidx, int hrange, cudaStream_t stream)
    {
        RGB2HSV<SRCCN, DSTCN, T> cvt(bidx, hrange);
        callConvert(src, dst, cvt, stream);
    }

    void RGB2HSV_gpu_8u(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, int bidx, int hrange, cudaStream_t stream)
    {
        typedef void (*RGB2HSV_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, int hrange, cudaStream_t stream);
        static const RGB2HSV_caller_t RGB2HSV_callers[2][2] =
        {
            {RGB2HSV_caller<uchar, 3, 3>, RGB2HSV_caller<uchar, 3, 4>},
            {RGB2HSV_caller<uchar, 4, 3>, RGB2HSV_caller<uchar, 4, 4>}
        };

        RGB2HSV_callers[srccn-3][dstcn-3](src, dst, bidx, hrange, stream);
    }

    void RGB2HSV_gpu_32f(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, int bidx, int hrange, cudaStream_t stream)
    {
        typedef void (*RGB2HSV_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, int hrange, cudaStream_t stream);
        static const RGB2HSV_caller_t RGB2HSV_callers[2][2] =
        {
            {RGB2HSV_caller<float, 3, 3>, RGB2HSV_caller<float, 3, 4>},
            {RGB2HSV_caller<float, 4, 3>, RGB2HSV_caller<float, 4, 4>}
        };

        RGB2HSV_callers[srccn-3][dstcn-3](src, dst, bidx, hrange, stream);
    }

    template <typename T, int SRCCN, int DSTCN>
    void HSV2RGB_caller(const DevMem2D& src, const DevMem2D& dst, int bidx, int hrange, cudaStream_t stream)
    {
        HSV2RGB<SRCCN, DSTCN, T> cvt(bidx, hrange);
        callConvert(src, dst, cvt, stream);
    }

    void HSV2RGB_gpu_8u(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, int bidx, int hrange, cudaStream_t stream)
    {
        typedef void (*HSV2RGB_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, int hrange, cudaStream_t stream);
        static const HSV2RGB_caller_t HSV2RGB_callers[2][2] =
        {
            {HSV2RGB_caller<uchar, 3, 3>, HSV2RGB_caller<uchar, 3, 4>},
            {HSV2RGB_caller<uchar, 4, 3>, HSV2RGB_caller<uchar, 4, 4>}
        };

        HSV2RGB_callers[srccn-3][dstcn-3](src, dst, bidx, hrange, stream);
    }

    void HSV2RGB_gpu_32f(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, int bidx, int hrange, cudaStream_t stream)
    {
        typedef void (*HSV2RGB_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, int hrange, cudaStream_t stream);
        static const HSV2RGB_caller_t HSV2RGB_callers[2][2] =
        {
            {HSV2RGB_caller<float, 3, 3>, HSV2RGB_caller<float, 3, 4>},
            {HSV2RGB_caller<float, 4, 3>, HSV2RGB_caller<float, 4, 4>}
        };

        HSV2RGB_callers[srccn-3][dstcn-3](src, dst, bidx, hrange, stream);
    }

/////////////////////////////////////// RGB <-> HLS ////////////////////////////////////////

    template <typename D> __device__ void RGB2HLSConvert(const float* src, D& dst, int bidx, int hr)
    {
        const float hscale = hr * (1.f / 360.f);

        float b = src[bidx], g = src[1], r = src[bidx^2];
        float h = 0.f, s = 0.f, l;
        float vmin, vmax, diff;

        vmax = vmin = r;
        vmax = fmax(vmax, g);
        vmax = fmax(vmax, b);
        vmin = fmin(vmin, g);
        vmin = fmin(vmin, b);

        diff = vmax - vmin;
        l = (vmax + vmin) * 0.5f;

        if (diff > numeric_limits_gpu<float>::epsilon())
        {
            s = l < 0.5f ? diff / (vmax + vmin) : diff / (2.0f - vmax - vmin);
            diff = 60.f / diff;

            if (vmax == r)
                h = (g - b)*diff;
            else if (vmax == g)
                h = (b - r)*diff + 120.f;
            else
                h = (r - g)*diff + 240.f;

            if (h < 0.f) h += 360.f;
        }

        dst.x = h * hscale;
        dst.y = l;
        dst.z = s;
    }
    template <typename D> __device__ void RGB2HLSConvert(const uchar* src, D& dst, int bidx, int hr)
    {
        float3 buf;

        buf.x = src[0]*(1.f/255.f);
        buf.y = src[1]*(1.f/255.f);
        buf.z = src[2]*(1.f/255.f);

        RGB2HLSConvert(&buf.x, buf, bidx, hr);

        dst.x = saturate_cast<uchar>(buf.x);
        dst.y = saturate_cast<uchar>(buf.y*255.f);
        dst.z = saturate_cast<uchar>(buf.z*255.f);
    }

    template <int SRCCN, int DSTCN, typename T> struct RGB2HLS
    {
        typedef typename TypeVec<T, SRCCN>::vec_t src_t;
        typedef typename TypeVec<T, DSTCN>::vec_t dst_t;

        RGB2HLS(int bidx, int hr) : bidx(bidx), hr(hr) {}

        __device__ __forceinline__ dst_t operator()(const src_t& src) const
        {
            dst_t dst;
            RGB2HLSConvert(&src.x, dst, bidx, hr);
            return dst;
        }

    private:
        int bidx;
        int hr;
    };

    __constant__ int cHlsSectorData[6][3] =
    {
        {1,3,0}, {1,0,2}, {3,0,1}, {0,2,1}, {0,1,3}, {2,1,0}
    };

    template <typename T> __device__ void HLS2RGBConvert(const T& src, float* dst, int bidx, int hr)
    {
        const float hscale = 6.0f / hr;

        float h = src.x, l = src.y, s = src.z;
        float b, g, r;

        if (s == 0)
            b = g = r = l;
        else
        {
            float tab[4];
            int sector;

            float p2 = l <= 0.5f ? l * (1 + s) : l + s - l * s;
            float p1 = 2 * l - p2;

            h *= hscale;

            if( h < 0 )
                do h += 6; while( h < 0 );
            else if( h >= 6 )
                do h -= 6; while( h >= 6 );

            sector = __float2int_rd(h);
            h -= sector;

            tab[0] = p2;
            tab[1] = p1;
            tab[2] = p1 + (p2 - p1) * (1 - h);
            tab[3] = p1 + (p2 - p1) * h;

            b = tab[cHlsSectorData[sector][0]];
            g = tab[cHlsSectorData[sector][1]];
            r = tab[cHlsSectorData[sector][2]];
        }

        dst[bidx] = b;
        dst[1] = g;
        dst[bidx^2] = r;
    }
    template <typename T> __device__ void HLS2RGBConvert(const T& src, uchar* dst, int bidx, int hr)
    {
        float3 buf;

        buf.x = src.x;
        buf.y = src.y*(1.f/255.f);
        buf.z = src.z*(1.f/255.f);

        HLS2RGBConvert(buf, &buf.x, bidx, hr);

        dst[0] = saturate_cast<uchar>(buf.x*255.f);
        dst[1] = saturate_cast<uchar>(buf.y*255.f);
        dst[2] = saturate_cast<uchar>(buf.z*255.f);
    }

    template <int SRCCN, int DSTCN, typename T> struct HLS2RGB
    {
        typedef typename TypeVec<T, SRCCN>::vec_t src_t;
        typedef typename TypeVec<T, DSTCN>::vec_t dst_t;

        HLS2RGB(int bidx, int hr) : bidx(bidx), hr(hr) {}

        __device__ __forceinline__ dst_t operator()(const src_t& src) const
        {
            dst_t dst;
            HLS2RGBConvert(src, &dst.x, bidx, hr);
            setAlpha(dst, ColorChannel<T>::max());
            return dst;
        }

    private:
        int bidx;
        int hr;
    };

    template <typename T, int SRCCN, int DSTCN>
    void RGB2HLS_caller(const DevMem2D& src, const DevMem2D& dst, int bidx, int hrange, cudaStream_t stream)
    {
        RGB2HLS<SRCCN, DSTCN, T> cvt(bidx, hrange);
        callConvert(src, dst, cvt, stream);
    }

    void RGB2HLS_gpu_8u(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, int bidx, int hrange, cudaStream_t stream)
    {
        typedef void (*RGB2HLS_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, int hrange, cudaStream_t stream);
        static const RGB2HLS_caller_t RGB2HLS_callers[2][2] =
        {
            {RGB2HLS_caller<uchar, 3, 3>, RGB2HLS_caller<uchar, 3, 4>},
            {RGB2HLS_caller<uchar, 4, 3>, RGB2HLS_caller<uchar, 4, 4>}
        };

        RGB2HLS_callers[srccn-3][dstcn-3](src, dst, bidx, hrange, stream);
    }

    void RGB2HLS_gpu_32f(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, int bidx, int hrange, cudaStream_t stream)
    {
        typedef void (*RGB2HLS_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, int hrange, cudaStream_t stream);
        static const RGB2HLS_caller_t RGB2HLS_callers[2][2] =
        {
            {RGB2HLS_caller<float, 3, 3>, RGB2HLS_caller<float, 3, 4>},
            {RGB2HLS_caller<float, 4, 3>, RGB2HLS_caller<float, 4, 4>}
        };

        RGB2HLS_callers[srccn-3][dstcn-3](src, dst, bidx, hrange, stream);
    }


    template <typename T, int SRCCN, int DSTCN>
    void HLS2RGB_caller(const DevMem2D& src, const DevMem2D& dst, int bidx, int hrange, cudaStream_t stream)
    {
        HLS2RGB<SRCCN, DSTCN, T> cvt(bidx, hrange);
        callConvert(src, dst, cvt, stream);
    }

    void HLS2RGB_gpu_8u(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, int bidx, int hrange, cudaStream_t stream)
    {
        typedef void (*HLS2RGB_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, int hrange, cudaStream_t stream);
        static const HLS2RGB_caller_t HLS2RGB_callers[2][2] =
        {
            {HLS2RGB_caller<uchar, 3, 3>, HLS2RGB_caller<uchar, 3, 4>},
            {HLS2RGB_caller<uchar, 4, 3>, HLS2RGB_caller<uchar, 4, 4>}
        };

        HLS2RGB_callers[srccn-3][dstcn-3](src, dst, bidx, hrange, stream);
    }

    void HLS2RGB_gpu_32f(const DevMem2D& src, int srccn, const DevMem2D& dst, int dstcn, int bidx, int hrange, cudaStream_t stream)
    {
        typedef void (*HLS2RGB_caller_t)(const DevMem2D& src, const DevMem2D& dst, int bidx, int hrange, cudaStream_t stream);
        static const HLS2RGB_caller_t HLS2RGB_callers[2][2] =
        {
            {HLS2RGB_caller<float, 3, 3>, HLS2RGB_caller<float, 3, 4>},
            {HLS2RGB_caller<float, 4, 3>, HLS2RGB_caller<float, 4, 4>}
        };

        HLS2RGB_callers[srccn-3][dstcn-3](src, dst, bidx, hrange, stream);
    }
}}}
