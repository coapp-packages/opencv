Common Interfaces of Feature Detectors
======================================

.. highlight:: cpp

Feature detectors in OpenCV have wrappers with a common interface that enables you to easily switch
between different algorithms solving the same problem. All objects that implement keypoint detectors
inherit the
:ref:`FeatureDetector` interface.

.. index:: KeyPoint

.. KeyPoint:

KeyPoint
--------
.. cpp:class:: KeyPoint

Data structure for salient point detectors ::

    class KeyPoint
    {
    public:
        // the default constructor
        KeyPoint() : pt(0,0), size(0), angle(-1), response(0), octave(0),
                     class_id(-1) {}
        // the full constructor
        KeyPoint(Point2f _pt, float _size, float _angle=-1,
                float _response=0, int _octave=0, int _class_id=-1)
                : pt(_pt), size(_size), angle(_angle), response(_response),
                  octave(_octave), class_id(_class_id) {}
        // another form of the full constructor
        KeyPoint(float x, float y, float _size, float _angle=-1,
                float _response=0, int _octave=0, int _class_id=-1)
                : pt(x, y), size(_size), angle(_angle), response(_response),
                  octave(_octave), class_id(_class_id) {}
        // converts vector of keypoints to vector of points
        static void convert(const std::vector<KeyPoint>& keypoints,
                            std::vector<Point2f>& points2f,
                            const std::vector<int>& keypointIndexes=std::vector<int>());
        // converts vector of points to the vector of keypoints, where each
        // keypoint is assigned to the same size and the same orientation
        static void convert(const std::vector<Point2f>& points2f,
                            std::vector<KeyPoint>& keypoints,
                            float size=1, float response=1, int octave=0,
                            int class_id=-1);

        // computes overlap for pair of keypoints;
        // overlap is a ratio between area of keypoint regions intersection and
        // area of keypoint regions union (now keypoint region is a circle)
        static float overlap(const KeyPoint& kp1, const KeyPoint& kp2);

        Point2f pt; // coordinates of the keypoints
        float size; // diameter of the meaningful keypoint neighborhood
        float angle; // computed orientation of the keypoint (-1 if not applicable)
        float response; // the response by which the most strong keypoints
                        // have been selected. Can be used for further sorting
                        // or subsampling
        int octave; // octave (pyramid layer) from which the keypoint has been extracted
        int class_id; // object class (if the keypoints need to be clustered by
                      // an object they belong to)
    };

    // writes vector of keypoints to the file storage
    void write(FileStorage& fs, const string& name, const vector<KeyPoint>& keypoints);
    // reads vector of keypoints from the specified file storage node
    void read(const FileNode& node, CV_OUT vector<KeyPoint>& keypoints);

..


.. index:: FeatureDetector

.. _FeatureDetector:

FeatureDetector
---------------
.. cpp:class:: FeatureDetector

Abstract base class for 2D image feature detectors ::

    class CV_EXPORTS FeatureDetector
    {
    public:
        virtual ~FeatureDetector();

        void detect( const Mat& image, vector<KeyPoint>& keypoints,
                     const Mat& mask=Mat() ) const;

        void detect( const vector<Mat>& images,
                     vector<vector<KeyPoint> >& keypoints,
                     const vector<Mat>& masks=vector<Mat>() ) const;

        virtual void read(const FileNode&);
        virtual void write(FileStorage&) const;

        static Ptr<FeatureDetector> create( const string& detectorType );

    protected:
    ...
    };


.. index:: FeatureDetector::detect

FeatureDetector::detect
---------------------------
.. cpp:function:: void FeatureDetector::detect( const Mat& image,                                vector<KeyPoint>& keypoints,                                 const Mat& mask=Mat() ) const

    Detects keypoints in an image (first variant) or image set (second variant).

    :param image: Image.

    :param keypoints: Detected keypoints.

    :param mask: Mask specifying where to look for keypoints (optional). It must be a char matrix with non-zero values in the region of interest.

.. cpp:function:: void FeatureDetector::detect( const vector<Mat>& images,                                                            vector<vector<KeyPoint> >& keypoints,                                                             const vector<Mat>& masks=vector<Mat>() ) const

    :param images: Image set.

    :param keypoints: Collection of keypoints detected in input images. ``keypoints[i]`` is a set of keypoints detected in ``images[i]`` .

    :param masks: Masks for each input image specifying where to look for keypoints (optional). ``masks[i]`` is a mask for ``images[i]`` .                     Each element of the ``masks``  vector must be a char matrix with non-zero values in the region of interest.

.. index:: FeatureDetector::read

FeatureDetector::read
-------------------------
.. cpp:function:: void FeatureDetector::read( const FileNode& fn )

    Reads a feature detector object from a file node.

    :param fn: File node from which the detector is read.

.. index:: FeatureDetector::write

FeatureDetector::write
--------------------------
.. cpp:function:: void FeatureDetector::write( FileStorage& fs ) const

    Writes a feature detector object to a file storage.

    :param fs: File storage where the detector is written.

.. index:: FeatureDetector::create

FeatureDetector::create
---------------------------
.. cpp:function:: Ptr<FeatureDetector> FeatureDetector::create( const string& detectorType )

    Creates a feature detector by its name.

    :param detectorType: Feature detector type.

The following detector types are supported:

* ``"FAST"`` -- :ref:`FastFeatureDetector`
* ``"STAR"`` -- :ref:`StarFeatureDetector`
* ``"SIFT"`` -- :ref:`SiftFeatureDetector`
* ``"SURF"`` -- :ref:`SurfFeatureDetector`
* ``"MSER"`` -- :ref:`MserFeatureDetector`
* ``"GFTT"`` -- :ref:`GfttFeatureDetector`
* ``"HARRIS"`` -- :ref:`HarrisFeatureDetector`

Also a combined format is supported: feature detector adapter name ( ``"Grid"`` --
:ref:`GridAdaptedFeatureDetector`, ``"Pyramid"`` --
:ref:`PyramidAdaptedFeatureDetector` ) + feature detector name (see above),
for example: ``"GridFAST"``, ``"PyramidSTAR"`` .

.. index:: FastFeatureDetector

.. _FastFeatureDetector:

FastFeatureDetector
-------------------
.. cpp:class:: FastFeatureDetector

Wrapping class for feature detection using the
:ref:`FAST` method ::

    class FastFeatureDetector : public FeatureDetector
    {
    public:
        FastFeatureDetector( int threshold=1, bool nonmaxSuppression=true );
        virtual void read( const FileNode& fn );
        virtual void write( FileStorage& fs ) const;
    protected:
        ...
    };


.. index:: GoodFeaturesToTrackDetector

.. _GoodFeaturesToTrackDetector:

GoodFeaturesToTrackDetector
---------------------------
.. cpp:class:: GoodFeaturesToTrackDetector

Wrapping class for feature detection using the
:ref:`goodFeaturesToTrack` function ::

    class GoodFeaturesToTrackDetector : public FeatureDetector
    {
    public:
        class Params
        {
        public:
            Params( int maxCorners=1000, double qualityLevel=0.01,
                    double minDistance=1., int blockSize=3,
                    bool useHarrisDetector=false, double k=0.04 );
            void read( const FileNode& fn );
            void write( FileStorage& fs ) const;

            int maxCorners;
            double qualityLevel;
            double minDistance;
            int blockSize;
            bool useHarrisDetector;
            double k;
        };

        GoodFeaturesToTrackDetector( const GoodFeaturesToTrackDetector::Params& params=
                                                GoodFeaturesToTrackDetector::Params() );
        GoodFeaturesToTrackDetector( int maxCorners, double qualityLevel,
                                     double minDistance, int blockSize=3,
                                     bool useHarrisDetector=false, double k=0.04 );
        virtual void read( const FileNode& fn );
        virtual void write( FileStorage& fs ) const;
    protected:
        ...
    };


.. index:: MserFeatureDetector

.. _MserFeatureDetector:

MserFeatureDetector
-------------------
.. cpp:class:: MserFeatureDetector

Wrapping class for feature detection using the
:ref:`MSER` class ::

    class MserFeatureDetector : public FeatureDetector
    {
    public:
        MserFeatureDetector( CvMSERParams params=cvMSERParams() );
        MserFeatureDetector( int delta, int minArea, int maxArea,
                             double maxVariation, double minDiversity,
                             int maxEvolution, double areaThreshold,
                             double minMargin, int edgeBlurSize );
        virtual void read( const FileNode& fn );
        virtual void write( FileStorage& fs ) const;
    protected:
        ...
    };


.. index:: StarFeatureDetector

.. _StarFeatureDetector:

StarFeatureDetector
-------------------
.. cpp:class:: StarFeatureDetector

Wrapping class for feature detection using the
:ref:`StarDetector` class ::

    class StarFeatureDetector : public FeatureDetector
    {
    public:
        StarFeatureDetector( int maxSize=16, int responseThreshold=30,
                             int lineThresholdProjected = 10,
                             int lineThresholdBinarized=8, int suppressNonmaxSize=5 );
        virtual void read( const FileNode& fn );
        virtual void write( FileStorage& fs ) const;
    protected:
        ...
    };


.. index:: SiftFeatureDetector

.. _SiftFeatureDetector:

SiftFeatureDetector
-------------------
.. cpp:class:: SiftFeatureDetector

Wrapping class for feature detection using the
:ref:`SIFT` class ::

    class SiftFeatureDetector : public FeatureDetector
    {
    public:
        SiftFeatureDetector(
            const SIFT::DetectorParams& detectorParams=SIFT::DetectorParams(),
            const SIFT::CommonParams& commonParams=SIFT::CommonParams() );
        SiftFeatureDetector( double threshold, double edgeThreshold,
                             int nOctaves=SIFT::CommonParams::DEFAULT_NOCTAVES,
                             int nOctaveLayers=SIFT::CommonParams::DEFAULT_NOCTAVE_LAYERS,
                             int firstOctave=SIFT::CommonParams::DEFAULT_FIRST_OCTAVE,
                             int angleMode=SIFT::CommonParams::FIRST_ANGLE );
        virtual void read( const FileNode& fn );
        virtual void write( FileStorage& fs ) const;
    protected:
        ...
    };


.. index:: SurfFeatureDetector

.. _SurfFeatureDetector:

SurfFeatureDetector
-------------------
.. cpp:class:: SurfFeatureDetector

Wrapping class for feature detection using the
:ref:`SURF` class ::

    class SurfFeatureDetector : public FeatureDetector
    {
    public:
        SurfFeatureDetector( double hessianThreshold = 400., int octaves = 3,
                             int octaveLayers = 4 );
        virtual void read( const FileNode& fn );
        virtual void write( FileStorage& fs ) const;
    protected:
        ...
    };


.. index:: GridAdaptedFeatureDetector

.. _GridAdaptedFeatureDetector:

GridAdaptedFeatureDetector
--------------------------
.. cpp:class:: GridAdaptedFeatureDetector

Class adapting a detector to partition the source image into a grid and detect points in each cell ::

    class GridAdaptedFeatureDetector : public FeatureDetector
    {
    public:
        /*
         * detector            Detector that will be adapted.
         * maxTotalKeypoints   Maximum count of keypoints detected on the image.
         *                     Only the strongest keypoints will be kept.
         * gridRows            Grid row count.
         * gridCols            Grid column count.
         */
        GridAdaptedFeatureDetector( const Ptr<FeatureDetector>& detector,
                                    int maxTotalKeypoints, int gridRows=4,
                                    int gridCols=4 );
        virtual void read( const FileNode& fn );
        virtual void write( FileStorage& fs ) const;
    protected:
        ...
    };


.. index:: PyramidAdaptedFeatureDetector

.. _PyramidAdaptedFeatureDetector:

PyramidAdaptedFeatureDetector
-----------------------------
.. cpp:class:: PyramidAdaptedFeatureDetector

Class adapting a detector to detect points over multiple levels of a Gaussian pyramid. Consider using this class for detectors that are not inherently scaled. ::

    class PyramidAdaptedFeatureDetector : public FeatureDetector
    {
    public:
        PyramidAdaptedFeatureDetector( const Ptr<FeatureDetector>& detector,
                                       int levels=2 );
        virtual void read( const FileNode& fn );
        virtual void write( FileStorage& fs ) const;
    protected:
        ...
    };


.. index:: DynamicAdaptedFeatureDetector

DynamicAdaptedFeatureDetector
-----------------------------

.. cpp:class:: DynamicAdaptedFeatureDetector

Adaptively adjusting detector that iteratively detects features until the desired number is found ::

       class DynamicAdaptedFeatureDetector: public FeatureDetector
       {
       public:
           DynamicAdaptedFeatureDetector( const Ptr<AdjusterAdapter>& adjuster,
               int min_features=400, int max_features=500, int max_iters=5 );
           ...
       };

If the detector is persisted, it "remembers" the parameters
used for the last detection. In this case, the detector may be used for consistent numbers
of keypoints in a set of temporally related images, such as video streams or
panorama series.

``DynamicAdaptedFeatureDetector``  uses another detector such as FAST or SURF to do the dirty work,
with the help of ``AdjusterAdapter`` .
If the detected number of features is not large enough,
``AdjusterAdapter`` adjusts the detection parameters so that the next detection 
results in a bigger or smaller number of features.  This is repeated until either the number of desired features are found
or the parameters are maxed out.

Adapters can be easily implemented for any detector via the
``AdjusterAdapter`` interface.

Beware that this is not thread-safe since the adjustment of parameters requires modification of the feature detector class instance.

Example of creating ``DynamicAdaptedFeatureDetector`` : ::

    //sample usage:
    //will create a detector that attempts to find
    //100 - 110 FAST Keypoints, and will at most run
    //FAST feature detection 10 times until that
    //number of keypoints are found
    Ptr<FeatureDetector> detector(new DynamicAdaptedFeatureDetector (100, 110, 10,
                                  new FastAdjuster(20,true)));



.. index:: DynamicAdaptedFeatureDetector::DynamicAdaptedFeatureDetector

DynamicAdaptedFeatureDetector::DynamicAdaptedFeatureDetector
----------------------------------------------------------------
.. cpp:function:: DynamicAdaptedFeatureDetector::DynamicAdaptedFeatureDetector(       const Ptr<AdjusterAdapter>& adjuster,       int min_features,   int max_features,   int max_iters )

    Constructs the class.

    :param adjuster:  :ref:`AdjusterAdapter`  that detects features and adjusts parameters.

    :param min_features: Minimum desired number of features.

    :param max_features: Maximum desired number of features.

    :param max_iters: Maximum number of times to try adjusting the feature detector parameters. For :ref:`FastAdjuster` , this number can be high, but with ``Star`` or ``Surf``  many iterations can be time-comsuming.  At each iteration the detector is rerun. 

.. index:: AdjusterAdapter

AdjusterAdapter
---------------

.. cpp:class:: AdjusterAdapter

Class providing an interface for adjusting parameters of a feature detector. This interface is used by :ref:`DynamicAdaptedFeatureDetector` . It is a wrapper for :ref:`FeatureDetector` that enables adjusting parameters after feature detection. ::
  
     class AdjusterAdapter: public FeatureDetector
     {
     public:
         virtual ~AdjusterAdapter() {}
         virtual void tooFew(int min, int n_detected) = 0;
         virtual void tooMany(int max, int n_detected) = 0;
         virtual bool good() const = 0;
     };


See
:ref:`FastAdjuster`,
:ref:`StarAdjuster`,
:ref:`SurfAdjuster` for concrete implementations.


.. index:: AdjusterAdapter::tooFew

AdjusterAdapter::tooFew
---------------------------
.. cpp:function:: void AdjusterAdapter::tooFew(int min, int n_detected)

    Adjusts the detector parameters to detect more features.

    :param min: Minimum desired number of features.

    :param n_detected: Number of features detected during the latest run.

Example: ::

    void FastAdjuster::tooFew(int min, int n_detected)
    {
            thresh_--;
    }


.. index:: AdjusterAdapter::tooMany

AdjusterAdapter::tooMany
----------------------------
.. cpp:function:: void AdjusterAdapter::tooMany(int max, int n_detected)

    Adjusts the detector parameters to detect less features.

    :param max: Maximum desired number of features.

    :param n_detected: Number of features detected during the latest run.

Example: ::

    void FastAdjuster::tooMany(int min, int n_detected)
    {
            thresh_++;
    }


.. index:: AdjusterAdapter::good

AdjusterAdapter::good
-------------------------
.. cpp:function:: bool AdjusterAdapter::good() const

    Returns false if the detector parameters cannot be adjusted any more. 

Example: ::

        bool FastAdjuster::good() const
        {
                return (thresh_ > 1) && (thresh_ < 200);
        }


.. index:: FastAdjuster

FastAdjuster
------------

.. cpp:class:: FastAdjuster

:ref:`AdjusterAdapter` for :ref:`FastFeatureDetector`. This class decreases or increases the threshold value by 1. ::

        class FastAdjuster FastAdjuster: public AdjusterAdapter
        {
        public:
                FastAdjuster(int init_thresh = 20, bool nonmax = true);
                ...
        };

.. index:: StarAdjuster

StarAdjuster
------------

.. cpp:class:: StarAdjuster

:ref:`AdjusterAdapter` for :ref:`StarFeatureDetector`. This class adjusts the ``responseThreshhold`` of ``StarFeatureDetector``.  ::

        class StarAdjuster: public AdjusterAdapter
        {
                StarAdjuster(double initial_thresh = 30.0);
                ...
        };

.. index:: SurfAdjuster

SurfAdjuster
------------

.. cpp:class:: SurfAdjuster

:ref:`AdjusterAdapter` for :ref:`SurfFeatureDetector`. This class adjusts the ``hessianThreshold`` of ``SurfFeatureDetector``. ::

        class SurfAdjuster: public SurfAdjuster
        {
                SurfAdjuster();
                ...
        };

.. index:: FeatureDetector

FeatureDetector
---------------
.. cpp:class:: FeatureDetector

Abstract base class for 2D image feature detectors ::

    class CV_EXPORTS FeatureDetector
    {
    public:
        virtual ~FeatureDetector();

        void detect( const Mat& image, vector<KeyPoint>& keypoints,
                     const Mat& mask=Mat() ) const;

        void detect( const vector<Mat>& images,
                     vector<vector<KeyPoint> >& keypoints,
                     const vector<Mat>& masks=vector<Mat>() ) const;

        virtual void read(const FileNode&);
        virtual void write(FileStorage&) const;

        static Ptr<FeatureDetector> create( const string& detectorType );

    protected:
    ...
    };
