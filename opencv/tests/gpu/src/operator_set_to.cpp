#include "gputest.hpp"
#include "highgui.h"
#include <string>
#include <iostream>
#include <fstream>
#include <iterator>
#include <limits>
#include <numeric>
#include <iomanip> // for  cout << setw()

using namespace cv;
using namespace std;
using namespace gpu;

class CV_GpuMatOpSetTo : public CvTest
{
    public:
        CV_GpuMatOpSetTo();
        ~CV_GpuMatOpSetTo();
    protected:
        void print_mat(cv::Mat & mat, std::string name = "cpu mat");
        void print_mat(gpu::GpuMat & mat, std::string name = "gpu mat");
        void run(int);

        bool compare_matrix(cv::Mat & cpumat, gpu::GpuMat & gpumat);

    private:
        int rows;
        int cols;
        Scalar s;
};

CV_GpuMatOpSetTo::CV_GpuMatOpSetTo(): CvTest( "GPU-MatOperatorSetTo", "setTo" )
{
    rows = 256;
    cols = 124;

    s.val[0] = 127.0;
    s.val[1] = 127.0;
    s.val[2] = 127.0;
    s.val[3] = 127.0;

    //#define PRINT_MATRIX
}

CV_GpuMatOpSetTo::~CV_GpuMatOpSetTo() {}

void CV_GpuMatOpSetTo::print_mat(cv::Mat & mat, std::string name )
{
    cv::imshow(name, mat);
}

void CV_GpuMatOpSetTo::print_mat(gpu::GpuMat & mat, std::string name)
{
    cv::Mat newmat;
    mat.download(newmat);
    print_mat(newmat, name);
}

bool CV_GpuMatOpSetTo::compare_matrix(cv::Mat & cpumat, gpu::GpuMat & gpumat)
{
    //int64 time = getTickCount();
    cpumat.setTo(s);
    //int64 time1 = getTickCount();
    gpumat.setTo(s);
    //int64 time2 = getTickCount();

    //std::cout << "\ntime cpu: " << std::fixed << std::setprecision(12) << double((time1 - time)  / (double)getTickFrequency());
    //std::cout << "\ntime gpu: " << std::fixed << std::setprecision(12) << double((time2 - time1) / (double)getTickFrequency());
    //std::cout << "\n";

#ifdef PRINT_MATRIX
    print_mat(cpumat);
    print_mat(gpumat);
    cv::waitKey(0);
#endif

    double ret = norm(cpumat, gpumat);

    if (ret < 1.0)
        return true;
    else
    {
        std::cout << "return : " << ret << "\n";
        return false;
    }
}

void CV_GpuMatOpSetTo::run( int /* start_from */)
{
    bool is_test_good = true;

    for (int i = 0; i < 7; i++)
    {
        Mat cpumat(rows, cols, i, Scalar::all(0));
        GpuMat gpumat(cpumat);
        is_test_good &= compare_matrix(cpumat, gpumat);
    }

    if (is_test_good == true)
        ts->set_failed_test_info(CvTS::OK);
    else
        ts->set_failed_test_info(CvTS::FAIL_GENERIC);
}

CV_GpuMatOpSetTo CV_GpuMatOpSetTo_test;
