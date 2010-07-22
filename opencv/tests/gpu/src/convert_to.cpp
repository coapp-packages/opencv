#include "gputest.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <iterator>
#include <limits>
#include <numeric>

using namespace cv;
using namespace std;
using namespace gpu;

class CV_GpuMatOpConvertTo : public CvTest
{
    public:
        CV_GpuMatOpConvertTo();
        ~CV_GpuMatOpConvertTo();
    protected:
        void run(int);
};

CV_GpuMatOpConvertTo::CV_GpuMatOpConvertTo(): CvTest( "GpuMatOperatorConvertTo", "convertTo" ) {}
CV_GpuMatOpConvertTo::~CV_GpuMatOpConvertTo() {}

void CV_GpuMatOpConvertTo::run( int /* start_from */)
{
    const Size img_size(67, 35);

    const int types[] = {CV_8U, CV_8S, CV_16U, CV_16S, CV_32S, CV_32F, CV_64F/**/};
    const int types_num = sizeof(types) / sizeof(int);

    const char* types_str[] = {"CV_8U", "CV_8S", "CV_16U", "CV_16S", "CV_32S", "CV_32F", "CV_64F"};

    bool passed = true;

    for (int i = 0; i < types_num && passed; ++i)
    {
        for (int j = 0; j < types_num && passed; ++j)
        {
            for (int c = 1; c < 2 && passed; ++c)
            {
                //if (i == j)
                  //  continue;

                const int src_type = CV_MAKETYPE(types[i], c);
                const int dst_type = types[j];
                const double alpha = (double)rand() / RAND_MAX * 10.0;
                const double beta = (double)rand() / RAND_MAX * 10.0;

                Mat cpumatsrc(img_size, src_type);
                randu(cpumatsrc, Scalar::all(0), Scalar::all(10));
                GpuMat gpumatsrc(cpumatsrc);
                Mat cpumatdst;
                GpuMat gpumatdst;
                
                //double cput = (double)getTickCount();
                cpumatsrc.convertTo(cpumatdst, dst_type, alpha, beta);
                //cput = ((double)getTickCount() - cput) / getTickFrequency();

                //double gput = (double)getTickCount();
                gpumatsrc.convertTo(gpumatdst, dst_type, alpha, beta);
                //gput = ((double)getTickCount() - gput) / getTickFrequency();

                /*cout << "convertTo time: " << endl;
                cout << "CPU time: " << cput << endl;
                cout << "GPU time: " << gput << endl;/**/               

                double r = norm(cpumatdst, gpumatdst, NORM_L1);
                if (r > 1)
                {
                    /*namedWindow("CPU"); 
                    imshow("CPU", cpumatdst); 
                    namedWindow("GPU"); 
                    imshow("GPU", gpumatdst); 
                    waitKey();/**/
                    
                    cout << "Failed:" << endl;
                    cout << "\tr = " << r << endl;
                    cout << "\tSRC_TYPE=" << types_str[i] << "C" << c << " DST_TYPE=" << types_str[j] << endl;/**/

                    passed = false;
                }
            }
        }
    }
    ts->set_failed_test_info(passed ? CvTS::OK : CvTS::FAIL_GENERIC);
}

CV_GpuMatOpConvertTo CV_GpuMatOpConvertTo_test;
