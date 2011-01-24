#include <iomanip>
#include "performance.h"

using namespace std;
using namespace cv;


void Test::gen(Mat& mat, int rows, int cols, int type, Scalar low, Scalar high)
{   
    mat.create(rows, cols, type);

    RNG rng(0);
    rng.fill(mat, RNG::UNIFORM, low, high);
}


void Test::gen(Mat& mat, int rows, int cols, int type)
{   
    mat.create(rows, cols, type);

    Mat mat8u(rows, cols * mat.elemSize(), CV_8U, mat.data, mat.step);

    RNG rng(0);
    rng.fill(mat, RNG::UNIFORM, Scalar(0), Scalar(256));
}


void TestSystem::run()
{
    cout << setiosflags(ios_base::left);
    cout << "    " << setw(10) << "CPU, ms" << setw(10) << "GPU, ms" 
        << setw(10) << "SPEEDUP" << "DESCRIPTION\n";
    cout << resetiosflags(ios_base::left);

    vector<Test*>::iterator it = tests_.begin();
    for (; it != tests_.end(); ++it)
    {
        can_flush_ = false;
        Test* test = *it;

        cout << endl << test->name() << ":\n";
        test->run();

        flush();
    }

    cout << setiosflags(ios_base::fixed | ios_base::left);
    cout << "\nCPU Total: " << setprecision(3) << cpu_total_ / getTickFrequency() << " sec\n";
    cout << "GPU Total: " << setprecision(3) << gpu_total_ / getTickFrequency() << " sec (x" 
        << setprecision(3) << (double)cpu_total_ / gpu_total_ << ")\n";
    cout << resetiosflags(ios_base::fixed | ios_base::left);
}


void TestSystem::flush()
{
    if (!can_flush_)
        return;

    int cpu_time = static_cast<int>(cpu_elapsed_ / getTickFrequency() * 1000.0);
    int gpu_time = static_cast<int>(gpu_elapsed_ / getTickFrequency() * 1000.0);
    double speedup = (double)cpu_time / gpu_time;

    cpu_elapsed_ = 0;
    gpu_elapsed_ = 0;

    cout << "    " << setiosflags(ios_base::fixed | ios_base::left);

    stringstream stream;
    stream << cpu_time;
    cout << setw(10) << stream.str();

    stream.str("");
    stream << gpu_time;
    cout << setw(10) << stream.str();

    stream.str("");
    stream << "x" << setprecision(3) << speedup;
    cout << setw(10) << stream.str();

    cout << description_.str();
    description_.str("");

    cout << resetiosflags(ios_base::fixed | ios_base::left) << endl;
    can_flush_ = false;
}


int main()
{
    TestSystem::instance()->run();
    return 0;
}