Feature Detection
=================

.. index:: Canny

.. _Canny:

Canny
---------
.. c:function:: void Canny( const Mat& image, Mat& edges, double threshold1, double threshold2, int apertureSize=3, bool L2gradient=false )

    Finds edges in an image using Canny algorithm.

    :param image: Single-channel 8-bit input image

    :param edges: The output edge map. It will have the same size and the same type as  ``image``

    :param threshold1: The first threshold for the hysteresis procedure

    :param threshold2: The second threshold for the hysteresis procedure

    :param apertureSize: Aperture size for the  :func:`Sobel`  operator

    :param L2gradient: Indicates, whether the more accurate  :math:`L_2`  norm  :math:`=\sqrt{(dI/dx)^2 + (dI/dy)^2}`  should be used to compute the image gradient magnitude ( ``L2gradient=true`` ), or a faster default  :math:`L_1`  norm  :math:`=|dI/dx|+|dI/dy|`  is enough ( ``L2gradient=false`` )

The function finds edges in the input image ``image`` and marks them in the output map ``edges`` using the Canny algorithm. The smallest value between ``threshold1`` and ``threshold2`` is used for edge linking, the largest value is used to find the initial segments of strong edges, see
http://en.wikipedia.org/wiki/Canny_edge_detector

.. index:: cornerEigenValsAndVecs

.. _cornerEigenValsAndVecs:

cornerEigenValsAndVecs
----------------------

.. c:function:: void cornerEigenValsAndVecs( const Mat& src, Mat& dst, int blockSize, int apertureSize, int borderType=BORDER_DEFAULT )

    Calculates eigenvalues and eigenvectors of image blocks for corner detection.

    :param src: Input single-channel 8-bit or floating-point image

    :param dst: Image to store the results. It will have the same size as  ``src``  and the type  ``CV_32FC(6)``
    
    :param blockSize: Neighborhood size (see discussion)

    :param apertureSize: Aperture parameter for the  :func:`Sobel`  operator

    :param boderType: Pixel extrapolation method; see  :func:`borderInterpolate`

For every pixel
:math:`p` , the function ``cornerEigenValsAndVecs`` considers a ``blockSize`` :math:`\times` ``blockSize`` neigborhood
:math:`S(p)` . It calculates the covariation matrix of derivatives over the neighborhood as:

.. math::

    M =  \begin{bmatrix} \sum _{S(p)}(dI/dx)^2 &  \sum _{S(p)}(dI/dx dI/dy)^2  \\ \sum _{S(p)}(dI/dx dI/dy)^2 &  \sum _{S(p)}(dI/dy)^2 \end{bmatrix}

Where the derivatives are computed using
:func:`Sobel` operator.

After that it finds eigenvectors and eigenvalues of
:math:`M` and stores them into destination image in the form
:math:`(\lambda_1, \lambda_2, x_1, y_1, x_2, y_2)` where

* :math:`\lambda_1, \lambda_2` are the eigenvalues of :math:`M`; not sorted

* :math:`x_1, y_1` are the eigenvectors corresponding to :math:`\lambda_1`
    
* :math:`x_2, y_2` are the eigenvectors corresponding to :math:`\lambda_2`

The output of the function can be used for robust edge or corner detection.

See also:
:func:`cornerMinEigenVal`,:func:`cornerHarris`,:func:`preCornerDetect`

.. index:: cornerHarris

.. _cornerHarris:

cornerHarris
------------

.. c:function:: void cornerHarris( const Mat& src, Mat& dst, int blockSize, int apertureSize, double k, int borderType=BORDER_DEFAULT )

    Harris edge detector.

    :param src: Input single-channel 8-bit or floating-point image

    :param dst: Image to store the Harris detector responses; will have type  ``CV_32FC1``  and the same size as  ``src``
    
    :param blockSize: Neighborhood size (see the discussion of  :func:`cornerEigenValsAndVecs` )

    :param apertureSize: Aperture parameter for the  :func:`Sobel`  operator

    :param k: Harris detector free parameter. See the formula below

    :param boderType: Pixel extrapolation method; see  :func:`borderInterpolate`

The function runs the Harris edge detector on the image. Similarly to
:func:`cornerMinEigenVal` and
:func:`cornerEigenValsAndVecs` , for each pixel
:math:`(x, y)` it calculates a
:math:`2\times2` gradient covariation matrix
:math:`M^{(x,y)}` over a
:math:`\texttt{blockSize} \times \texttt{blockSize}` neighborhood. Then, it computes the following characteristic:

.. math::

    \texttt{dst} (x,y) =  \mathrm{det} M^{(x,y)} - k  \cdot \left ( \mathrm{tr} M^{(x,y)} \right )^2

Corners in the image can be found as the local maxima of this response map.

.. index:: cornerMinEigenVal

.. _cornerMinEigenVal:

cornerMinEigenVal
-----------------

.. c:function:: void cornerMinEigenVal( const Mat& src, Mat& dst, int blockSize, int apertureSize=3, int borderType=BORDER_DEFAULT )

    Calculates the minimal eigenvalue of gradient matrices for corner detection.

    :param src: Input single-channel 8-bit or floating-point image

    :param dst: Image to store the minimal eigenvalues; will have type  ``CV_32FC1``  and the same size as  ``src``
    
    :param blockSize: Neighborhood size (see the discussion of  :func:`cornerEigenValsAndVecs` )

    :param apertureSize: Aperture parameter for the  :func:`Sobel`  operator

    :param boderType: Pixel extrapolation method; see  :func:`borderInterpolate`

The function is similar to
:func:`cornerEigenValsAndVecs` but it calculates and stores only the minimal eigenvalue of the covariation matrix of derivatives, i.e.
:math:`\min(\lambda_1, \lambda_2)` in terms of the formulae in
:func:`cornerEigenValsAndVecs` description.

.. index:: cornerSubPix

.. _cornerSubPix:

cornerSubPix
----------------
.. c:function:: void cornerSubPix( const Mat& image, vector<Point2f>& corners, Size winSize, Size zeroZone, TermCriteria criteria )

    Refines the corner locations.

    :param image: Input image

    :param corners: Initial coordinates of the input corners; refined coordinates on output

    :param winSize: Half of the side length of the search window. For example, if  ``winSize=Size(5,5)`` , then a  :math:`5*2+1 \times 5*2+1 = 11 \times 11`  search window would be used

    :param zeroZone: Half of the size of the dead region in the middle of the search zone over which the summation in the formula below is not done. It is used sometimes to avoid possible singularities of the autocorrelation matrix. The value of (-1,-1) indicates that there is no such size

    :param criteria: Criteria for termination of the iterative process of corner refinement. That is, the process of corner position refinement stops either after a certain number of iterations or when a required accuracy is achieved. The  ``criteria``  may specify either of or both the maximum number of iteration and the required accuracy

The function iterates to find the sub-pixel accurate location of corners, or radial saddle points, as shown in on the picture below.

.. image:: pics/cornersubpix.png

Sub-pixel accurate corner locator is based on the observation that every vector from the center
:math:`q` to a point
:math:`p` located within a neighborhood of
:math:`q` is orthogonal to the image gradient at
:math:`p` subject to image and measurement noise. Consider the expression:

.. math::

    \epsilon _i = {DI_{p_i}}^T  \cdot (q - p_i)

where
:math:`{DI_{p_i}}` is the image gradient at the one of the points
:math:`p_i` in a neighborhood of
:math:`q` . The value of
:math:`q` is to be found such that
:math:`\epsilon_i` is minimized. A system of equations may be set up with
:math:`\epsilon_i` set to zero:

.. math::

    \sum _i(DI_{p_i}  \cdot {DI_{p_i}}^T) -  \sum _i(DI_{p_i}  \cdot {DI_{p_i}}^T  \cdot p_i)

where the gradients are summed within a neighborhood ("search window") of
:math:`q` . Calling the first gradient term
:math:`G` and the second gradient term
:math:`b` gives:

.. math::

    q = G^{-1}  \cdot b

The algorithm sets the center of the neighborhood window at this new center
:math:`q` and then iterates until the center keeps within a set threshold.

.. index:: goodFeaturesToTrack

.. _goodFeaturesToTrack:

goodFeaturesToTrack
-------------------

.. c:function:: void goodFeaturesToTrack( const Mat& image, vector<Point2f>& corners, int maxCorners, double qualityLevel, double minDistance, const Mat& mask=Mat(), int blockSize=3, bool useHarrisDetector=false, double k=0.04 )

    Determines strong corners on an image.

    :param image: The input 8-bit or floating-point 32-bit, single-channel image

    :param corners: The output vector of detected corners

    :param maxCorners: The maximum number of corners to return. If there are more corners than that will be found, the strongest of them will be returned

    :param qualityLevel: Characterizes the minimal accepted quality of image corners; the value of the parameter is multiplied by the by the best corner quality measure (which is the min eigenvalue, see  :func:`cornerMinEigenVal` , or the Harris function response, see  :func:`cornerHarris` ). The corners, which quality measure is less than the product, will be rejected. For example, if the best corner has the quality measure = 1500, and the  ``qualityLevel=0.01`` , then all the corners which quality measure is less than 15 will be rejected.

    :param minDistance: The minimum possible Euclidean distance between the returned corners

    :param mask: The optional region of interest. If the image is not empty (then it needs to have the type  ``CV_8UC1``  and the same size as  ``image`` ), it will specify the region in which the corners are detected

    :param blockSize: Size of the averaging block for computing derivative covariation matrix over each pixel neighborhood, see  :func:`cornerEigenValsAndVecs`
    
    :param useHarrisDetector: Indicates, whether to use   operator or  :func:`cornerMinEigenVal`
    
    :param k: Free parameter of Harris detector

The function finds the most prominent corners in the image or in the specified image region, as described in Shi94:

#.
    the function first calculates the corner quality measure at every source image pixel using the
    :func:`cornerMinEigenVal`     or
    :func:`cornerHarris`
    
#.
    then it performs non-maxima suppression (the local maxima in
    :math:`3\times 3`     neighborhood
    are retained).

#.
    the next step rejects the corners with the minimal eigenvalue less than
    :math:`\texttt{qualityLevel} \cdot \max_{x,y} qualityMeasureMap(x,y)`     .

#.
    the remaining corners are then sorted by the quality measure in the descending order.

#.
    finally, the function throws away each corner
    :math:`pt_j`     if there is a stronger corner
    :math:`pt_i`     (
    :math:`i < j`     ) such that the distance between them is less than ``minDistance``
    
The function can be used to initialize a point-based tracker of an object.

Note that the if the function is called with different values ``A`` and ``B`` of the parameter ``qualityLevel`` , and ``A`` > {B}, the vector of returned corners with ``qualityLevel=A`` will be the prefix of the output vector with ``qualityLevel=B`` .

See also: :func:`cornerMinEigenVal`, :func:`cornerHarris`, :func:`calcOpticalFlowPyrLK`, :func:`estimateRigidMotion`, :func:`PlanarObjectDetector`, :func:`OneWayDescriptor`

.. index:: HoughCircles

.. _HoughCircles:

HoughCircles
------------

.. c:function:: void HoughCircles( Mat& image, vector<Vec3f>& circles, int method, double dp, double minDist, double param1=100, double param2=100, int minRadius=0, int maxRadius=0 )

    Finds circles in a grayscale image using a Hough transform.

    :param image: The 8-bit, single-channel, grayscale input image

    :param circles: The output vector of found circles. Each vector is encoded as 3-element floating-point vector  :math:`(x, y, radius)`
    
    :param method: Currently, the only implemented method is  ``CV_HOUGH_GRADIENT`` , which is basically  *21HT* , described in  Yuen90 .

    :param dp: The inverse ratio of the accumulator resolution to the image resolution. For example, if  ``dp=1`` , the accumulator will have the same resolution as the input image, if  ``dp=2``  - accumulator will have half as big width and height, etc

    :param minDist: Minimum distance between the centers of the detected circles. If the parameter is too small, multiple neighbor circles may be falsely detected in addition to a true one. If it is too large, some circles may be missed

    :param param1: The first method-specific parameter. in the case of  ``CV_HOUGH_GRADIENT``  it is the higher threshold of the two passed to  :func:`Canny`  edge detector (the lower one will be twice smaller)

    :param param2: The second method-specific parameter. in the case of  ``CV_HOUGH_GRADIENT``  it is the accumulator threshold at the center detection stage. The smaller it is, the more false circles may be detected. Circles, corresponding to the larger accumulator values, will be returned first

    :param minRadius: Minimum circle radius

    :param maxRadius: Maximum circle radius

The function finds circles in a grayscale image using some modification of Hough transform. Here is a short usage example: ::

    #include <cv.h>
    #include <highgui.h>
    #include <math.h>

    using namespace cv;

    int main(int argc, char** argv)
    {
        Mat img, gray;
        if( argc != 2 && !(img=imread(argv[1], 1)).data)
            return -1;
        cvtColor(img, gray, CV_BGR2GRAY);
        // smooth it, otherwise a lot of false circles may be detected
        GaussianBlur( gray, gray, Size(9, 9), 2, 2 );
        vector<Vec3f> circles;
        HoughCircles(gray, circles, CV_HOUGH_GRADIENT,
                     2, gray->rows/4, 200, 100 );
        for( size_t i = 0; i < circles.size(); i++ )
        {
             Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
             int radius = cvRound(circles[i][2]);
             // draw the circle center
             circle( img, center, 3, Scalar(0,255,0), -1, 8, 0 );
             // draw the circle outline
             circle( img, center, radius, Scalar(0,0,255), 3, 8, 0 );
        }
        namedWindow( "circles", 1 );
        imshow( "circles", img );
        return 0;
    }

Note that usually the function detects the circles' centers well, however it may fail to find the correct radii. You can assist the function by specifying the radius range ( ``minRadius`` and ``maxRadius`` ) if you know it, or you may ignore the returned radius, use only the center and find the correct radius using some additional procedure.

See also:
:func:`fitEllipse`,:func:`minEnclosingCircle`

.. index:: HoughLines

.. _HoughLines:

HoughLines
----------

.. c:function:: void HoughLines( Mat& image, vector<Vec2f>& lines, double rho, double theta, int threshold, double srn=0, double stn=0 )

    Finds lines in a binary image using standard Hough transform.

    :param image: The 8-bit, single-channel, binary source image. The image may be modified by the function

    :param lines: The output vector of lines. Each line is represented by a two-element vector  :math:`(\rho, \theta)` .  :math:`\rho`  is the distance from the coordinate origin  :math:`(0,0)`  (top-left corner of the image) and  :math:`\theta`  is the line rotation angle in radians ( :math:`0 \sim \textrm{vertical line}, \pi/2 \sim \textrm{horizontal line}` )

    :param rho: Distance resolution of the accumulator in pixels

    :param theta: Angle resolution of the accumulator in radians

    :param threshold: The accumulator threshold parameter. Only those lines are returned that get enough votes ( :math:`>\texttt{threshold}` )

    :param srn: For the multi-scale Hough transform it is the divisor for the distance resolution  ``rho`` . The coarse accumulator distance resolution will be  ``rho``  and the accurate accumulator resolution will be  ``rho/srn`` . If both  ``srn=0``  and  ``stn=0``  then the classical Hough transform is used, otherwise both these parameters should be positive.

    :param stn: For the multi-scale Hough transform it is the divisor for the distance resolution  ``theta``
    
The function implements standard or standard multi-scale Hough transform algorithm for line detection. See
:func:`HoughLinesP` for the code example.

.. index:: HoughLinesP

.. _HoughLinesP:

HoughLinesP
-----------

.. c:function:: void HoughLinesP( Mat& image, vector<Vec4i>& lines, double rho, double theta, int threshold, double minLineLength=0, double maxLineGap=0 )

    Finds lines segments in a binary image using probabilistic Hough transform.

    :param image: The 8-bit, single-channel, binary source image. The image may be modified by the function

    :param lines: The output vector of lines. Each line is represented by a 4-element vector  :math:`(x_1, y_1, x_2, y_2)` , where  :math:`(x_1,y_1)`  and  :math:`(x_2, y_2)`  are the ending points of each line segment detected.

    :param rho: Distance resolution of the accumulator in pixels

    :param theta: Angle resolution of the accumulator in radians

    :param threshold: The accumulator threshold parameter. Only those lines are returned that get enough votes ( :math:`>\texttt{threshold}` )

    :param minLineLength: The minimum line length. Line segments shorter than that will be rejected

    :param maxLineGap: The maximum allowed gap between points on the same line to link them.

The function implements probabilistic Hough transform algorithm for line detection, described in
Matas00
. Below is line detection example: ::

    /* This is a standalone program. Pass an image name as a first parameter
    of the program.  Switch between standard and probabilistic Hough transform
    by changing "#if 1" to "#if 0" and back */
    #include <cv.h>
    #include <highgui.h>
    #include <math.h>

    using namespace cv;

    int main(int argc, char** argv)
    {
        Mat src, dst, color_dst;
        if( argc != 2 || !(src=imread(argv[1], 0)).data)
            return -1;

        Canny( src, dst, 50, 200, 3 );
        cvtColor( dst, color_dst, CV_GRAY2BGR );

    #if 0
        vector<Vec2f> lines;
        HoughLines( dst, lines, 1, CV_PI/180, 100 );

        for( size_t i = 0; i < lines.size(); i++ )
        {
            float rho = lines[i][0];
            float theta = lines[i][1];
            double a = cos(theta), b = sin(theta);
            double x0 = a*rho, y0 = b*rho;
            Point pt1(cvRound(x0 + 1000*(-b)),
                      cvRound(y0 + 1000*(a)));
            Point pt2(cvRound(x0 - 1000*(-b)),
                      cvRound(y0 - 1000*(a)));
            line( color_dst, pt1, pt2, Scalar(0,0,255), 3, 8 );
        }
    #else
        vector<Vec4i> lines;
        HoughLinesP( dst, lines, 1, CV_PI/180, 80, 30, 10 );
        for( size_t i = 0; i < lines.size(); i++ )
        {
            line( color_dst, Point(lines[i][0], lines[i][1]),
                Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8 );
        }
    #endif
        namedWindow( "Source", 1 );
        imshow( "Source", src );

        namedWindow( "Detected Lines", 1 );
        imshow( "Detected Lines", color_dst );

        waitKey(0);
        return 0;
    }

This is the sample picture the function parameters have been tuned for:

.. image:: pics/building.jpg

And this is the output of the above program in the case of probabilistic Hough transform

.. image:: pics/houghp.png

.. index:: preCornerDetect

.. _preCornerDetect:

preCornerDetect
---------------

.. c:function:: void preCornerDetect( const Mat& src, Mat& dst, int apertureSize, int borderType=BORDER_DEFAULT )

    Calculates the feature map for corner detection

    :param src: The source single-channel 8-bit of floating-point image

    :param dst: The output image; will have type  ``CV_32F``  and the same size as  ``src``
    
    :param apertureSize: Aperture size of  :func:`Sobel`
    
    :param borderType: The pixel extrapolation method; see  :func:`borderInterpolate`
    
The function calculates the complex spatial derivative-based function of the source image

.. math::

    \texttt{dst} = (D_x  \texttt{src} )^2  \cdot D_{yy}  \texttt{src} + (D_y  \texttt{src} )^2  \cdot D_{xx}  \texttt{src} - 2 D_x  \texttt{src} \cdot D_y  \texttt{src} \cdot D_{xy}  \texttt{src}

where
:math:`D_x`,:math:`D_y` are the first image derivatives,
:math:`D_{xx}`,:math:`D_{yy}` are the second image derivatives and
:math:`D_{xy}` is the mixed derivative.

The corners can be found as local maximums of the functions, as shown below: ::

    Mat corners, dilated_corners;
    preCornerDetect(image, corners, 3);
    // dilation with 3x3 rectangular structuring element
    dilate(corners, dilated_corners, Mat(), 1);
    Mat corner_mask = corners == dilated_corners;


