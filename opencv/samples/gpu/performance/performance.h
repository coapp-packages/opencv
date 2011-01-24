#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <opencv2/core/core.hpp>

class Test
{
public:
    Test(const std::string& name): name_(name) {}

    const std::string& name() const { return name_; }

    void gen(cv::Mat& mat, int rows, int cols, int type,
             cv::Scalar low, cv::Scalar high);

    void gen(cv::Mat& mat, int rows, int cols, int type);

    virtual void run() = 0;

private:
    std::string name_;
};


class TestSystem
{
public:
    static TestSystem* instance()
    {
        static TestSystem me;
        return &me;
    }

    void add(Test* test) { tests_.push_back(test); }

    void run();

    void cpuOn() { cpu_started_ = cv::getTickCount(); }

    void cpuOff() 
    {
        int64 delta = cv::getTickCount() - cpu_started_;
        cpu_elapsed_ += delta;
        cpu_total_ += delta; 
        can_flush_ = true;
    }  

    void gpuOn() { gpu_started_ = cv::getTickCount(); }

    void gpuOff() 
    {
        int64 delta = cv::getTickCount() - gpu_started_;
        gpu_elapsed_ += delta;
        gpu_total_ += delta; 
        can_flush_ = true;
    }

    // Ends current subtest and starts new one
    std::stringstream& subtest()
    {
        flush();
        return description_;
    }

private:
    TestSystem(): can_flush_(false),
                  cpu_elapsed_(0), cpu_total_(0), 
                  gpu_elapsed_(0), gpu_total_(0) {};

    void flush();

    std::vector<Test*> tests_;

    // Current test (subtest) description
    std::stringstream description_;

    bool can_flush_;

    int64 cpu_started_, cpu_elapsed_, cpu_total_;
    int64 gpu_started_, gpu_elapsed_, gpu_total_;
};


#define TEST(name) \
    struct name##_test: public Test \
    { \
        name##_test(): Test(#name) { TestSystem::instance()->add(this); } \
        void run(); \
    } name##_test_instance; \
    void name##_test::run()


#define CPU_ON TestSystem::instance()->cpuOn()
#define GPU_ON TestSystem::instance()->gpuOn()
#define CPU_OFF TestSystem::instance()->cpuOff()
#define GPU_OFF TestSystem::instance()->gpuOff()
#define SUBTEST TestSystem::instance()->subtest()
#define DESCRIPTION TestSystem::instance()->subtest()