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

#ifndef __OPENCV_FEATURES_2D_HPP__
#define __OPENCV_FEATURES_2D_HPP__

#include "opencv2/core/core.hpp"

#ifdef __cplusplus
#include <limits>

extern "C" {
#endif

typedef struct CvSURFPoint
{
    CvPoint2D32f pt;
    int laplacian;
    int size;
    float dir;
    float hessian;
} CvSURFPoint;

CV_INLINE CvSURFPoint cvSURFPoint( CvPoint2D32f pt, int laplacian,
                                   int size, float dir CV_DEFAULT(0),
                                   float hessian CV_DEFAULT(0))
{
    CvSURFPoint kp;
    kp.pt = pt;
    kp.laplacian = laplacian;
    kp.size = size;
    kp.dir = dir;
    kp.hessian = hessian;
    return kp;
}

typedef struct CvSURFParams
{
    int extended;
    double hessianThreshold;

    int nOctaves;
    int nOctaveLayers;
} CvSURFParams;

CVAPI(CvSURFParams) cvSURFParams( double hessianThreshold, int extended CV_DEFAULT(0) );

// If useProvidedKeyPts!=0, keypoints are not detected, but descriptors are computed
//  at the locations provided in keypoints (a CvSeq of CvSURFPoint).
CVAPI(void) cvExtractSURF( const CvArr* img, const CvArr* mask,
                           CvSeq** keypoints, CvSeq** descriptors,
                           CvMemStorage* storage, CvSURFParams params, int useProvidedKeyPts CV_DEFAULT(0)  );

/*!
 Maximal Stable Regions Parameters
*/
typedef struct CvMSERParams
{
    //! delta, in the code, it compares (size_{i}-size_{i-delta})/size_{i-delta}
    int delta;
    //! prune the area which bigger than maxArea
    int maxArea;
    //! prune the area which smaller than minArea
    int minArea;
    //! prune the area have simliar size to its children
    float maxVariation;
    //! trace back to cut off mser with diversity < min_diversity
    float minDiversity;
    
    /////// the next few params for MSER of color image
    
    //! for color image, the evolution steps
    int maxEvolution;
    //! the area threshold to cause re-initialize
    double areaThreshold;
    //! ignore too small margin
    double minMargin;
    //! the aperture size for edge blur
    int edgeBlurSize;
} CvMSERParams;

CVAPI(CvMSERParams) cvMSERParams( int delta CV_DEFAULT(5), int min_area CV_DEFAULT(60),
                           int max_area CV_DEFAULT(14400), float max_variation CV_DEFAULT(.25f),
                           float min_diversity CV_DEFAULT(.2f), int max_evolution CV_DEFAULT(200),
                           double area_threshold CV_DEFAULT(1.01),
                           double min_margin CV_DEFAULT(.003),
                           int edge_blur_size CV_DEFAULT(5) );

// Extracts the contours of Maximally Stable Extremal Regions
CVAPI(void) cvExtractMSER( CvArr* _img, CvArr* _mask, CvSeq** contours, CvMemStorage* storage, CvMSERParams params );


typedef struct CvStarKeypoint
{
    CvPoint pt;
    int size;
    float response;
} CvStarKeypoint;

CV_INLINE CvStarKeypoint cvStarKeypoint(CvPoint pt, int size, float response)
{
    CvStarKeypoint kpt;
    kpt.pt = pt;
    kpt.size = size;
    kpt.response = response;
    return kpt;
}

typedef struct CvStarDetectorParams
{
    int maxSize;
    int responseThreshold;
    int lineThresholdProjected;
    int lineThresholdBinarized;
    int suppressNonmaxSize;
} CvStarDetectorParams;

CV_INLINE CvStarDetectorParams cvStarDetectorParams(
    int maxSize CV_DEFAULT(45),
    int responseThreshold CV_DEFAULT(30),
    int lineThresholdProjected CV_DEFAULT(10),
    int lineThresholdBinarized CV_DEFAULT(8),
    int suppressNonmaxSize CV_DEFAULT(5))
{
    CvStarDetectorParams params;
    params.maxSize = maxSize;
    params.responseThreshold = responseThreshold;
    params.lineThresholdProjected = lineThresholdProjected;
    params.lineThresholdBinarized = lineThresholdBinarized;
    params.suppressNonmaxSize = suppressNonmaxSize;

    return params;
}

CVAPI(CvSeq*) cvGetStarKeypoints( const CvArr* img, CvMemStorage* storage,
        CvStarDetectorParams params CV_DEFAULT(cvStarDetectorParams()));

#ifdef __cplusplus
}

namespace cv
{

// CvAffinePose: defines a parameterized affine transformation of an image patch.
// An image patch is rotated on angle phi (in degrees), then scaled lambda1 times
// along horizontal and lambda2 times along vertical direction, and then rotated again
// on angle (theta - phi).
class CV_EXPORTS CvAffinePose
{
public:
    float phi;
    float theta;
    float lambda1;
    float lambda2;
};

/*!
 The Keypoint Class
 
 The class instance stores a keypoint, i.e. a point feature found by one of many available keypoint detectors, such as
 Harris corner detector, cv::FAST, cv::StarDetector, cv::SURF, cv::SIFT, cv::LDetector etc.
 
 The keypoint is characterized by the 2D position, scale
 (proportional to the diameter of the neighborhood that needs to be taken into account),
 orientation and some other parameters. The keypoint neighborhood is then analyzed by another algorithm that builds a descriptor
 (usually represented as a feature vector). The keypoints representing the same object in different images can then be matched using
 cv::KDTree or another method.
*/
class CV_EXPORTS KeyPoint
{
public:
    //! the default constructor
    KeyPoint() : pt(0,0), size(0), angle(-1), response(0), octave(0), class_id(-1) {}
    //! the full constructor
    KeyPoint(Point2f _pt, float _size, float _angle=-1,
            float _response=0, int _octave=0, int _class_id=-1)
            : pt(_pt), size(_size), angle(_angle),
            response(_response), octave(_octave), class_id(_class_id) {}
    //! another form of the full constructor
    KeyPoint(float x, float y, float _size, float _angle=-1,
            float _response=0, int _octave=0, int _class_id=-1)
            : pt(x, y), size(_size), angle(_angle),
            response(_response), octave(_octave), class_id(_class_id) {}
    //! converts vector of keypoints to vector of points
    static void convert(const std::vector<KeyPoint>& u, std::vector<Point2f>& v);
    //! converts vector of points to the vector of keypoints, where each keypoint is assigned the same size and the same orientation
    static void convert(const std::vector<Point2f>& u, std::vector<KeyPoint>& v,
                        float size=1, float response=1, int octave=0, int class_id=-1);
    Point2f pt; //!< coordinates of the keypoints
    float size; //!< diameter of the meaningfull keypoint neighborhood
    float angle; //!< computed orientation of the keypoint (-1 if not applicable)
    float response; //!< the response by which the most strong keypoints have been selected. Can be used for the further sorting or subsampling
    int octave; //!< octave (pyramid layer) from which the keypoint has been extracted
    int class_id; //!< object class (if the keypoints need to be clustered by an object they belong to) 
};

//! writes vector of keypoints to the file storage
CV_EXPORTS void write(FileStorage& fs, const string& name, const vector<KeyPoint>& keypoints);
//! reads vector of keypoints from the specified file storage node
CV_EXPORTS void read(const FileNode& node, vector<KeyPoint>& keypoints);    

/*!
 SIFT implementation.
 
 The class implements SIFT algorithm by D. Lowe.
*/
class CV_EXPORTS SIFT
{
public:
    struct CommonParams
    {
        static const int DEFAULT_NOCTAVES = 4;
        static const int DEFAULT_NOCTAVE_LAYERS = 3;
        static const int DEFAULT_FIRST_OCTAVE = -1;
        enum{ FIRST_ANGLE = 0, AVERAGE_ANGLE = 1 };

        CommonParams();
        CommonParams( int _nOctaves, int _nOctaveLayers, int _firstOctave, int _angleMode );
        int nOctaves, nOctaveLayers, firstOctave;
        int angleMode;
    };

    struct DetectorParams
    {
        static double GET_DEFAULT_THRESHOLD() { return 0.04 / SIFT::CommonParams::DEFAULT_NOCTAVE_LAYERS / 2.0; }
        static double GET_DEFAULT_EDGE_THRESHOLD() { return 10.0; }

        DetectorParams();
        DetectorParams( double _threshold, double _edgeThreshold );
        double threshold, edgeThreshold;
    };

    struct DescriptorParams
    {
        static double GET_DEFAULT_MAGNIFICATION() { return 3.0; }
        static const bool DEFAULT_IS_NORMALIZE = true;
        static const int DESCRIPTOR_SIZE = 128;

        DescriptorParams();
        DescriptorParams( double _magnification, bool _isNormalize, bool _recalculateAngles );
        double magnification;
        bool isNormalize;
        bool recalculateAngles;
    };

    SIFT();
    //! sift-detector constructor
    SIFT( double _threshold, double _edgeThreshold,
          int _nOctaves=CommonParams::DEFAULT_NOCTAVES,
          int _nOctaveLayers=CommonParams::DEFAULT_NOCTAVE_LAYERS,
          int _firstOctave=CommonParams::DEFAULT_FIRST_OCTAVE,
          int _angleMode=CommonParams::FIRST_ANGLE );
    //! sift-descriptor constructor
    SIFT( double _magnification, bool _isNormalize=true,
          bool _recalculateAngles = true,
          int _nOctaves=CommonParams::DEFAULT_NOCTAVES,
          int _nOctaveLayers=CommonParams::DEFAULT_NOCTAVE_LAYERS,
          int _firstOctave=CommonParams::DEFAULT_FIRST_OCTAVE,
          int _angleMode=CommonParams::FIRST_ANGLE );
    SIFT( const CommonParams& _commParams,
          const DetectorParams& _detectorParams = DetectorParams(),
          const DescriptorParams& _descriptorParams = DescriptorParams() );

    //! returns the descriptor size in floats (128)
    int descriptorSize() const { return DescriptorParams::DESCRIPTOR_SIZE; }
    //! finds the keypoints using SIFT algorithm
    void operator()(const Mat& img, const Mat& mask,
                    vector<KeyPoint>& keypoints) const;
    //! finds the keypoints and computes descriptors for them using SIFT algorithm.
    //! Optionally it can compute descriptors for the user-provided keypoints
    void operator()(const Mat& img, const Mat& mask,
                    vector<KeyPoint>& keypoints,
                    Mat& descriptors,
                    bool useProvidedKeypoints=false) const;

    CommonParams getCommonParams () const { return commParams; }
    DetectorParams getDetectorParams () const { return detectorParams; }
    DescriptorParams getDescriptorParams () const { return descriptorParams; }
protected:
    CommonParams commParams;
    DetectorParams detectorParams;
    DescriptorParams descriptorParams;
};

    
/*!
 SURF implementation.
 
 The class implements SURF algorithm by H. Bay et al.
 */
class CV_EXPORTS SURF : public CvSURFParams
{
public:
    //! the default constructor
    SURF();
    //! the full constructor taking all the necessary parameters
    SURF(double _hessianThreshold, int _nOctaves=4,
         int _nOctaveLayers=2, bool _extended=false);

    //! returns the descriptor size in float's (64 or 128)
    int descriptorSize() const;
    //! finds the keypoints using fast hessian detector used in SURF
    void operator()(const Mat& img, const Mat& mask,
                    vector<KeyPoint>& keypoints) const;
    //! finds the keypoints and computes their descriptors. Optionally it can compute descriptors for the user-provided keypoints
    void operator()(const Mat& img, const Mat& mask,
                    vector<KeyPoint>& keypoints,
                    vector<float>& descriptors,
                    bool useProvidedKeypoints=false) const;
};

/*!
 Maximal Stable Extremal Regions class.
 
 The class implements MSER algorithm introduced by J. Matas.
 Unlike SIFT, SURF and many other detectors in OpenCV, this is salient region detector,
 not the salient point detector.
 
 It returns the regions, each of those is encoded as a contour.
*/
class CV_EXPORTS MSER : public CvMSERParams
{
public:
    //! the default constructor
    MSER();
    //! the full constructor
    MSER( int _delta, int _min_area, int _max_area,
          float _max_variation, float _min_diversity,
          int _max_evolution, double _area_threshold,
          double _min_margin, int _edge_blur_size );
    //! the operator that extracts the MSERs from the image or the specific part of it
    void operator()( const Mat& image, vector<vector<Point> >& msers, const Mat& mask ) const;
};

/*!
 The "Star" Detector.
 
 The class implements the keypoint detector introduced by K. Konolige.
*/
class CV_EXPORTS StarDetector : public CvStarDetectorParams
{
public:
    //! the default constructor
    StarDetector();
    //! the full constructor
    StarDetector(int _maxSize, int _responseThreshold,
                 int _lineThresholdProjected,
                 int _lineThresholdBinarized,
                 int _suppressNonmaxSize);
    //! finds the keypoints in the image
    void operator()(const Mat& image, vector<KeyPoint>& keypoints) const;
};

//! detects corners using FAST algorithm by E. Rosten
CV_EXPORTS void FAST( const Mat& image, vector<KeyPoint>& keypoints, int threshold, bool nonmaxSupression=true );

/*!
 The Patch Generator class
 
 
 
*/
class CV_EXPORTS PatchGenerator
{
public:
    PatchGenerator();
    PatchGenerator(double _backgroundMin, double _backgroundMax,
                   double _noiseRange, bool _randomBlur=true,
                   double _lambdaMin=0.6, double _lambdaMax=1.5,
                   double _thetaMin=-CV_PI, double _thetaMax=CV_PI,
                   double _phiMin=-CV_PI, double _phiMax=CV_PI );
    void operator()(const Mat& image, Point2f pt, Mat& patch, Size patchSize, RNG& rng) const;
    void operator()(const Mat& image, const Mat& transform, Mat& patch,
                    Size patchSize, RNG& rng) const;
    void warpWholeImage(const Mat& image, Mat& matT, Mat& buf,
                        Mat& warped, int border, RNG& rng) const;
    void generateRandomTransform(Point2f srcCenter, Point2f dstCenter,
                                 Mat& transform, RNG& rng, bool inverse=false) const;
    void setAffineParam(double lambda, double theta, double phi);
    
    double backgroundMin, backgroundMax;
    double noiseRange;
    bool randomBlur;
    double lambdaMin, lambdaMax;
    double thetaMin, thetaMax;
    double phiMin, phiMax;
};


class CV_EXPORTS LDetector
{
public:
    LDetector();
    LDetector(int _radius, int _threshold, int _nOctaves,
              int _nViews, double _baseFeatureSize, double _clusteringDistance);
    void operator()(const Mat& image, vector<KeyPoint>& keypoints, int maxCount=0, bool scaleCoords=true) const;
    void operator()(const vector<Mat>& pyr, vector<KeyPoint>& keypoints, int maxCount=0, bool scaleCoords=true) const;
    void getMostStable2D(const Mat& image, vector<KeyPoint>& keypoints,
                         int maxCount, const PatchGenerator& patchGenerator) const;
    void setVerbose(bool verbose);
    
    void read(const FileNode& node);
    void write(FileStorage& fs, const String& name=String()) const;
    
    int radius;
    int threshold;
    int nOctaves;
    int nViews;
    bool verbose;
    
    double baseFeatureSize;
    double clusteringDistance;
};

typedef LDetector YAPE;

class CV_EXPORTS FernClassifier
{
public:
    FernClassifier();
    FernClassifier(const FileNode& node);
    FernClassifier(const vector<Point2f>& points,
                   const vector<Ptr<Mat> >& refimgs,
                   const vector<int>& labels=vector<int>(),
                   int _nclasses=0, int _patchSize=PATCH_SIZE,
                   int _signatureSize=DEFAULT_SIGNATURE_SIZE,
                   int _nstructs=DEFAULT_STRUCTS,
                   int _structSize=DEFAULT_STRUCT_SIZE,
                   int _nviews=DEFAULT_VIEWS,
                   int _compressionMethod=COMPRESSION_NONE,
                   const PatchGenerator& patchGenerator=PatchGenerator());
    virtual ~FernClassifier();
    virtual void read(const FileNode& n);
    virtual void write(FileStorage& fs, const String& name=String()) const;
    virtual void trainFromSingleView(const Mat& image,
                                     const vector<KeyPoint>& keypoints,
                                     int _patchSize=PATCH_SIZE,
                                     int _signatureSize=DEFAULT_SIGNATURE_SIZE,
                                     int _nstructs=DEFAULT_STRUCTS,
                                     int _structSize=DEFAULT_STRUCT_SIZE,
                                     int _nviews=DEFAULT_VIEWS,
                                     int _compressionMethod=COMPRESSION_NONE,
                                     const PatchGenerator& patchGenerator=PatchGenerator());
    virtual void train(const vector<Point2f>& points,
                       const vector<Ptr<Mat> >& refimgs,
                       const vector<int>& labels=vector<int>(),
                       int _nclasses=0, int _patchSize=PATCH_SIZE,
                       int _signatureSize=DEFAULT_SIGNATURE_SIZE,
                       int _nstructs=DEFAULT_STRUCTS,
                       int _structSize=DEFAULT_STRUCT_SIZE,
                       int _nviews=DEFAULT_VIEWS,
                       int _compressionMethod=COMPRESSION_NONE,
                       const PatchGenerator& patchGenerator=PatchGenerator());
    virtual int operator()(const Mat& img, Point2f kpt, vector<float>& signature) const;
    virtual int operator()(const Mat& patch, vector<float>& signature) const;
    virtual void clear();
    void setVerbose(bool verbose);
    
    int getClassCount() const;
    int getStructCount() const;
    int getStructSize() const;
    int getSignatureSize() const;
    int getCompressionMethod() const;
    Size getPatchSize() const;
    
    struct Feature
    {
        uchar x1, y1, x2, y2;
        Feature() : x1(0), y1(0), x2(0), y2(0) {}
        Feature(int _x1, int _y1, int _x2, int _y2)
        : x1((uchar)_x1), y1((uchar)_y1), x2((uchar)_x2), y2((uchar)_y2)
        {}
        template<typename _Tp> bool operator ()(const Mat_<_Tp>& patch) const
        { return patch(y1,x1) > patch(y2, x2); }
    };
    
    enum
    {
        PATCH_SIZE = 31,
        DEFAULT_STRUCTS = 50,
        DEFAULT_STRUCT_SIZE = 9,
        DEFAULT_VIEWS = 5000,
        DEFAULT_SIGNATURE_SIZE = 176,
        COMPRESSION_NONE = 0,
        COMPRESSION_RANDOM_PROJ = 1,
        COMPRESSION_PCA = 2,
        DEFAULT_COMPRESSION_METHOD = COMPRESSION_NONE
    };
    
protected:
    virtual void prepare(int _nclasses, int _patchSize, int _signatureSize,
                         int _nstructs, int _structSize,
                         int _nviews, int _compressionMethod);
    virtual void finalize(RNG& rng);
    virtual int getLeaf(int fidx, const Mat& patch) const;
    
    bool verbose;
    int nstructs;
    int structSize;
    int nclasses;
    int signatureSize;
    int compressionMethod;
    int leavesPerStruct;
    Size patchSize;
    vector<Feature> features;
    vector<int> classCounters;
    vector<float> posteriors;
};

class CV_EXPORTS PlanarObjectDetector
{
public:
    PlanarObjectDetector();
    PlanarObjectDetector(const FileNode& node);
    PlanarObjectDetector(const vector<Mat>& pyr, int _npoints=300,
                         int _patchSize=FernClassifier::PATCH_SIZE,
                         int _nstructs=FernClassifier::DEFAULT_STRUCTS,
                         int _structSize=FernClassifier::DEFAULT_STRUCT_SIZE,
                         int _nviews=FernClassifier::DEFAULT_VIEWS,
                         const LDetector& detector=LDetector(),
                         const PatchGenerator& patchGenerator=PatchGenerator());
    virtual ~PlanarObjectDetector();
    virtual void train(const vector<Mat>& pyr, int _npoints=300,
                       int _patchSize=FernClassifier::PATCH_SIZE,
                       int _nstructs=FernClassifier::DEFAULT_STRUCTS,
                       int _structSize=FernClassifier::DEFAULT_STRUCT_SIZE,
                       int _nviews=FernClassifier::DEFAULT_VIEWS,
                       const LDetector& detector=LDetector(),
                       const PatchGenerator& patchGenerator=PatchGenerator());
    virtual void train(const vector<Mat>& pyr, const vector<KeyPoint>& keypoints,
                       int _patchSize=FernClassifier::PATCH_SIZE,
                       int _nstructs=FernClassifier::DEFAULT_STRUCTS,
                       int _structSize=FernClassifier::DEFAULT_STRUCT_SIZE,
                       int _nviews=FernClassifier::DEFAULT_VIEWS,
                       const LDetector& detector=LDetector(),
                       const PatchGenerator& patchGenerator=PatchGenerator());
    Rect getModelROI() const;
    vector<KeyPoint> getModelPoints() const;
    const LDetector& getDetector() const;
    const FernClassifier& getClassifier() const;
    void setVerbose(bool verbose);
    
    void read(const FileNode& node);
    void write(FileStorage& fs, const String& name=String()) const;
    bool operator()(const Mat& image, Mat& H, vector<Point2f>& corners) const;
    bool operator()(const vector<Mat>& pyr, const vector<KeyPoint>& keypoints,
                    Mat& H, vector<Point2f>& corners, vector<int>* pairs=0) const;
    
protected:
    bool verbose;
    Rect modelROI;
    vector<KeyPoint> modelPoints;
    LDetector ldetector;
    FernClassifier fernClassifier;
};



/****************************************************************************************\
*            Calonder Descriptor                                                         *
\****************************************************************************************/

struct CV_EXPORTS DefaultRngAuto
{
    const static uint64 def_state = (uint64)-1;
    const uint64 old_state;

    DefaultRngAuto() : old_state(theRNG().state) { theRNG().state = def_state; }
    ~DefaultRngAuto() { theRNG().state = old_state; }

    DefaultRngAuto& operator=(const DefaultRngAuto&);
};

/*
A pseudo-random number generator usable with std::random_shuffle.
*/
typedef cv::RNG CalonderRng;
typedef unsigned int int_type;

//----------------------------
//randomized_tree.h

//class RTTester;

//namespace features {
static const size_t DEFAULT_REDUCED_NUM_DIM = 176;
static const float LOWER_QUANT_PERC = .03f;
static const float UPPER_QUANT_PERC = .92f;
static const int PATCH_SIZE = 32;
static const int DEFAULT_DEPTH = 9;
static const int DEFAULT_VIEWS = 5000;
struct RTreeNode;

struct BaseKeypoint
{
    int x;
    int y;
    IplImage* image;

    BaseKeypoint()
        : x(0), y(0), image(NULL)
    {}

    BaseKeypoint(int x, int y, IplImage* image)
        : x(x), y(y), image(image)
    {}
};

class CSMatrixGenerator {
public:
    typedef enum { PDT_GAUSS=1, PDT_BERNOULLI, PDT_DBFRIENDLY } PHI_DISTR_TYPE;
    ~CSMatrixGenerator();
    static float* getCSMatrix(int m, int n, PHI_DISTR_TYPE dt);     // do NOT free returned pointer

private:
    static float *cs_phi_;    // matrix for compressive sensing
    static int cs_phi_m_, cs_phi_n_;
};


template< typename T >
struct AlignedMemBlock
{
  AlignedMemBlock() : raw(NULL), data(NULL) { };

  // Alloc's an `a` bytes-aligned block good to hold `sz` elements of class T
  AlignedMemBlock(const int n, const int a)
  {
     alloc(n, a);
  }

  ~AlignedMemBlock()
  {
     free(raw);
  }

  void alloc(const int n, const int a)
  {
     uchar* raw = (uchar*)malloc(n*sizeof(T) + a);
     int delta = (a - uint64(raw)%a)%a;          // # bytes required for padding s.t. we get `a`-aligned
     data = reinterpret_cast<T*>(raw + delta);
  }

  // Methods to access the aligned data. NEVER EVER FREE A RETURNED POINTER!
  inline T* p() { return data; }
  inline T* operator()() { return data; }

private:
  T *raw;     // raw block, probably not aligned
  T *data;    // exposed data, aligned, DO NOT FREE
};

typedef AlignedMemBlock<float> FloatSignature;
typedef AlignedMemBlock<uchar> Signature;

class CV_EXPORTS RandomizedTree
{
public:
    friend class RTreeClassifier;
    //friend class ::RTTester;

    RandomizedTree();
    ~RandomizedTree();

    void train(std::vector<BaseKeypoint> const& base_set, cv::RNG &rng,
        int depth, int views, size_t reduced_num_dim, int num_quant_bits);

    void train(std::vector<BaseKeypoint> const& base_set, cv::RNG &rng,
        PatchGenerator &make_patch, int depth, int views, size_t reduced_num_dim,
        int num_quant_bits);

    // following two funcs are EXPERIMENTAL (do not use unless you know exactly what you do)
    static void quantizeVector(float *vec, int dim, int N, float bnds[2], int clamp_mode=0);
    static void quantizeVector(float *src, int dim, int N, float bnds[2], uchar *dst);

    // patch_data must be a 32x32 array (no row padding)
    float* getPosterior(uchar* patch_data);
    const float* getPosterior(uchar* patch_data) const;
    uchar* getPosterior2(uchar* patch_data);

    void read(const char* file_name, int num_quant_bits);
    void read(std::istream &is, int num_quant_bits);
    void write(const char* file_name) const;
    void write(std::ostream &os) const;

    inline int classes() { return classes_; }
    inline int depth() { return depth_; }

    inline void applyQuantization(int num_quant_bits) { makePosteriors2(num_quant_bits); }

    // debug
    void savePosteriors(std::string url, bool append=false);
    void savePosteriors2(std::string url, bool append=false);

private:
    int classes_;
    int depth_;
    int num_leaves_;
    std::vector<RTreeNode> nodes_;
    //float **posteriors_;       // 16-bytes aligned posteriors
    //uchar **posteriors2_;      // 16-bytes aligned posteriors
  FloatSignature *posteriors_;
  Signature  *posteriors2_;
  std::vector<int> leaf_counts_;

    void createNodes(int num_nodes, cv::RNG &rng);
    void allocPosteriorsAligned(int num_leaves, int num_classes);
    void freePosteriors(int which);    // which: 1=posteriors_, 2=posteriors2_, 3=both
    void init(int classes, int depth, cv::RNG &rng);
    void addExample(int class_id, uchar* patch_data);
    void finalize(size_t reduced_num_dim, int num_quant_bits);
    int getIndex(uchar* patch_data) const;
    inline float* getPosteriorByIndex(int index);
    inline uchar* getPosteriorByIndex2(int index);
    inline const float* getPosteriorByIndex(int index) const;
    //void makeRandomMeasMatrix(float *cs_phi, PHI_DISTR_TYPE dt, size_t reduced_num_dim);
    void convertPosteriorsToChar();
    void makePosteriors2(int num_quant_bits);
    void compressLeaves(size_t reduced_num_dim);
    void estimateQuantPercForPosteriors(float perc[2]);
};

struct RTreeNode
{
    short offset1, offset2;

    RTreeNode() {}

    RTreeNode(uchar x1, uchar y1, uchar x2, uchar y2)
        : offset1(y1*PATCH_SIZE + x1),
        offset2(y2*PATCH_SIZE + x2)
    {}

    //! Left child on 0, right child on 1
    inline bool operator() (uchar* patch_data) const
    {
        return patch_data[offset1] > patch_data[offset2];
    }
};



//} // namespace features
//----------------------------
//rtree_classifier.h
//class RTTester;

//namespace features {

class CV_EXPORTS RTreeClassifier
{
public:
    //friend class ::RTTester;
    static const int DEFAULT_TREES = 80;
    static const size_t DEFAULT_NUM_QUANT_BITS = 4;

  //static const int SIG_LEN = 176;

  RTreeClassifier();

    //modified
    void train(std::vector<BaseKeypoint> const& base_set,
        cv::RNG &rng,
        int num_trees = RTreeClassifier::DEFAULT_TREES,
        int depth = DEFAULT_DEPTH,
        int views = DEFAULT_VIEWS,
        size_t reduced_num_dim = DEFAULT_REDUCED_NUM_DIM,
        int num_quant_bits = DEFAULT_NUM_QUANT_BITS,
        bool print_status = true);

  void train(std::vector<BaseKeypoint> const& base_set,
        cv::RNG &rng,
        PatchGenerator &make_patch,
        int num_trees = DEFAULT_TREES,
        int depth = DEFAULT_DEPTH,
        int views = DEFAULT_VIEWS,
        size_t reduced_num_dim = DEFAULT_REDUCED_NUM_DIM,
        int num_quant_bits = DEFAULT_NUM_QUANT_BITS,
        bool print_status = true);

    // sig must point to a memory block of at least classes()*sizeof(float|uchar) bytes
    void getSignature(IplImage *patch, uchar *sig);
    void getSignature(IplImage *patch, float *sig);
    void getSparseSignature(IplImage *patch, float *sig, float thresh);
    // TODO: deprecated in favor of getSignature overload, remove
    void getFloatSignature(IplImage *patch, float *sig) { getSignature(patch, sig); }

    static int countNonZeroElements(float *vec, int n, double tol=1e-10);
    static inline void safeSignatureAlloc(uchar **sig, int num_sig=1, int sig_len=176);
    static inline uchar* safeSignatureAlloc(int num_sig=1, int sig_len=176);

    inline int classes() const { return classes_; }
    inline int original_num_classes() { return original_num_classes_; }

    void setQuantization(int num_quant_bits);
    void discardFloatPosteriors();

    void read(const char* file_name);
    void read(std::istream &is);
    void write(const char* file_name) const;
    void write(std::ostream &os) const;

    // experimental and debug
    void saveAllFloatPosteriors(std::string file_url);
    void saveAllBytePosteriors(std::string file_url);
    void setFloatPosteriorsFromTextfile_176(std::string url);
    float countZeroElements();

    std::vector<RandomizedTree> trees_;

private:
    int classes_;
    int num_quant_bits_;
    uchar **posteriors_;
    ushort *ptemp_;
    int original_num_classes_;
    bool keep_floats_;
};


/****************************************************************************************\
*                                     One-Way Descriptor                                 *
\****************************************************************************************/

class CV_EXPORTS OneWayDescriptor
{
public:
    OneWayDescriptor();
    ~OneWayDescriptor();

    // allocates memory for given descriptor parameters
    void Allocate(int pose_count, CvSize size, int nChannels);

    // GenerateSamples: generates affine transformed patches with averaging them over small transformation variations.
    // If external poses and transforms were specified, uses them instead of generating random ones
    // - pose_count: the number of poses to be generated
    // - frontal: the input patch (can be a roi in a larger image)
    // - norm: if nonzero, normalizes the output patch so that the sum of pixel intensities is 1
    void GenerateSamples(int pose_count, IplImage* frontal, int norm = 0);

    // GenerateSamplesFast: generates affine transformed patches with averaging them over small transformation variations.
    // Uses precalculated transformed pca components.
    // - frontal: the input patch (can be a roi in a larger image)
    // - pca_hr_avg: pca average vector
    // - pca_hr_eigenvectors: pca eigenvectors
    // - pca_descriptors: an array of precomputed descriptors of pca components containing their affine transformations
    //   pca_descriptors[0] corresponds to the average, pca_descriptors[1]-pca_descriptors[pca_dim] correspond to eigenvectors
    void GenerateSamplesFast(IplImage* frontal, CvMat* pca_hr_avg,
                             CvMat* pca_hr_eigenvectors, OneWayDescriptor* pca_descriptors);

    // sets the poses and corresponding transforms
    void SetTransforms(CvAffinePose* poses, CvMat** transforms);

    // Initialize: builds a descriptor.
    // - pose_count: the number of poses to build. If poses were set externally, uses them rather than generating random ones
    // - frontal: input patch. Can be a roi in a larger image
    // - feature_name: the feature name to be associated with the descriptor
    // - norm: if 1, the affine transformed patches are normalized so that their sum is 1
    void Initialize(int pose_count, IplImage* frontal, const char* feature_name = 0, int norm = 0);

    // InitializeFast: builds a descriptor using precomputed descriptors of pca components
    // - pose_count: the number of poses to build
    // - frontal: input patch. Can be a roi in a larger image
    // - feature_name: the feature name to be associated with the descriptor
    // - pca_hr_avg: average vector for PCA
    // - pca_hr_eigenvectors: PCA eigenvectors (one vector per row)
    // - pca_descriptors: precomputed descriptors of PCA components, the first descriptor for the average vector
    // followed by the descriptors for eigenvectors
    void InitializeFast(int pose_count, IplImage* frontal, const char* feature_name,
                        CvMat* pca_hr_avg, CvMat* pca_hr_eigenvectors, OneWayDescriptor* pca_descriptors);

    // ProjectPCASample: unwarps an image patch into a vector and projects it into PCA space
    // - patch: input image patch
    // - avg: PCA average vector
    // - eigenvectors: PCA eigenvectors, one per row
    // - pca_coeffs: output PCA coefficients
    void ProjectPCASample(IplImage* patch, CvMat* avg, CvMat* eigenvectors, CvMat* pca_coeffs) const;

    // InitializePCACoeffs: projects all warped patches into PCA space
    // - avg: PCA average vector
    // - eigenvectors: PCA eigenvectors, one per row
    void InitializePCACoeffs(CvMat* avg, CvMat* eigenvectors);

    // EstimatePose: finds the closest match between an input patch and a set of patches with different poses
    // - patch: input image patch
    // - pose_idx: the output index of the closest pose
    // - distance: the distance to the closest pose (L2 distance)
    void EstimatePose(IplImage* patch, int& pose_idx, float& distance) const;

    // EstimatePosePCA: finds the closest match between an input patch and a set of patches with different poses.
    // The distance between patches is computed in PCA space
    // - patch: input image patch
    // - pose_idx: the output index of the closest pose
    // - distance: distance to the closest pose (L2 distance in PCA space)
    // - avg: PCA average vector. If 0, matching without PCA is used
    // - eigenvectors: PCA eigenvectors, one per row
    void EstimatePosePCA(CvArr* patch, int& pose_idx, float& distance, CvMat* avg, CvMat* eigenvalues) const;

    // GetPatchSize: returns the size of each image patch after warping (2 times smaller than the input patch)
    CvSize GetPatchSize() const
    {
        return m_patch_size;
    }

    // GetInputPatchSize: returns the required size of the patch that the descriptor is built from
    // (2 time larger than the patch after warping)
    CvSize GetInputPatchSize() const
    {
        return cvSize(m_patch_size.width*2, m_patch_size.height*2);
    }

    // GetPatch: returns a patch corresponding to specified pose index
    // - index: pose index
    // - return value: the patch corresponding to specified pose index
    IplImage* GetPatch(int index);

    // GetPose: returns a pose corresponding to specified pose index
    // - index: pose index
    // - return value: the pose corresponding to specified pose index
    CvAffinePose GetPose(int index) const;

    // Save: saves all patches with different poses to a specified path
    void Save(const char* path);

    // ReadByName: reads a descriptor from a file storage
    // - fs: file storage
    // - parent: parent node
    // - name: node name
    // - return value: 1 if succeeded, 0 otherwise
    int ReadByName(CvFileStorage* fs, CvFileNode* parent, const char* name);

    // ReadByName: reads a descriptor from a file node
    // - parent: parent node
    // - name: node name
    // - return value: 1 if succeeded, 0 otherwise
    int ReadByName(const FileNode &parent, const char* name);

    // Write: writes a descriptor into a file storage
    // - fs: file storage
    // - name: node name
    void Write(CvFileStorage* fs, const char* name);

    // GetFeatureName: returns a name corresponding to a feature
    const char* GetFeatureName() const;

    // GetCenter: returns the center of the feature
    CvPoint GetCenter() const;

    void SetPCADimHigh(int pca_dim_high) {m_pca_dim_high = pca_dim_high;};
    void SetPCADimLow(int pca_dim_low) {m_pca_dim_low = pca_dim_low;};

    int GetPCADimLow() const;
    int GetPCADimHigh() const;

    CvMat** GetPCACoeffs() const {return m_pca_coeffs;}

protected:
    int m_pose_count; // the number of poses
    CvSize m_patch_size; // size of each image
    IplImage** m_samples; // an array of length m_pose_count containing the patch in different poses
    IplImage* m_input_patch;
    IplImage* m_train_patch;
    CvMat** m_pca_coeffs; // an array of length m_pose_count containing pca decomposition of the patch in different poses
    CvAffinePose* m_affine_poses; // an array of poses
    CvMat** m_transforms; // an array of affine transforms corresponding to poses

    std::string m_feature_name; // the name of the feature associated with the descriptor
    CvPoint m_center; // the coordinates of the feature (the center of the input image ROI)

    int m_pca_dim_high; // the number of descriptor pca components to use for generating affine poses
    int m_pca_dim_low; // the number of pca components to use for comparison
};


// OneWayDescriptorBase: encapsulates functionality for training/loading a set of one way descriptors
// and finding the nearest closest descriptor to an input feature
class CV_EXPORTS OneWayDescriptorBase
{
public:

    // creates an instance of OneWayDescriptor from a set of training files
    // - patch_size: size of the input (large) patch
    // - pose_count: the number of poses to generate for each descriptor
    // - train_path: path to training files
    // - pca_config: the name of the file that contains PCA for small patches (2 times smaller
    // than patch_size each dimension
    // - pca_hr_config: the name of the file that contains PCA for large patches (of patch_size size)
    // - pca_desc_config: the name of the file that contains descriptors of PCA components
    OneWayDescriptorBase(CvSize patch_size, int pose_count, const char* train_path = 0, const char* pca_config = 0,
                         const char* pca_hr_config = 0, const char* pca_desc_config = 0, int pyr_levels = 1,
                         int pca_dim_high = 100, int pca_dim_low = 100);

    OneWayDescriptorBase(CvSize patch_size, int pose_count, const string &pca_filename, const string &train_path = string(), const string &images_list = string(),
                         float _scale_min = 0.7f, float _scale_max=1.5f, float _scale_step=1.2f, int pyr_levels = 1,
                         int pca_dim_high = 100, int pca_dim_low = 100);


    virtual ~OneWayDescriptorBase();
    void clear ();


    // Allocate: allocates memory for a given number of descriptors
    void Allocate(int train_feature_count);

    // AllocatePCADescriptors: allocates memory for pca descriptors
    void AllocatePCADescriptors();

    // returns patch size
    CvSize GetPatchSize() const {return m_patch_size;};
    // returns the number of poses for each descriptor
    int GetPoseCount() const {return m_pose_count;};

    // returns the number of pyramid levels
    int GetPyrLevels() const {return m_pyr_levels;};

    // returns the number of descriptors
    int GetDescriptorCount() const {return m_train_feature_count;};

    // CreateDescriptorsFromImage: creates descriptors for each of the input features
    // - src: input image
    // - features: input features
    // - pyr_levels: the number of pyramid levels
    void CreateDescriptorsFromImage(IplImage* src, const std::vector<cv::KeyPoint>& features);

    // CreatePCADescriptors: generates descriptors for PCA components, needed for fast generation of feature descriptors
    void CreatePCADescriptors();

    // returns a feature descriptor by feature index
    const OneWayDescriptor* GetDescriptor(int desc_idx) const {return &m_descriptors[desc_idx];};

    // FindDescriptor: finds the closest descriptor
    // - patch: input image patch
    // - desc_idx: output index of the closest descriptor to the input patch
    // - pose_idx: output index of the closest pose of the closest descriptor to the input patch
    // - distance: distance from the input patch to the closest feature pose
    // - _scales: scales of the input patch for each descriptor
    // - scale_ranges: input scales variation (float[2])
    void FindDescriptor(IplImage* patch, int& desc_idx, int& pose_idx, float& distance, float* _scale = 0, float* scale_ranges = 0) const;

    // - patch: input image patch
    // - n: number of the closest indexes
    // - desc_idxs: output indexes of the closest descriptor to the input patch (n)
    // - pose_idx: output indexes of the closest pose of the closest descriptor to the input patch (n)
    // - distances: distance from the input patch to the closest feature pose (n)
    // - _scales: scales of the input patch
    // - scale_ranges: input scales variation (float[2])
    void FindDescriptor(IplImage* patch, int n, std::vector<int>& desc_idxs, std::vector<int>& pose_idxs,
                        std::vector<float>& distances, std::vector<float>& _scales, float* scale_ranges = 0) const;

    // FindDescriptor: finds the closest descriptor
    // - src: input image
    // - pt: center of the feature
    // - desc_idx: output index of the closest descriptor to the input patch
    // - pose_idx: output index of the closest pose of the closest descriptor to the input patch
    // - distance: distance from the input patch to the closest feature pose
    void FindDescriptor(IplImage* src, cv::Point2f pt, int& desc_idx, int& pose_idx, float& distance) const;

    // InitializePoses: generates random poses
    void InitializePoses();

    // InitializeTransformsFromPoses: generates 2x3 affine matrices from poses (initializes m_transforms)
    void InitializeTransformsFromPoses();

    // InitializePoseTransforms: subsequently calls InitializePoses and InitializeTransformsFromPoses
    void InitializePoseTransforms();

    // InitializeDescriptor: initializes a descriptor
    // - desc_idx: descriptor index
    // - train_image: image patch (ROI is supported)
    // - feature_label: feature textual label
    void InitializeDescriptor(int desc_idx, IplImage* train_image, const char* feature_label);

    void InitializeDescriptor(int desc_idx, IplImage* train_image, const cv::KeyPoint& keypoint, const char* feature_label);

    // InitializeDescriptors: load features from an image and create descriptors for each of them
    void InitializeDescriptors(IplImage* train_image, const vector<cv::KeyPoint>& features,
                               const char* feature_label = "", int desc_start_idx = 0);

    // Write: writes this object to a file storage
    // - fs: output filestorage
    void Write (FileStorage &fs) const;
    
    // Read: reads OneWayDescriptorBase object from a file node
    // - fn: input file node    
    void Read (const FileNode &fn);

    // LoadPCADescriptors: loads PCA descriptors from a file
    // - filename: input filename
    int LoadPCADescriptors(const char* filename);

    // LoadPCADescriptors: loads PCA descriptors from a file node
    // - fn: input file node
    int LoadPCADescriptors(const FileNode &fn);

    // SavePCADescriptors: saves PCA descriptors to a file
    // - filename: output filename
    void SavePCADescriptors(const char* filename);

    // SavePCADescriptors: saves PCA descriptors to a file storage
    // - fs: output file storage
    void SavePCADescriptors(CvFileStorage* fs) const;

    // GeneratePCA: calculate and save PCA components and descriptors
    // - img_path: path to training PCA images directory
    // - images_list: filename with filenames of training PCA images
    void GeneratePCA(const char* img_path, const char* images_list, int pose_count=500);

    // SetPCAHigh: sets the high resolution pca matrices (copied to internal structures)
    void SetPCAHigh(CvMat* avg, CvMat* eigenvectors);

    // SetPCALow: sets the low resolution pca matrices (copied to internal structures)
    void SetPCALow(CvMat* avg, CvMat* eigenvectors);

    int GetLowPCA(CvMat** avg, CvMat** eigenvectors)
    {
        *avg = m_pca_avg;
        *eigenvectors = m_pca_eigenvectors;
        return m_pca_dim_low;
    };

    int GetPCADimLow() const {return m_pca_dim_low;};
    int GetPCADimHigh() const {return m_pca_dim_high;};

    void ConvertDescriptorsArrayToTree(); // Converting pca_descriptors array to KD tree

    // GetPCAFilename: get default PCA filename
    static string GetPCAFilename () { return "pca.yml"; }

protected:
    CvSize m_patch_size; // patch size
    int m_pose_count; // the number of poses for each descriptor
    int m_train_feature_count; // the number of the training features
    OneWayDescriptor* m_descriptors; // array of train feature descriptors
    CvMat* m_pca_avg; // PCA average Vector for small patches
    CvMat* m_pca_eigenvectors; // PCA eigenvectors for small patches
    CvMat* m_pca_hr_avg; // PCA average Vector for large patches
    CvMat* m_pca_hr_eigenvectors; // PCA eigenvectors for large patches
    OneWayDescriptor* m_pca_descriptors; // an array of PCA descriptors

    cv::flann::Index* m_pca_descriptors_tree;
    CvMat* m_pca_descriptors_matrix;

    CvAffinePose* m_poses; // array of poses
    CvMat** m_transforms; // array of affine transformations corresponding to poses

    int m_pca_dim_high;
    int m_pca_dim_low;

    int m_pyr_levels;
    float scale_min;
    float scale_max;
    float scale_step;

    // SavePCAall: saves PCA components and descriptors to a file storage
    // - fs: output file storage
    void SavePCAall (FileStorage &fs) const;

    // LoadPCAall: loads PCA components and descriptors from a file node
    // - fn: input file node
    void LoadPCAall (const FileNode &fn);
};

class CV_EXPORTS OneWayDescriptorObject : public OneWayDescriptorBase
{
public:
    // creates an instance of OneWayDescriptorObject from a set of training files
    // - patch_size: size of the input (large) patch
    // - pose_count: the number of poses to generate for each descriptor
    // - train_path: path to training files
    // - pca_config: the name of the file that contains PCA for small patches (2 times smaller
    // than patch_size each dimension
    // - pca_hr_config: the name of the file that contains PCA for large patches (of patch_size size)
    // - pca_desc_config: the name of the file that contains descriptors of PCA components
    OneWayDescriptorObject(CvSize patch_size, int pose_count, const char* train_path, const char* pca_config,
                           const char* pca_hr_config = 0, const char* pca_desc_config = 0, int pyr_levels = 1);

    OneWayDescriptorObject(CvSize patch_size, int pose_count, const string &pca_filename,
                           const string &train_path = string (), const string &images_list = string (),
                           float _scale_min = 0.7f, float _scale_max=1.5f, float _scale_step=1.2f, int pyr_levels = 1);


    virtual ~OneWayDescriptorObject();

    // Allocate: allocates memory for a given number of features
    // - train_feature_count: the total number of features
    // - object_feature_count: the number of features extracted from the object
    void Allocate(int train_feature_count, int object_feature_count);


    void SetLabeledFeatures(const vector<cv::KeyPoint>& features) {m_train_features = features;};
    vector<cv::KeyPoint>& GetLabeledFeatures() {return m_train_features;};
    const vector<cv::KeyPoint>& GetLabeledFeatures() const {return m_train_features;};
    vector<cv::KeyPoint> _GetLabeledFeatures() const;

    // IsDescriptorObject: returns 1 if descriptor with specified index is positive, otherwise 0
    int IsDescriptorObject(int desc_idx) const;

    // MatchPointToPart: returns the part number of a feature if it matches one of the object parts, otherwise -1
    int MatchPointToPart(CvPoint pt) const;

    // GetDescriptorPart: returns the part number of the feature corresponding to a specified descriptor
    // - desc_idx: descriptor index
    int GetDescriptorPart(int desc_idx) const;


    void InitializeObjectDescriptors(IplImage* train_image, const vector<cv::KeyPoint>& features,
                                     const char* feature_label, int desc_start_idx = 0, float scale = 1.0f,
                                     int is_background = 0);

    // GetObjectFeatureCount: returns the number of object features
    int GetObjectFeatureCount() const {return m_object_feature_count;};

protected:
    int* m_part_id; // contains part id for each of object descriptors
    vector<cv::KeyPoint> m_train_features; // train features
    int m_object_feature_count; // the number of the positive features

};


/****************************************************************************************\
*                                    FeatureDetector                                     *
\****************************************************************************************/

/*
 * Abstract base class for 2D image feature detectors.
 */
class CV_EXPORTS FeatureDetector
{
public:
    /*
     * Detect keypoints in an image.
     *
     * image        The image.
     * keypoints    The detected keypoints.
     * mask         Mask specifying where to look for keypoints (optional). Must be a char matrix with non-zero values in the region of interest.
     */
    void detect( const Mat& image, vector<KeyPoint>& keypoints, const Mat& mask=Mat() ) const
    {
        detectImpl( image, mask, keypoints );
    }

    virtual void read(const FileNode& fn) {};
    virtual void write(FileStorage& fs) const {};

protected:
    /*
     * Detect keypoints; detect() calls this. Must be implemented by the subclass.
     */
    virtual void detectImpl( const Mat& image, const Mat& mask, vector<KeyPoint>& keypoints ) const = 0;

    /*
     * Remove keypoints that are not in the mask.
     *
     * Helper function, useful when wrapping a library call for keypoint detection that
     * does not support a mask argument.
     */
    static void removeInvalidPoints( const Mat& mask, vector<KeyPoint>& keypoints );
};

class CV_EXPORTS FastFeatureDetector : public FeatureDetector
{
public:
    FastFeatureDetector( int _threshold = 1, bool _nonmaxSuppression = true );

    virtual void read (const FileNode& fn);
    virtual void write (FileStorage& fs) const;

protected:
    virtual void detectImpl( const Mat& image, const Mat& mask, vector<KeyPoint>& keypoints ) const;

    int threshold;
    bool nonmaxSuppression;
};


class CV_EXPORTS GoodFeaturesToTrackDetector : public FeatureDetector
{
public:
    GoodFeaturesToTrackDetector( int _maxCorners, double _qualityLevel, double _minDistance,
                                 int _blockSize=3, bool _useHarrisDetector=false, double _k=0.04 );

    virtual void read (const FileNode& fn);
    virtual void write (FileStorage& fs) const;

protected:
    virtual void detectImpl( const Mat& image, const Mat& mask, vector<KeyPoint>& keypoints ) const;

    int maxCorners;
    double qualityLevel;
    double minDistance;
    int blockSize;
    bool useHarrisDetector;
    double k;
};

class CV_EXPORTS MserFeatureDetector : public FeatureDetector
{
public:
    MserFeatureDetector( CvMSERParams params = cvMSERParams () );
    MserFeatureDetector( int delta, int minArea, int maxArea, float maxVariation, float minDiversity,
                         int maxEvolution, double areaThreshold, double minMargin, int edgeBlurSize );

    virtual void read (const FileNode& fn);
    virtual void write (FileStorage& fs) const;

protected:
    virtual void detectImpl( const Mat& image, const Mat& mask, vector<KeyPoint>& keypoints ) const;

    MSER mser;
};

class CV_EXPORTS StarFeatureDetector : public FeatureDetector
{
public:
    StarFeatureDetector( int maxSize=16, int responseThreshold=30, int lineThresholdProjected = 10,
                         int lineThresholdBinarized=8, int suppressNonmaxSize=5 );

    virtual void read (const FileNode& fn);
    virtual void write (FileStorage& fs) const;

protected:
    virtual void detectImpl( const Mat& image, const Mat& mask, vector<KeyPoint>& keypoints ) const;

    StarDetector star;
};

class CV_EXPORTS SiftFeatureDetector : public FeatureDetector
{
public:
    SiftFeatureDetector( double threshold=SIFT::DetectorParams::GET_DEFAULT_THRESHOLD(),
                         double edgeThreshold=SIFT::DetectorParams::GET_DEFAULT_EDGE_THRESHOLD(),
                         int nOctaves=SIFT::CommonParams::DEFAULT_NOCTAVES,
                         int nOctaveLayers=SIFT::CommonParams::DEFAULT_NOCTAVE_LAYERS,
                         int firstOctave=SIFT::CommonParams::DEFAULT_FIRST_OCTAVE,
                         int angleMode=SIFT::CommonParams::FIRST_ANGLE );

    virtual void read (const FileNode& fn);
    virtual void write (FileStorage& fs) const;

protected:
    virtual void detectImpl( const Mat& image, const Mat& mask, vector<KeyPoint>& keypoints ) const;

    SIFT sift;
};

class CV_EXPORTS SurfFeatureDetector : public FeatureDetector
{
public:
    SurfFeatureDetector( double hessianThreshold = 400., int octaves = 3, int octaveLayers = 4 );

    virtual void read (const FileNode& fn);
    virtual void write (FileStorage& fs) const;

protected:
    virtual void detectImpl( const Mat& image, const Mat& mask, vector<KeyPoint>& keypoints ) const;

    SURF surf;
};


/****************************************************************************************\
*                                 DescriptorExtractor                                    *
\****************************************************************************************/

/*
 * Abstract base class for computing descriptors for image keypoints.
 *
 * In this interface we assume a keypoint descriptor can be represented as a
 * dense, fixed-dimensional vector of some basic type. Most descriptors used
 * in practice follow this pattern, as it makes it very easy to compute
 * distances between descriptors. Therefore we represent a collection of
 * descriptors as a cv::Mat, where each row is one keypoint descriptor.
 */
class CV_EXPORTS DescriptorExtractor
{
public:
    /*
     * Compute the descriptors for a set of keypoints in an image.
     *
     * Must be implemented by the subclass.
     *
     * image        The image.
     * keypoints    The keypoints. Keypoints for which a descriptor cannot be computed are removed.
     * descriptors  The descriptors. Row i is the descriptor for keypoint i.
     */
    virtual void compute( const Mat& image, vector<KeyPoint>& keypoints, Mat& descriptors ) const = 0;

    virtual void read (const FileNode &fn) {};
    virtual void write (FileStorage &fs) const {};

protected:
    /*
     * Remove keypoints within border_pixels of an image edge.
     */
    static void removeBorderKeypoints( vector<KeyPoint>& keypoints,
                                       Size imageSize, int borderPixels );
};

class CV_EXPORTS SiftDescriptorExtractor : public DescriptorExtractor
{
public:
    SiftDescriptorExtractor( double magnification=SIFT::DescriptorParams::GET_DEFAULT_MAGNIFICATION(),
                             bool isNormalize=true, bool recalculateAngles=true,
                             int nOctaves=SIFT::CommonParams::DEFAULT_NOCTAVES,
                             int nOctaveLayers=SIFT::CommonParams::DEFAULT_NOCTAVE_LAYERS,
                             int firstOctave=SIFT::CommonParams::DEFAULT_FIRST_OCTAVE,
                             int angleMode=SIFT::CommonParams::FIRST_ANGLE );

    virtual void compute( const Mat& image, vector<KeyPoint>& keypoints, Mat& descriptors) const;
    virtual void read (const FileNode &fn);
    virtual void write (FileStorage &fs) const;

protected:
    SIFT sift;
};

class CV_EXPORTS SurfDescriptorExtractor : public DescriptorExtractor
{
public:
    SurfDescriptorExtractor( int nOctaves=4,
                             int nOctaveLayers=2, bool extended=false );

    virtual void compute( const Mat& image, vector<KeyPoint>& keypoints, Mat& descriptors) const;
    virtual void read (const FileNode &fn);
    virtual void write (FileStorage &fs) const;

protected:
    SURF surf;
};

/****************************************************************************************\
*                                          Distance                                      *
\****************************************************************************************/
template<typename T>
struct CV_EXPORTS Accumulator
{
    typedef T Type;
};

template<> struct Accumulator<unsigned char>  { typedef unsigned int Type; };
template<> struct Accumulator<unsigned short> { typedef unsigned int Type; };
template<> struct Accumulator<char>   { typedef int Type; };
template<> struct Accumulator<short>  { typedef int Type; };

/*
 * Squared Euclidean distance functor
 */
template<class T>
struct CV_EXPORTS L2
{
    typedef T ValueType;
    typedef typename Accumulator<T>::Type ResultType;

    ResultType operator()( const T* a, const T* b, int size ) const
    {
        ResultType result = ResultType();
        for( int i = 0; i < size; i++ )
        {
            ResultType diff = a[i] - b[i];
            result += diff*diff;
        }
        return sqrt(result);
    }
};

/****************************************************************************************\
*                                  DescriptorMatcher                                     *
\****************************************************************************************/
/*
 * Abstract base class for matching two sets of descriptors.
 */
class CV_EXPORTS DescriptorMatcher
{
public:
    /*
     * Add descriptors to the training set
     * descriptors Descriptors to add to the training set
     */
    void add( const Mat& descriptors );

    /*
     * Index the descriptors training set
     */
    void index();

    /*
     * Find the best match for each descriptor from a query set
     *
     * query         The query set of descriptors
     * matches       Indices of the closest matches from the training set
     */
    void match( const Mat& query, vector<int>& matches ) const;

    /*
     * Find the best matches between two descriptor sets, with constraints
     * on which pairs of descriptors can be matched.
     *
     * The mask describes which descriptors can be matched. descriptors_1[i]
     * can be matched with descriptors_2[j] only if mask.at<char>(i,j) is non-zero.
     *
     * query         The query set of descriptors
     * mask          Mask specifying permissible matches.
     * matches       Indices of the closest matches from the training set
     */
    void match( const Mat& query, const Mat& mask,
                vector<int>& matches ) const;

    /*
     * Find the best keypoint matches for small view changes.
     *
     * This function will only match descriptors whose keypoints have close enough
     * image coordinates.
     *
     * keypoints_1   The first set of keypoints.
     * descriptors_1 The first set of descriptors.
     * keypoints_2   The second set of keypoints.
     * descriptors_2 The second set of descriptors.
     * maxDeltaX     The maximum horizontal displacement.
     * maxDeltaY     The maximum vertical displacement.
     * matches       The matches between both sets.
     */
    /*void matchWindowed( const vector<KeyPoint>& keypoints_1, const Mat& descriptors_1,
                        const vector<KeyPoint>& keypoints_2, const Mat& descriptors_2,
                        float maxDeltaX, float maxDeltaY, vector<Match>& matches) const;*/
    virtual void clear();

protected:
    Mat train;

    /*
     * Find matches; match() calls this. Must be implemented by the subclass.
     * The mask may be empty.
     */
    virtual void matchImpl( const Mat& descriptors_1, const Mat& descriptors_2,
                            const Mat& mask, vector<int>& matches ) const = 0;

    static bool possibleMatch( const Mat& mask, int index_1, int index_2 )
    {
        return mask.empty() || mask.at<char>(index_1, index_2);
    }
};

inline void DescriptorMatcher::add( const Mat& descriptors )
{
    if( train.empty() )
    {
        train = descriptors;
    }
    else
    {
        // merge train and descriptors
        Mat m( train.rows + descriptors.rows, train.cols, CV_32F );
        Mat m1 = m.rowRange( 0, train.rows );
        train.copyTo( m1 );
        Mat m2 = m.rowRange( train.rows + 1, m.rows );
        descriptors.copyTo( m2 );
        train = m;
    }
}

inline void DescriptorMatcher::match( const Mat& query, vector<int>& matches ) const
{
    matchImpl( query, train, Mat(), matches );
}

inline void DescriptorMatcher::match( const Mat& query, const Mat& mask,
                                      vector<int>& matches ) const
{
    matchImpl( query, train, mask, matches );
}

inline void DescriptorMatcher::clear()
{
    train.release();
}

/*
 * Brute-force descriptor matcher.
 *
 * For each descriptor in the first set, this matcher finds the closest
 * descriptor in the second set by trying each one.
 *
 * For efficiency, BruteForceMatcher is templated on the distance metric.
 * For float descriptors, a common choice would be features_2d::L2<float>.
 */
template<class Distance>
class CV_EXPORTS BruteForceMatcher : public DescriptorMatcher
{
public:
    BruteForceMatcher( Distance d = Distance() ) : distance(d) {}

protected:
   virtual void matchImpl( const Mat& descriptors_1, const Mat& descriptors_2,
                           const Mat& mask, vector<int>& matches ) const;

   Distance distance;
};

template<class Distance>
void BruteForceMatcher<Distance>::matchImpl( const Mat& descriptors_1, const Mat& descriptors_2,
                                             const Mat& mask, vector<int>& matches ) const
{
    typedef typename Distance::ValueType ValueType;
    typedef typename Distance::ResultType DistanceType;

    assert( mask.empty() || (mask.rows == descriptors_1.rows && mask.cols == descriptors_2.rows) );

    assert( descriptors_1.cols == descriptors_2.cols ||  descriptors_1.empty() ||  descriptors_2.empty() );
    assert( DataType<ValueType>::type == descriptors_1.type() ||  descriptors_1.empty() );
    assert( DataType<ValueType>::type == descriptors_2.type() ||  descriptors_2.empty() );

    int dimension = descriptors_1.cols;
    matches.clear();
    matches.reserve(descriptors_1.rows);

    for( int i = 0; i < descriptors_1.rows; i++ )
    {
        const ValueType* d1 = descriptors_1.ptr<ValueType>(i);
        int matchIndex = -1;
        DistanceType matchDistance = std::numeric_limits<DistanceType>::max();

        for( int j = 0; j < descriptors_2.rows; j++ )
        {
            if( possibleMatch(mask, i, j) )
            {
                const ValueType* d2 = descriptors_2.ptr<ValueType>(j);
                DistanceType curDistance = distance(d1, d2, dimension);
                if( curDistance < matchDistance )
                {
                    matchDistance = curDistance;
                    matchIndex = j;
                }
            }
        }

        if( matchIndex != -1 )
            matches.push_back( matchIndex );
    }
}


/****************************************************************************************\
*                                GenericDescriptorMatch                                  *
\****************************************************************************************/
/*
 * A storage for sets of keypoints together with corresponding images and class IDs
 */
class CV_EXPORTS KeyPointCollection
{
public:
    // Adds keypoints from a single image to the storage
    // image    Source image
    // points   A vector of keypoints
    void add( const Mat& _image, const vector<KeyPoint>& _points );

    // Returns the total number of keypoints in the collection
    size_t calcKeypointCount() const;

    // Returns the keypoint by its global index
    KeyPoint getKeyPoint( int index ) const;

    // Clears images, keypoints and startIndices
    void clear();

    vector<Mat> images;
    vector<vector<KeyPoint> > points;

    // global indices of the first points in each image,
    // startIndices.size() = points.size()
    vector<int> startIndices;
};

/*
 *   Abstract interface for a keypoint descriptor
 */
class CV_EXPORTS GenericDescriptorMatch
{
public:
    enum IndexType
    {
        NoIndex,
        KDTreeIndex
    };

    GenericDescriptorMatch() {}
    virtual ~GenericDescriptorMatch() {}

    // Adds keypoints to the training set (descriptors are supposed to be calculated here)
    virtual void add( KeyPointCollection& keypoints );

    // Adds keypoints from a single image to the training set (descriptors are supposed to be calculated here)
    virtual void add( const Mat& image, vector<KeyPoint>& points ) = 0;

    // Classifies test keypoints
    // image    The source image
    // points   Test keypoints from the source image
    virtual void classify( const Mat& image, vector<KeyPoint>& points );

    // Matches test keypoints to the training set
    // image        The source image
    // points       Test keypoints from the source image
    // indices      A vector to be filled with keypoint class indices
    virtual void match( const Mat& image, vector<KeyPoint>& points, vector<int>& indices ) = 0;

    // Clears keypoints storing in collection
    virtual void clear();

    // Reads match object from a file node
    virtual void read( const FileNode& fn ) {};
    
    // Writes match object to a file storage
    virtual void write( FileStorage& fs ) const {};
    
protected:
    KeyPointCollection collection;
};

/*
 *  OneWayDescriptorMatch
 */
class CV_EXPORTS OneWayDescriptorMatch : public GenericDescriptorMatch
{
public:
    class Params
    {
    public:
        static const int POSE_COUNT = 500;
        static const int PATCH_WIDTH = 24;
        static const int PATCH_HEIGHT = 24;
        static float GET_MIN_SCALE() { return 0.7f; }
        static float GET_MAX_SCALE() { return 1.5f; }
        static float GET_STEP_SCALE() { return 1.2f; }

        Params( int _poseCount = POSE_COUNT,
                Size _patchSize = Size(PATCH_WIDTH, PATCH_HEIGHT),
                string _pcaFilename = string (),
                string _trainPath = string(),
                string _trainImagesList = string(),
                float _minScale = GET_MIN_SCALE(), float _maxScale = GET_MAX_SCALE(),
                float _stepScale = GET_STEP_SCALE() ) :
        poseCount(_poseCount), patchSize(_patchSize), pcaFilename(_pcaFilename),
        trainPath(_trainPath), trainImagesList(_trainImagesList),
        minScale(_minScale), maxScale(_maxScale), stepScale(_stepScale) {}

        int poseCount;
        Size patchSize;
        string pcaFilename;
        string trainPath;
        string trainImagesList;

        float minScale, maxScale, stepScale;
    };

    OneWayDescriptorMatch();

    // Equivalent to calling PointMatchOneWay() followed by Initialize(_params)
    OneWayDescriptorMatch( const Params& _params );
    virtual ~OneWayDescriptorMatch();

    // Sets one way descriptor parameters
    void initialize( const Params& _params, OneWayDescriptorBase *_base = 0 );

    // Calculates one way descriptors for a set of keypoints
    virtual void add( const Mat& image, vector<KeyPoint>& keypoints );

    // Calculates one way descriptors for a set of keypoints
    virtual void add( KeyPointCollection& keypoints );

    // Matches a set of keypoints from a single image of the training set. A rectangle with a center in a keypoint
    // and size (patch_width/2*scale, patch_height/2*scale) is cropped from the source image for each
    // keypoint. scale is iterated from DescriptorOneWayParams::min_scale to DescriptorOneWayParams::max_scale.
    // The minimum distance to each training patch with all its affine poses is found over all scales.
    // The class ID of a match is returned for each keypoint. The distance is calculated over PCA components
    // loaded with DescriptorOneWay::Initialize, kd tree is used for finding minimum distances.
    virtual void match( const Mat& image, vector<KeyPoint>& points, vector<int>& indices );

    // Classify a set of keypoints. The same as match, but returns point classes rather than indices
    virtual void classify( const Mat& image, vector<KeyPoint>& points );

    // Clears keypoints storing in collection and OneWayDescriptorBase
    virtual void clear ();

    // Reads match object from a file node
    virtual void read (const FileNode &fn);
    
    // Writes match object to a file storage
    virtual void write (FileStorage& fs) const;

protected:
    Ptr<OneWayDescriptorBase> base;
    Params params;
};

/*
 *  CalonderDescriptorMatch
 */
class CV_EXPORTS CalonderDescriptorMatch : public GenericDescriptorMatch
{
public:
    class Params
    {
    public:
        static const int DEFAULT_NUM_TREES = 80;
        static const int DEFAULT_DEPTH = 9;
        static const int DEFAULT_VIEWS = 5000;
        static const size_t DEFAULT_REDUCED_NUM_DIM = 176;
        static const size_t DEFAULT_NUM_QUANT_BITS = 4;
        static const int DEFAULT_PATCH_SIZE = PATCH_SIZE;

        Params( const RNG& _rng = RNG(), const PatchGenerator& _patchGen = PatchGenerator(),
                int _numTrees=DEFAULT_NUM_TREES,
                int _depth=DEFAULT_DEPTH,
                int _views=DEFAULT_VIEWS,
                size_t _reducedNumDim=DEFAULT_REDUCED_NUM_DIM,
                int _numQuantBits=DEFAULT_NUM_QUANT_BITS,
                bool _printStatus=true,
                int _patchSize=DEFAULT_PATCH_SIZE );
        Params( const string& _filename );

        RNG rng;
        PatchGenerator patchGen;
        int numTrees;
        int depth;
        int views;
        int patchSize;
        size_t reducedNumDim;
        int numQuantBits;
        bool printStatus;

        string filename;
    };

    CalonderDescriptorMatch();

    CalonderDescriptorMatch( const Params& _params );
    virtual ~CalonderDescriptorMatch();

    void initialize( const Params& _params );

    virtual void add( const Mat& image, vector<KeyPoint>& keypoints );

    virtual void match( const Mat& image, vector<KeyPoint>& keypoints, vector<int>& indices );

    virtual void classify( const Mat& image, vector<KeyPoint>& keypoints );

    virtual void clear ();

    virtual void read( const FileNode &fn );
    
    virtual void write( FileStorage& fs ) const;
    
protected:
    void trainRTreeClassifier();
    Mat extractPatch( const Mat& image, const Point& pt, int patchSize ) const;
    void calcBestProbAndMatchIdx( const Mat& image, const Point& pt,
                                  float& bestProb, int& bestMatchIdx, float* signature );

    Ptr<RTreeClassifier> classifier;
    Params params;
};

/*
 *  FernDescriptorMatch
 */
class CV_EXPORTS FernDescriptorMatch : public GenericDescriptorMatch
{
public:
    class Params
    {
    public:
        Params( int _nclasses=0,
                int _patchSize=FernClassifier::PATCH_SIZE,
                int _signatureSize=FernClassifier::DEFAULT_SIGNATURE_SIZE,
                int _nstructs=FernClassifier::DEFAULT_STRUCTS,
                int _structSize=FernClassifier::DEFAULT_STRUCT_SIZE,
                int _nviews=FernClassifier::DEFAULT_VIEWS,
                int _compressionMethod=FernClassifier::COMPRESSION_NONE,
                const PatchGenerator& patchGenerator=PatchGenerator() );

        Params( const string& _filename );

        int nclasses;
        int patchSize;
        int signatureSize;
        int nstructs;
        int structSize;
        int nviews;
        int compressionMethod;
        PatchGenerator patchGenerator;

        string filename;
    };

    FernDescriptorMatch();

    FernDescriptorMatch( const Params& _params );
    virtual ~FernDescriptorMatch();

    void initialize( const Params& _params );

    virtual void add( const Mat& image, vector<KeyPoint>& keypoints );

    virtual void match( const Mat& image, vector<KeyPoint>& keypoints, vector<int>& indices );

    virtual void classify( const Mat& image, vector<KeyPoint>& keypoints );

    virtual void clear ();

    virtual void read( const FileNode &fn );

    virtual void write( FileStorage& fs ) const;
    
protected:
    void trainFernClassifier();
    void calcBestProbAndMatchIdx( const Mat& image, const Point2f& pt,
                                  float& bestProb, int& bestMatchIdx, vector<float>& signature );
    Ptr<FernClassifier> classifier;
    Params params;
};

/****************************************************************************************\
*                                VectorDescriptorMatch                                   *
\****************************************************************************************/

/*
 *  A class used for matching descriptors that can be described as vectors in a finite-dimensional space
 */
template<class Extractor, class Matcher>
class CV_EXPORTS VectorDescriptorMatch : public GenericDescriptorMatch
{
public:
    using GenericDescriptorMatch::add;

    VectorDescriptorMatch( const Extractor& _extractor = Extractor(), const Matcher& _matcher = Matcher() ) :
                           extractor(_extractor), matcher(_matcher) {}

    ~VectorDescriptorMatch() {}

    // Builds flann index
    void index();

    // Calculates descriptors for a set of keypoints from a single image
    virtual void add( const Mat& image, vector<KeyPoint>& keypoints )
    {
        Mat descriptors;
        extractor.compute( image, keypoints, descriptors );
        matcher.add( descriptors );

        collection.add( Mat(), keypoints );
    };

    // Matches a set of keypoints with the training set
    virtual void match( const Mat& image, vector<KeyPoint>& points, vector<int>& keypointIndices )
    {
        Mat descriptors;
        extractor.compute( image, points, descriptors );

        matcher.match( descriptors, keypointIndices );
    };

    virtual void clear()
    {
        GenericDescriptorMatch::clear();
        matcher.clear();
    }

    virtual void read (const FileNode& fn)
    {
        GenericDescriptorMatch::read(fn);
        extractor.read (fn);
    }

    virtual void write (FileStorage& fs) const
    {
        GenericDescriptorMatch::write(fs);
        extractor.write (fs);
    }
protected:
    Extractor extractor;
    Matcher matcher;
    //vector<int> classIds;
};

}

#endif /* __cplusplus */

#endif

/* End of file. */
