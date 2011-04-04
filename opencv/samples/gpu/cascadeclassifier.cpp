// WARNING: this sample is under construction! Use it on your own risk.

#include <opencv2/contrib/contrib.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/gpu/gpu.hpp>

#include <iostream>
#include <iomanip>

using namespace std;
using namespace cv;
using namespace cv::gpu;


void help()
{
    cout << "Usage: ./cascadeclassifier <cascade_file> <image_or_video_or_cameraid>\n"
            "Using OpenCV version " << CV_VERSION << endl << endl;
}


void DetectAndDraw(Mat& img, CascadeClassifier_GPU& cascade);


String cascadeName = "../../data/haarcascades/haarcascade_frontalface_alt.xml";
String nestedCascadeName = "../../data/haarcascades/haarcascade_eye_tree_eyeglasses.xml";


template<class T> void convertAndResize(const T& src, T& gray, T& resized, double scale)
{
    if (src.channels() == 3)
    {
        cvtColor( src, gray, CV_BGR2GRAY );
    }
    else
    {
        gray = src;
    }

    Size sz(cvRound(gray.cols * scale), cvRound(gray.rows * scale));

    if (scale != 1)
    {
        resize(gray, resized, sz);
    }
    else
    {
        resized = gray;
    }
}


void matPrint(Mat &img, int lineOffsY, Scalar fontColor, const ostringstream &ss)
{
    int fontFace = FONT_HERSHEY_PLAIN;
    double fontScale = 1.5;
    int fontThickness = 2;
    Size fontSize = cv::getTextSize("T[]", fontFace, fontScale, fontThickness, 0);

    Point org;
    org.x = 1;
    org.y = 3 * fontSize.height * (lineOffsY + 1) / 2;
    putText(img, ss.str(), org, fontFace, fontScale, fontColor, fontThickness);
}


void displayState(Mat &canvas, bool bHelp, bool bGpu, bool bLargestFace, bool bFilter, double fps)
{
    Scalar fontColorRed = CV_RGB(255,0,0);
    Scalar fontColorNV  = CV_RGB(118,185,0);

    ostringstream ss;
    ss << "[" << canvas.cols << "x" << canvas.rows << "], " <<
        (bGpu ? "GPU, " : "CPU, ") <<
        (bLargestFace ? "OneFace, " : "MultiFace, ") <<
        (bFilter ? "Filter:ON, " : "Filter:OFF, ") <<
        "FPS = " << setprecision(1) << fixed << fps;

    matPrint(canvas, 0, fontColorRed, ss);

    if (bHelp)
    {
        matPrint(canvas, 1, fontColorNV, ostringstream("Space - switch GPU / CPU"));
        matPrint(canvas, 2, fontColorNV, ostringstream("M - switch OneFace / MultiFace"));
        matPrint(canvas, 3, fontColorNV, ostringstream("F - toggle rectangles Filter (only in MultiFace)"));
        matPrint(canvas, 4, fontColorNV, ostringstream("H - toggle hotkeys help"));
        matPrint(canvas, 5, fontColorNV, ostringstream("1/Q - increase/decrease scale"));
    }
    else
    {
        matPrint(canvas, 1, fontColorNV, ostringstream("H - toggle hotkeys help"));
    }
}


int main(int argc, const char *argv[])
{
    if (argc != 3)
    {
        return help(), -1;
    }

    if (getCudaEnabledDeviceCount() == 0)
    {
        return cerr << "No GPU found or the library is compiled without GPU support" << endl, -1;
    }

    VideoCapture capture;

    string cascadeName = argv[1];
    string inputName = argv[2];

    CascadeClassifier_GPU cascade_gpu;
    if (!cascade_gpu.load(cascadeName))
    {
        return cerr << "ERROR: Could not load cascade classifier \"" << cascadeName << "\"" << endl, help(), -1;
    }

    CascadeClassifier cascade_cpu;
    if (!cascade_cpu.load(cascadeName))
    {
        return cerr << "ERROR: Could not load cascade classifier \"" << cascadeName << "\"" << endl, help(), -1;
    }

    Mat image = imread(inputName);

    if (image.empty())
    {
        if (!capture.open(inputName))
        {
            int camid = 0;
            sscanf(inputName.c_str(), "%d", &camid);
            if (!capture.open(camid))
            {
                cout << "Can't open source" << endl;
                return help(), -1;
            }
        }
    }

    namedWindow("result", 1);

    Mat frame, frame_cpu, gray_cpu, resized_cpu, faces_downloaded, frameDisp;
    vector<Rect> facesBuf_cpu;

    GpuMat frame_gpu, gray_gpu, resized_gpu, facesBuf_gpu;

    /* parameters */
    bool useGPU = true;
    double scaleFactor = 1.0;
    bool findLargestObject = false;
    bool filterRects = true;
    bool helpScreen = false;

    int detections_num;
    for (;;)
    {
        if (capture.isOpened())
        {
            capture >> frame;
            if (frame.empty())
            {
                break;
            }
        }

        (image.empty() ? frame : image).copyTo(frame_cpu);
        frame_gpu.upload(image.empty() ? frame : image);

        convertAndResize(frame_gpu, gray_gpu, resized_gpu, scaleFactor);
        convertAndResize(frame_cpu, gray_cpu, resized_cpu, scaleFactor);

        TickMeter tm;
        tm.start();

        if (useGPU)
        {
            cascade_gpu.visualizeInPlace = true;
            cascade_gpu.findLargestObject = findLargestObject;

            detections_num = cascade_gpu.detectMultiScale(resized_gpu, facesBuf_gpu, 1.2, filterRects ? 4 : 0);
            facesBuf_gpu.colRange(0, detections_num).download(faces_downloaded);
        }
        else
        {
            Size minSize = cascade_gpu.getClassifierSize();
            cascade_cpu.detectMultiScale(resized_cpu, facesBuf_cpu, 1.2, filterRects ? 4 : 0, (findLargestObject ? CV_HAAR_FIND_BIGGEST_OBJECT : 0) | CV_HAAR_SCALE_IMAGE, minSize);
            detections_num = (int)facesBuf_cpu.size();
        }

        if (!useGPU)
        {
            if (detections_num)
            {
                for (int i = 0; i < detections_num; ++i)
                {
                    rectangle(resized_cpu, facesBuf_cpu[i], Scalar(255));
                }
            }
        }

        if (useGPU)
        {
            resized_gpu.download(resized_cpu);
        }

        tm.stop();
        double detectionTime = tm.getTimeMilli();
        double fps = 1000 / detectionTime;

        //print detections to console
        cout << setfill(' ') << setprecision(2);
        cout << setw(6) << fixed << fps << " FPS, " << detections_num << " det";
        if ((filterRects || findLargestObject) && detections_num > 0)
        {
            Rect *faceRects = useGPU ? faces_downloaded.ptr<Rect>() : &facesBuf_cpu[0];
            for (int i = 0; i < min(detections_num, 2); ++i)
            {
                cout << ", [" << setw(4) << faceRects[i].x
                     << ", " << setw(4) << faceRects[i].y
                     << ", " << setw(4) << faceRects[i].width
                     << ", " << setw(4) << faceRects[i].height << "]";
            }
        }
        cout << endl;

        cvtColor(resized_cpu, frameDisp, CV_GRAY2BGR);
        displayState(frameDisp, helpScreen, useGPU, findLargestObject, filterRects, fps);
        imshow("result", frameDisp);

        int key = waitKey(5);
        if (key == 27)
        {
            break;
        }

        switch ((char)key)
        {
        case ' ':
            useGPU = !useGPU;
            break;
        case 'm':
        case 'M':
            findLargestObject = !findLargestObject;
            break;
        case 'f':
        case 'F':
            filterRects = !filterRects;
            break;
        case '1':
            scaleFactor *= 1.05;
            break;
        case 'q':
        case 'Q':
            scaleFactor /= 1.05;
            break;
        case 'h':
        case 'H':
            helpScreen = !helpScreen;
            break;
        }
    }

    return 0;
}
