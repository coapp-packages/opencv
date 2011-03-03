Image Filtering
===============

.. highlight:: cpp

Functions and classes described in this section are used to perform various linear or non-linear filtering operations on 2D images (represented as
:func:`Mat` 's), that is, for each pixel location
:math:`(x,y)` in the source image some its (normally rectangular) neighborhood is considered and used to compute the response. In case of a linear filter it is a weighted sum of pixel values, in case of morphological operations it is the minimum or maximum etc. The computed response is stored to the destination image at the same location
:math:`(x,y)` . It means, that the output image will be of the same size as the input image. Normally, the functions supports multi-channel arrays, in which case every channel is processed independently, therefore the output image will also have the same number of channels as the input one.

Another common feature of the functions and classes described in this section is that, unlike simple arithmetic functions, they need to extrapolate values of some non-existing pixels. For example, if we want to smooth an image using a Gaussian
:math:`3 \times 3` filter, then during the processing of the left-most pixels in each row we need pixels to the left of them, i.e. outside of the image. We can let those pixels be the same as the left-most image pixels (i.e. use "replicated border" extrapolation method), or assume that all the non-existing pixels are zeros ("contant border" extrapolation method) etc.
OpenCV let the user to specify the extrapolation method; see the function  :func:`borderInterpolate`  and discussion of  ``borderType``  parameter in various functions below.

.. index:: BaseColumnFilter

.. _BaseColumnFilter:

BaseColumnFilter
----------------
.. c:type:: BaseColumnFilter

Base class for filters with single-column kernels ::

    class BaseColumnFilter
    {
    public:
        virtual ~BaseColumnFilter();

        // To be overriden by the user.
        //
        // runs filtering operation on the set of rows,
        // "dstcount + ksize - 1" rows on input,
        // "dstcount" rows on output,
        // each input and output row has "width" elements
        // the filtered rows are written into "dst" buffer.
        virtual void operator()(const uchar** src, uchar* dst, int dststep,
                                int dstcount, int width) = 0;
        // resets the filter state (may be needed for IIR filters)
        virtual void reset();

        int ksize; // the aperture size
        int anchor; // position of the anchor point,
                    // normally not used during the processing
    };


The class ``BaseColumnFilter`` is the base class for filtering data using single-column kernels. The filtering does not have to be a linear operation. In general, it could be written as following:

.. math::

    \texttt{dst} (x,y) = F( \texttt{src} [y](x), \; \texttt{src} [y+1](x), \; ..., \; \texttt{src} [y+ \texttt{ksize} -1](x)

where
:math:`F` is the filtering function, but, as it is represented as a class, it can produce any side effects, memorize previously processed data etc. The class only defines the interface and is not used directly. Instead, there are several functions in OpenCV (and you can add more) that return pointers to the derived classes that implement specific filtering operations. Those pointers are then passed to
:func:`FilterEngine` constructor. While the filtering operation interface uses ``uchar`` type, a particular implementation is not limited to 8-bit data.

See also:
:func:`BaseRowFilter`,:func:`BaseFilter`,:func:`FilterEngine`,
:func:`getColumnSumFilter`,:func:`getLinearColumnFilter`,:func:`getMorphologyColumnFilter`
.. index:: BaseFilter

.. _BaseFilter:

BaseFilter
----------
.. c:type:: BaseFilter

Base class for 2D image filters ::

    class BaseFilter
    {
    public:
        virtual ~BaseFilter();

        // To be overriden by the user.
        //
        // runs filtering operation on the set of rows,
        // "dstcount + ksize.height - 1" rows on input,
        // "dstcount" rows on output,
        // each input row has "(width + ksize.width-1)*cn" elements
        // each output row has "width*cn" elements.
        // the filtered rows are written into "dst" buffer.
        virtual void operator()(const uchar** src, uchar* dst, int dststep,
                                int dstcount, int width, int cn) = 0;
        // resets the filter state (may be needed for IIR filters)
        virtual void reset();
        Size ksize;
        Point anchor;
    };


The class ``BaseFilter`` is the base class for filtering data using 2D kernels. The filtering does not have to be a linear operation. In general, it could be written as following:

.. math::

    \begin{array}{l} \texttt{dst} (x,y) = F(  \texttt{src} [y](x), \; \texttt{src} [y](x+1), \; ..., \; \texttt{src} [y](x+ \texttt{ksize.width} -1),  \\ \texttt{src} [y+1](x), \; \texttt{src} [y+1](x+1), \; ..., \; \texttt{src} [y+1](x+ \texttt{ksize.width} -1),  \\ .........................................................................................  \\ \texttt{src} [y+ \texttt{ksize.height-1} ](x), \\ \texttt{src} [y+ \texttt{ksize.height-1} ](x+1), \\ ...
       \texttt{src} [y+ \texttt{ksize.height-1} ](x+ \texttt{ksize.width} -1))
       \end{array}

where
:math:`F` is the filtering function. The class only defines the interface and is not used directly. Instead, there are several functions in OpenCV (and you can add more) that return pointers to the derived classes that implement specific filtering operations. Those pointers are then passed to
:func:`FilterEngine` constructor. While the filtering operation interface uses ``uchar`` type, a particular implementation is not limited to 8-bit data.

See also:
:func:`BaseColumnFilter`,:func:`BaseRowFilter`,:func:`FilterEngine`,
:func:`getLinearFilter`,:func:`getMorphologyFilter`
.. index:: BaseRowFilter

.. _BaseRowFilter:

BaseRowFilter
-------------
.. c:type:: BaseRowFilter

Base class for filters with single-row kernels ::

    class BaseRowFilter
    {
    public:
        virtual ~BaseRowFilter();

        // To be overriden by the user.
        //
        // runs filtering operation on the single input row
        // of "width" element, each element is has "cn" channels.
        // the filtered row is written into "dst" buffer.
        virtual void operator()(const uchar* src, uchar* dst,
                                int width, int cn) = 0;
        int ksize, anchor;
    };


The class ``BaseRowFilter`` is the base class for filtering data using single-row kernels. The filtering does not have to be a linear operation. In general, it could be written as following:

.. math::

    \texttt{dst} (x,y) = F( \texttt{src} [y](x), \; \texttt{src} [y](x+1), \; ..., \; \texttt{src} [y](x+ \texttt{ksize.width} -1))

where
:math:`F` is the filtering function. The class only defines the interface and is not used directly. Instead, there are several functions in OpenCV (and you can add more) that return pointers to the derived classes that implement specific filtering operations. Those pointers are then passed to
:func:`FilterEngine` constructor. While the filtering operation interface uses ``uchar`` type, a particular implementation is not limited to 8-bit data.

See also:
:func:`BaseColumnFilter`,:func:`Filter`,:func:`FilterEngine`,
:func:`getLinearRowFilter`,:func:`getMorphologyRowFilter`,:func:`getRowSumFilter`
.. index:: FilterEngine

.. _FilterEngine:

FilterEngine
------------
.. c:type:: FilterEngine

Generic image filtering class ::

    class FilterEngine
    {
    public:
        // empty constructor
        FilterEngine();
        // builds a 2D non-separable filter (!_filter2D.empty()) or
        // a separable filter (!_rowFilter.empty() && !_columnFilter.empty())
        // the input data type will be "srcType", the output data type will be "dstType",
        // the intermediate data type is "bufType".
        // _rowBorderType and _columnBorderType determine how the image
        // will be extrapolated beyond the image boundaries.
        // _borderValue is only used when _rowBorderType and/or _columnBorderType
        // == BORDER_CONSTANT
        FilterEngine(const Ptr<BaseFilter>& _filter2D,
                     const Ptr<BaseRowFilter>& _rowFilter,
                     const Ptr<BaseColumnFilter>& _columnFilter,
                     int srcType, int dstType, int bufType,
                     int _rowBorderType=BORDER_REPLICATE,
                     int _columnBorderType=-1, // use _rowBorderType by default
                     const Scalar& _borderValue=Scalar());
        virtual ~FilterEngine();
        // separate function for the engine initialization
        void init(const Ptr<BaseFilter>& _filter2D,
                  const Ptr<BaseRowFilter>& _rowFilter,
                  const Ptr<BaseColumnFilter>& _columnFilter,
                  int srcType, int dstType, int bufType,
                  int _rowBorderType=BORDER_REPLICATE, int _columnBorderType=-1,
                  const Scalar& _borderValue=Scalar());
        // starts filtering of the ROI in an image of size "wholeSize".
        // returns the starting y-position in the source image.
        virtual int start(Size wholeSize, Rect roi, int maxBufRows=-1);
        // alternative form of start that takes the image
        // itself instead of "wholeSize". Set isolated to true to pretend that
        // there are no real pixels outside of the ROI
        // (so that the pixels will be extrapolated using the specified border modes)
        virtual int start(const Mat& src, const Rect& srcRoi=Rect(0,0,-1,-1),
                          bool isolated=false, int maxBufRows=-1);
        // processes the next portion of the source image,
        // "srcCount" rows starting from "src" and
        // stores the results to "dst".
        // returns the number of produced rows
        virtual int proceed(const uchar* src, int srcStep, int srcCount,
                            uchar* dst, int dstStep);
        // higher-level function that processes the whole
        // ROI or the whole image with a single call
        virtual void apply( const Mat& src, Mat& dst,
                            const Rect& srcRoi=Rect(0,0,-1,-1),
                            Point dstOfs=Point(0,0),
                            bool isolated=false);
        bool isSeparable() const { return filter2D.empty(); }
        // how many rows from the input image are not yet processed
        int remainingInputRows() const;
        // how many output rows are not yet produced
        int remainingOutputRows() const;
        ...
        // the starting and the ending rows in the source image
        int startY, endY;

        // pointers to the filters
        Ptr<BaseFilter> filter2D;
        Ptr<BaseRowFilter> rowFilter;
        Ptr<BaseColumnFilter> columnFilter;
    };


The class ``FilterEngine`` can be used to apply an arbitrary filtering operation to an image.
It contains all the necessary intermediate buffers, it computes extrapolated values
of the "virtual" pixels outside of the image etc. Pointers to the initialized ``FilterEngine`` instances
are returned by various ``create*Filter`` functions, see below, and they are used inside high-level functions such as
:func:`filter2D`,:func:`erode`,:func:`dilate` etc, that is, the class is the workhorse in many of OpenCV filtering functions.

This class makes it easier (though, maybe not very easy yet) to combine filtering operations with other operations, such as color space conversions, thresholding, arithmetic operations, etc. By combining several operations together you can get much better performance because your data will stay in cache. For example, below is the implementation of Laplace operator for a floating-point images, which is a simplified implementation of
:func:`Laplacian` : ::

    void laplace_f(const Mat& src, Mat& dst)
    {
        CV_Assert( src.type() == CV_32F );
        dst.create(src.size(), src.type());

        // get the derivative and smooth kernels for d2I/dx2.
        // for d2I/dy2 we could use the same kernels, just swapped
        Mat kd, ks;
        getSobelKernels( kd, ks, 2, 0, ksize, false, ktype );

        // let's process 10 source rows at once
        int DELTA = std::min(10, src.rows);
        Ptr<FilterEngine> Fxx = createSeparableLinearFilter(src.type(),
            dst.type(), kd, ks, Point(-1,-1), 0, borderType, borderType, Scalar() );
        Ptr<FilterEngine> Fyy = createSeparableLinearFilter(src.type(),
            dst.type(), ks, kd, Point(-1,-1), 0, borderType, borderType, Scalar() );

        int y = Fxx->start(src), dsty = 0, dy = 0;
        Fyy->start(src);
        const uchar* sptr = src.data + y*src.step;

        // allocate the buffers for the spatial image derivatives;
        // the buffers need to have more than DELTA rows, because at the
        // last iteration the output may take max(kd.rows-1,ks.rows-1)
        // rows more than the input.
        Mat Ixx( DELTA + kd.rows - 1, src.cols, dst.type() );
        Mat Iyy( DELTA + kd.rows - 1, src.cols, dst.type() );

        // inside the loop we always pass DELTA rows to the filter
        // (note that the "proceed" method takes care of possibe overflow, since
        // it was given the actual image height in the "start" method)
        // on output we can get:
        //  * < DELTA rows (the initial buffer accumulation stage)
        //  * = DELTA rows (settled state in the middle)
        //  * > DELTA rows (then the input image is over, but we generate
        //                  "virtual" rows using the border mode and filter them)
        // this variable number of output rows is dy.
        // dsty is the current output row.
        // sptr is the pointer to the first input row in the portion to process
        for( ; dsty < dst.rows; sptr += DELTA*src.step, dsty += dy )
        {
            Fxx->proceed( sptr, (int)src.step, DELTA, Ixx.data, (int)Ixx.step );
            dy = Fyy->proceed( sptr, (int)src.step, DELTA, d2y.data, (int)Iyy.step );
            if( dy > 0 )
            {
                Mat dstripe = dst.rowRange(dsty, dsty + dy);
                add(Ixx.rowRange(0, dy), Iyy.rowRange(0, dy), dstripe);
            }
        }
    }


If you do not need that much control of the filtering process, you can simply use the ``FilterEngine::apply`` method. Here is how the method is actually implemented: ::

    void FilterEngine::apply(const Mat& src, Mat& dst,
        const Rect& srcRoi, Point dstOfs, bool isolated)
    {
        // check matrix types
        CV_Assert( src.type() == srcType && dst.type() == dstType );

        // handle the "whole image" case
        Rect _srcRoi = srcRoi;
        if( _srcRoi == Rect(0,0,-1,-1) )
            _srcRoi = Rect(0,0,src.cols,src.rows);

        // check if the destination ROI is inside the dst.
        // and FilterEngine::start will check if the source ROI is inside src.
        CV_Assert( dstOfs.x >= 0 && dstOfs.y >= 0 &&
            dstOfs.x + _srcRoi.width <= dst.cols &&
            dstOfs.y + _srcRoi.height <= dst.rows );

        // start filtering
        int y = start(src, _srcRoi, isolated);

        // process the whole ROI. Note that "endY - startY" is the total number
        // of the source rows to process
        // (including the possible rows outside of srcRoi but inside the source image)
        proceed( src.data + y*src.step,
                 (int)src.step, endY - startY,
                 dst.data + dstOfs.y*dst.step +
                 dstOfs.x*dst.elemSize(), (int)dst.step );
    }


Unlike the earlier versions of OpenCV, now the filtering operations fully support the notion of image ROI, that is, pixels outside of the ROI but inside the image can be used in the filtering operations. For example, you can take a ROI of a single pixel and filter it - that will be a filter response at that particular pixel (however, it's possible to emulate the old behavior by passing ``isolated=false`` to ``FilterEngine::start`` or ``FilterEngine::apply`` ). You can pass the ROI explicitly to ``FilterEngine::apply`` , or construct a new matrix headers: ::

    // compute dI/dx derivative at src(x,y)

    // method 1:
    // form a matrix header for a single value
    float val1 = 0;
    Mat dst1(1,1,CV_32F,&val1);

    Ptr<FilterEngine> Fx = createDerivFilter(CV_32F, CV_32F,
                            1, 0, 3, BORDER_REFLECT_101);
    Fx->apply(src, Rect(x,y,1,1), Point(), dst1);

    // method 2:
    // form a matrix header for a single value
    float val2 = 0;
    Mat dst2(1,1,CV_32F,&val2);

    Mat pix_roi(src, Rect(x,y,1,1));
    Sobel(pix_roi, dst2, dst2.type(), 1, 0, 3, 1, 0, BORDER_REFLECT_101);

    printf("method1 =


Note on the data types. As it was mentioned in
:func:`BaseFilter` description, the specific filters can process data of any type, despite that ``Base*Filter::operator()`` only takes ``uchar`` pointers and no information about the actual types. To make it all work, the following rules are used:

*
    in case of separable filtering ``FilterEngine::rowFilter``     applied first. It transforms the input image data (of type ``srcType``     ) to the intermediate results stored in the internal buffers (of type ``bufType``     ). Then these intermediate results are processed
    *as single-channel data*
    with ``FilterEngine::columnFilter``     and stored in the output image (of type ``dstType``     ). Thus, the input type for ``rowFilter``     is ``srcType``     and the output type is ``bufType``     ; the input type for ``columnFilter``     is ``CV_MAT_DEPTH(bufType)``     and the output type is ``CV_MAT_DEPTH(dstType)``     .

*
    in case of non-separable filtering ``bufType``     must be the same as ``srcType``     . The source data is copied to the temporary buffer if needed and then just passed to ``FilterEngine::filter2D``     . That is, the input type for ``filter2D``     is ``srcType``     (= ``bufType``     ) and the output type is ``dstType``     .

See also:
:func:`BaseColumnFilter`,:func:`BaseFilter`,:func:`BaseRowFilter`,:func:`createBoxFilter`,:func:`createDerivFilter`,:func:`createGaussianFilter`,:func:`createLinearFilter`,:func:`createMorphologyFilter`,:func:`createSeparableLinearFilter`
.. index:: bilateralFilter

bilateralFilter
-------------------
.. c:function:: void bilateralFilter( const Mat\& src, Mat\& dst, int d,                      double sigmaColor, double sigmaSpace,                      int borderType=BORDER_DEFAULT )

    Applies bilateral filter to the image

    :param src: The source 8-bit or floating-point, 1-channel or 3-channel image

    :param dst: The destination image; will have the same size and the same type as  ``src``
    
    :param d: The diameter of each pixel neighborhood, that is used during filtering. If it is non-positive, it's computed from  ``sigmaSpace``
    
    :param sigmaColor: Filter sigma in the color space. Larger value of the parameter means that farther colors within the pixel neighborhood (see  ``sigmaSpace`` ) will be mixed together, resulting in larger areas of semi-equal color

    :param sigmaSpace: Filter sigma in the coordinate space. Larger value of the parameter means that farther pixels will influence each other (as long as their colors are close enough; see  ``sigmaColor`` ). Then  ``d>0`` , it specifies the neighborhood size regardless of  ``sigmaSpace`` , otherwise  ``d``  is proportional to  ``sigmaSpace``

The function applies bilateral filtering to the input image, as described in
http://www.dai.ed.ac.uk/CVonline/LOCAL\_COPIES/MANDUCHI1/Bilateral\_Filtering.html

.. index:: blur

blur
--------
.. c:function:: void blur( const Mat\& src, Mat\& dst,           Size ksize, Point anchor=Point(-1,-1),           int borderType=BORDER_DEFAULT )

    Smoothes image using normalized box filter

    :param src: The source image

    :param dst: The destination image; will have the same size and the same type as  ``src``
    
    :param ksize: The smoothing kernel size

    :param anchor: The anchor point. The default value  ``Point(-1,-1)``  means that the anchor is at the kernel center

    :param borderType: The border mode used to extrapolate pixels outside of the image

The function smoothes the image using the kernel:

.. math::

    \texttt{K} =  \frac{1}{\texttt{ksize.width*ksize.height}} \begin{bmatrix} 1 & 1 & 1 &  \cdots & 1 & 1  \\ 1 & 1 & 1 &  \cdots & 1 & 1  \\ \hdotsfor{6} \\ 1 & 1 & 1 &  \cdots & 1 & 1  \\ \end{bmatrix}

The call ``blur(src, dst, ksize, anchor, borderType)`` is equivalent to ``boxFilter(src, dst, src.type(), anchor, true, borderType)`` .

See also:
:func:`boxFilter`,:func:`bilateralFilter`,:func:`GaussianBlur`,:func:`medianBlur` .

.. index:: borderInterpolate

borderInterpolate
---------------------
.. c:function:: int borderInterpolate( int p, int len, int borderType )

    Computes source location of extrapolated pixel

    :param p: 0-based coordinate of the extrapolated pixel along one of the axes, likely <0 or >= ``len``
    
    :param len: length of the array along the corresponding axis

    :param borderType: the border type, one of the  ``BORDER_*`` , except for  ``BORDER_TRANSPARENT``  and  ``BORDER_ISOLATED`` . When  ``borderType==BORDER_CONSTANT``  the function always returns -1, regardless of  ``p``  and  ``len``

The function computes and returns the coordinate of the donor pixel, corresponding to the specified extrapolated pixel when using the specified extrapolation border mode. For example, if we use ``BORDER_WRAP`` mode in the horizontal direction, ``BORDER_REFLECT_101`` in the vertical direction and want to compute value of the "virtual" pixel ``Point(-5, 100)`` in a floating-point image ``img`` , it will be ::

    float val = img.at<float>(borderInterpolate(100, img.rows, BORDER_REFLECT_101),
                              borderInterpolate(-5, img.cols, BORDER_WRAP));


Normally, the function is not called directly; it is used inside
:func:`FilterEngine` and
:func:`copyMakeBorder` to compute tables for quick extrapolation.

See also:
:func:`FilterEngine`,:func:`copyMakeBorder`

.. index:: boxFilter

boxFilter
-------------
.. c:function:: void boxFilter( const Mat\& src, Mat\& dst, int ddepth,                Size ksize, Point anchor=Point(-1,-1),                bool normalize=true,                int borderType=BORDER_DEFAULT )

    Smoothes image using box filter

    :param src: The source image

    :param dst: The destination image; will have the same size and the same type as  ``src``
    
    :param ksize: The smoothing kernel size

    :param anchor: The anchor point. The default value  ``Point(-1,-1)``  means that the anchor is at the kernel center

    :param normalize: Indicates, whether the kernel is normalized by its area or not

    :param borderType: The border mode used to extrapolate pixels outside of the image

The function smoothes the image using the kernel:

.. math::

    \texttt{K} =  \alpha \begin{bmatrix} 1 & 1 & 1 &  \cdots & 1 & 1  \\ 1 & 1 & 1 &  \cdots & 1 & 1  \\ \hdotsfor{6} \\ 1 & 1 & 1 &  \cdots & 1 & 1 \end{bmatrix}

where

.. math::

    \alpha = \fork{\frac{1}{\texttt{ksize.width*ksize.height}}}{when \texttt{normalize=true}}{1}{otherwise}

Unnormalized box filter is useful for computing various integral characteristics over each pixel neighborhood, such as covariation matrices of image derivatives (used in dense optical flow algorithms,
etc.). If you need to compute pixel sums over variable-size windows, use
:func:`integral` .

See also:
:func:`boxFilter`,:func:`bilateralFilter`,:func:`GaussianBlur`,:func:`medianBlur`,:func:`integral` .

.. index:: buildPyramid

buildPyramid
----------------
.. c:function:: void buildPyramid( const Mat\& src, vector<Mat>\& dst, int maxlevel )

    Constructs Gaussian pyramid for an image

    :param src: The source image; check  :func:`pyrDown`  for the list of supported types

    :param dst: The destination vector of  ``maxlevel+1``  images of the same type as  ``src`` ; ``dst[0]``  will be the same as  ``src`` ,  ``dst[1]``  is the next pyramid layer,
        a smoothed and down-sized  ``src``  etc.

    :param maxlevel: The 0-based index of the last (i.e. the smallest) pyramid layer; it must be non-negative

The function constructs a vector of images and builds the gaussian pyramid by recursively applying
:func:`pyrDown` to the previously built pyramid layers, starting from ``dst[0]==src`` .

.. index:: copyMakeBorder

copyMakeBorder
------------------
.. c:function:: void copyMakeBorder( const Mat\& src, Mat\& dst,                    int top, int bottom, int left, int right,                    int borderType, const Scalar\& value=Scalar() )

    Forms a border around the image

    :param src: The source image

    :param dst: The destination image; will have the same type as  ``src``  and the size  ``Size(src.cols+left+right, src.rows+top+bottom)``
    
    :param top, bottom, left, right: Specify how much pixels in each direction from the source image rectangle one needs to extrapolate, e.g.  ``top=1, bottom=1, left=1, right=1``  mean that 1 pixel-wide border needs to be built

    :param borderType: The border type; see  :func:`borderInterpolate`
    
    :param value: The border value if  ``borderType==BORDER_CONSTANT``
    
The function copies the source image into the middle of the destination image. The areas to the left, to the right, above and below the copied source image will be filled with extrapolated pixels. This is not what
:func:`FilterEngine` or based on it filtering functions do (they extrapolate pixels on-fly), but what other more complex functions, including your own, may do to simplify image boundary handling.

The function supports the mode when ``src`` is already in the middle of ``dst`` . In this case the function does not copy ``src`` itself, but simply constructs the border, e.g.: ::

    // let border be the same in all directions
    int border=2;
    // constructs a larger image to fit both the image and the border
    Mat gray_buf(rgb.rows + border*2, rgb.cols + border*2, rgb.depth());
    // select the middle part of it w/o copying data
    Mat gray(gray_canvas, Rect(border, border, rgb.cols, rgb.rows));
    // convert image from RGB to grayscale
    cvtColor(rgb, gray, CV_RGB2GRAY);
    // form a border in-place
    copyMakeBorder(gray, gray_buf, border, border,
                   border, border, BORDER_REPLICATE);
    // now do some custom filtering ...
    ...


See also:
:func:`borderInterpolate`
.. index:: createBoxFilter

createBoxFilter
-------------------
.. c:function:: Ptr<FilterEngine> createBoxFilter( int srcType, int dstType,                                 Size ksize, Point anchor=Point(-1,-1),                                 bool normalize=true,                                 int borderType=BORDER_DEFAULT)

.. c:function:: Ptr<BaseRowFilter> getRowSumFilter(int srcType, int sumType,                                   int ksize, int anchor=-1)

.. c:function:: Ptr<BaseColumnFilter> getColumnSumFilter(int sumType, int dstType,                                   int ksize, int anchor=-1, double scale=1)

    Returns box filter engine

    :param srcType: The source image type

    :param sumType: The intermediate horizontal sum type; must have as many channels as  ``srcType``
    
    :param dstType: The destination image type; must have as many channels as  ``srcType``
    
    :param ksize: The aperture size

    :param anchor: The anchor position with the kernel; negative values mean that the anchor is at the kernel center

    :param normalize: Whether the sums are normalized or not; see  :func:`boxFilter`
    
    :param scale: Another way to specify normalization in lower-level  ``getColumnSumFilter``
    
    :param borderType: Which border type to use; see  :func:`borderInterpolate`

The function is a convenience function that retrieves horizontal sum primitive filter with
:func:`getRowSumFilter` , vertical sum filter with
:func:`getColumnSumFilter` , constructs new
:func:`FilterEngine` and passes both of the primitive filters there. The constructed filter engine can be used for image filtering with normalized or unnormalized box filter.

The function itself is used by
:func:`blur` and
:func:`boxFilter` .

See also:
:func:`FilterEngine`,:func:`blur`,:func:`boxFilter` .

.. index:: createDerivFilter

createDerivFilter
---------------------
.. c:function:: Ptr<FilterEngine> createDerivFilter( int srcType, int dstType,                                     int dx, int dy, int ksize,                                     int borderType=BORDER_DEFAULT )

    Returns engine for computing image derivatives

    :param srcType: The source image type

    :param dstType: The destination image type; must have as many channels as  ``srcType``
    
    :param dx: The derivative order in respect with x

    :param dy: The derivative order in respect with y

    :param ksize: The aperture size; see  :func:`getDerivKernels`
    
    :param borderType: Which border type to use; see  :func:`borderInterpolate`

The function :func:`createDerivFilter` is a small convenience function that retrieves linear filter coefficients for computing image derivatives using
:func:`getDerivKernels` and then creates a separable linear filter with
:func:`createSeparableLinearFilter` . The function is used by
:func:`Sobel` and
:func:`Scharr` .

See also:
:func:`createSeparableLinearFilter`,:func:`getDerivKernels`,:func:`Scharr`,:func:`Sobel` .

.. index:: createGaussianFilter

createGaussianFilter
------------------------
.. c:function:: Ptr<FilterEngine> createGaussianFilter( int type, Size ksize,                                   double sigmaX, double sigmaY=0,                                   int borderType=BORDER_DEFAULT)

    Returns engine for smoothing images with a Gaussian filter

    :param type: The source and the destination image type

    :param ksize: The aperture size; see  :func:`getGaussianKernel`
    
    :param sigmaX: The Gaussian sigma in the horizontal direction; see  :func:`getGaussianKernel`
    
    :param sigmaY: The Gaussian sigma in the vertical direction; if 0, then  :math:`\texttt{sigmaY}\leftarrow\texttt{sigmaX}`
    
    :param borderType: Which border type to use; see  :func:`borderInterpolate`

The function :func:`createGaussianFilter` computes Gaussian kernel coefficients and then returns separable linear filter for that kernel. The function is used by
:func:`GaussianBlur` . Note that while the function takes just one data type, both for input and output, you can pass by this limitation by calling
:func:`getGaussianKernel` and then
:func:`createSeparableFilter` directly.

See also:
:func:`createSeparableLinearFilter`,:func:`getGaussianKernel`,:func:`GaussianBlur` .

.. index:: createLinearFilter

createLinearFilter
----------------------
.. c:function:: Ptr<FilterEngine> createLinearFilter(int srcType, int dstType,               const Mat\& kernel, Point _anchor=Point(-1,-1),               double delta=0, int rowBorderType=BORDER_DEFAULT,               int columnBorderType=-1, const Scalar\& borderValue=Scalar())

.. c:function:: Ptr<BaseFilter> getLinearFilter(int srcType, int dstType,                               const Mat\& kernel,                               Point anchor=Point(-1,-1),                               double delta=0, int bits=0)

    Creates non-separable linear filter engine

    :param srcType: The source image type

    :param dstType: The destination image type; must have as many channels as  ``srcType``
    
    :param kernel: The 2D array of filter coefficients

    :param anchor: The anchor point within the kernel; special value  ``Point(-1,-1)``  means that the anchor is at the kernel center

    :param delta: The value added to the filtered results before storing them

    :param bits: When the kernel is an integer matrix representing fixed-point filter coefficients,
                     the parameter specifies the number of the fractional bits

    :param rowBorderType, columnBorderType: The pixel extrapolation methods in the horizontal and the vertical directions; see  :func:`borderInterpolate`
    
    :param borderValue: Used in case of constant border

The function returns pointer to 2D linear filter for the specified kernel, the source array type and the destination array type. The function is a higher-level function that calls ``getLinearFilter`` and passes the retrieved 2D filter to
:func:`FilterEngine` constructor.

See also:
:func:`createSeparableLinearFilter`,:func:`FilterEngine`,:func:`filter2D`
.. index:: createMorphologyFilter

createMorphologyFilter
--------------------------
.. c:function:: Ptr<FilterEngine> createMorphologyFilter(int op, int type,    const Mat\& element, Point anchor=Point(-1,-1),    int rowBorderType=BORDER_CONSTANT,    int columnBorderType=-1,    const Scalar\& borderValue=morphologyDefaultBorderValue())

.. c:function:: Ptr<BaseFilter> getMorphologyFilter(int op, int type, const Mat\& element,                                    Point anchor=Point(-1,-1))

.. c:function:: Ptr<BaseRowFilter> getMorphologyRowFilter(int op, int type,                                          int esize, int anchor=-1)

.. c:function:: Ptr<BaseColumnFilter> getMorphologyColumnFilter(int op, int type,                                                int esize, int anchor=-1)

.. c:function:: static inline Scalar morphologyDefaultBorderValue(){ return Scalar::all(DBL_MAX) }

    Creates engine for non-separable morphological operations

    :param op: The morphology operation id,  ``MORPH_ERODE``  or  ``MORPH_DILATE``
    
    :param type: The input/output image type

    :param element: The 2D 8-bit structuring element for the morphological operation. Non-zero elements indicate the pixels that belong to the element

    :param esize: The horizontal or vertical structuring element size for separable morphological operations

    :param anchor: The anchor position within the structuring element; negative values mean that the anchor is at the center

    :param rowBorderType, columnBorderType: The pixel extrapolation methods in the horizontal and the vertical directions; see  :func:`borderInterpolate`
    
    :param borderValue: The border value in case of a constant border. The default value, \   ``morphologyDefaultBorderValue`` , has the special meaning. It is transformed  :math:`+\inf`  for the erosion and to  :math:`-\inf`  for the dilation, which means that the minimum (maximum) is effectively computed only over the pixels that are inside the image.

The functions construct primitive morphological filtering operations or a filter engine based on them. Normally it's enough to use
:func:`createMorphologyFilter` or even higher-level
:func:`erode`,:func:`dilate` or
:func:`morphologyEx` , Note, that
:func:`createMorphologyFilter` analyses the structuring element shape and builds a separable morphological filter engine when the structuring element is square.

See also:
:func:`erode`,:func:`dilate`,:func:`morphologyEx`,:func:`FilterEngine`
.. index:: createSeparableLinearFilter

createSeparableLinearFilter
-------------------------------
.. c:function:: Ptr<FilterEngine> createSeparableLinearFilter(int srcType, int dstType,                         const Mat\& rowKernel, const Mat\& columnKernel,                         Point anchor=Point(-1,-1), double delta=0,                         int rowBorderType=BORDER_DEFAULT,                         int columnBorderType=-1,                         const Scalar\& borderValue=Scalar())

.. c:function:: Ptr<BaseColumnFilter> getLinearColumnFilter(int bufType, int dstType,                         const Mat\& columnKernel, int anchor,                         int symmetryType, double delta=0,                         int bits=0)

.. c:function:: Ptr<BaseRowFilter> getLinearRowFilter(int srcType, int bufType,                         const Mat\& rowKernel, int anchor,                         int symmetryType)

    Creates engine for separable linear filter

    :param srcType: The source array type

    :param dstType: The destination image type; must have as many channels as  ``srcType``
    
    :param bufType: The inermediate buffer type; must have as many channels as  ``srcType``
    
    :param rowKernel: The coefficients for filtering each row

    :param columnKernel: The coefficients for filtering each column

    :param anchor: The anchor position within the kernel; negative values mean that anchor is positioned at the aperture center

    :param delta: The value added to the filtered results before storing them

    :param bits: When the kernel is an integer matrix representing fixed-point filter coefficients,
                     the parameter specifies the number of the fractional bits

    :param rowBorderType, columnBorderType: The pixel extrapolation methods in the horizontal and the vertical directions; see  :func:`borderInterpolate`
    
    :param borderValue: Used in case of a constant border

    :param symmetryType: The type of each of the row and column kernel; see  :func:`getKernelType` .

The functions construct primitive separable linear filtering operations or a filter engine based on them. Normally it's enough to use
:func:`createSeparableLinearFilter` or even higher-level
:func:`sepFilter2D` . The function
:func:`createMorphologyFilter` is smart enough to figure out the ``symmetryType`` for each of the two kernels, the intermediate ``bufType`` , and, if the filtering can be done in integer arithmetics, the number of ``bits`` to encode the filter coefficients. If it does not work for you, it's possible to call ``getLinearColumnFilter``,``getLinearRowFilter`` directly and then pass them to
:func:`FilterEngine` constructor.

See also:
:func:`sepFilter2D`,:func:`createLinearFilter`,:func:`FilterEngine`,:func:`getKernelType`
.. index:: dilate

dilate
----------
.. c:function:: void dilate( const Mat\& src, Mat\& dst, const Mat\& element,             Point anchor=Point(-1,-1), int iterations=1,             int borderType=BORDER_CONSTANT,             const Scalar\& borderValue=morphologyDefaultBorderValue() )

    Dilates an image by using a specific structuring element.

    :param src: The source image

    :param dst: The destination image. It will have the same size and the same type as  ``src``
    
    :param element: The structuring element used for dilation. If  ``element=Mat()`` , a  :math:`3\times 3`  rectangular structuring element is used

    :param anchor: Position of the anchor within the element. The default value  :math:`(-1, -1)`  means that the anchor is at the element center

    :param iterations: The number of times dilation is applied

    :param borderType: The pixel extrapolation method; see  :func:`borderInterpolate`
    
    :param borderValue: The border value in case of a constant border. The default value has a special meaning, see  :func:`createMorphologyFilter`
    
The function dilates the source image using the specified structuring element that determines the shape of a pixel neighborhood over which the maximum is taken:

.. math::

    \texttt{dst} (x,y) =  \max _{(x',y'):  \, \texttt{element} (x',y') \ne0 } \texttt{src} (x+x',y+y')

The function supports the in-place mode. Dilation can be applied several ( ``iterations`` ) times. In the case of multi-channel images each channel is processed independently.

See also:
:func:`erode`,:func:`morphologyEx`,:func:`createMorphologyFilter`
.. index:: erode

erode
---------
.. c:function:: void erode( const Mat\& src, Mat\& dst, const Mat\& element,            Point anchor=Point(-1,-1), int iterations=1,            int borderType=BORDER_CONSTANT,            const Scalar\& borderValue=morphologyDefaultBorderValue() )

    Erodes an image by using a specific structuring element.

    :param src: The source image

    :param dst: The destination image. It will have the same size and the same type as  ``src``
    
    :param element: The structuring element used for dilation. If  ``element=Mat()`` , a  :math:`3\times 3`  rectangular structuring element is used

    :param anchor: Position of the anchor within the element. The default value  :math:`(-1, -1)`  means that the anchor is at the element center

    :param iterations: The number of times erosion is applied

    :param borderType: The pixel extrapolation method; see  :func:`borderInterpolate`
    
    :param borderValue: The border value in case of a constant border. The default value has a special meaning, see  :func:`createMorphoogyFilter`
    
The function erodes the source image using the specified structuring element that determines the shape of a pixel neighborhood over which the minimum is taken:

.. math::

    \texttt{dst} (x,y) =  \min _{(x',y'):  \, \texttt{element} (x',y') \ne0 } \texttt{src} (x+x',y+y')

The function supports the in-place mode. Erosion can be applied several ( ``iterations`` ) times. In the case of multi-channel images each channel is processed independently.

See also:
:func:`dilate`,:func:`morphologyEx`,:func:`createMorphologyFilter`

.. index:: filter2D

filter2D
------------
.. c:function:: void filter2D( const Mat\& src, Mat\& dst, int ddepth,               const Mat\& kernel, Point anchor=Point(-1,-1),               double delta=0, int borderType=BORDER_DEFAULT )

    Convolves an image with the kernel

    :param src: The source image

    :param dst: The destination image. It will have the same size and the same number of channels as  ``src``
    
    :param ddepth: The desired depth of the destination image. If it is negative, it will be the same as  ``src.depth()``
    
    :param kernel: Convolution kernel (or rather a correlation kernel), a single-channel floating point matrix. If you want to apply different kernels to different channels, split the image into separate color planes using  :func:`split`  and process them individually

    :param anchor: The anchor of the kernel that indicates the relative position of a filtered point within the kernel. The anchor should lie within the kernel. The special default value (-1,-1) means that the anchor is at the kernel center

    :param delta: The optional value added to the filtered pixels before storing them in  ``dst``
    
    :param borderType: The pixel extrapolation method; see  :func:`borderInterpolate`

The function applies an arbitrary linear filter to the image. In-place operation is supported. When the aperture is partially outside the image, the function interpolates outlier pixel values according to the specified border mode.

The function does actually computes correlation, not the convolution:

.. math::

    \texttt{dst} (x,y) =  \sum _{ \stackrel{0\leq x' < \texttt{kernel.cols},}{0\leq y' < \texttt{kernel.rows}} }  \texttt{kernel} (x',y')* \texttt{src} (x+x'- \texttt{anchor.x} ,y+y'- \texttt{anchor.y} )

That is, the kernel is not mirrored around the anchor point. If you need a real convolution, flip the kernel using
:func:`flip` and set the new anchor to ``(kernel.cols - anchor.x - 1, kernel.rows - anchor.y - 1)`` .

The function uses
-based algorithm in case of sufficiently large kernels (~
:math:`11\times11` ) and the direct algorithm (that uses the engine retrieved by
:func:`createLinearFilter` ) for small kernels.

See also:
:func:`sepFilter2D`,:func:`createLinearFilter`,:func:`dft`,:func:`matchTemplate`

.. index:: GaussianBlur

GaussianBlur
----------------
.. c:function:: void GaussianBlur( const Mat\& src, Mat\& dst, Size ksize,                   double sigmaX, double sigmaY=0,                   int borderType=BORDER_DEFAULT )

    Smoothes image using a Gaussian filter

    :param src: The source image

    :param dst: The destination image; will have the same size and the same type as  ``src``
    
    :param ksize: The Gaussian kernel size;  ``ksize.width``  and  ``ksize.height``  can differ, but they both must be positive and odd. Or, they can be zero's, then they are computed from  ``sigma*``
    
    :param sigmaX, sigmaY: The Gaussian kernel standard deviations in X and Y direction. If  ``sigmaY``  is zero, it is set to be equal to  ``sigmaX`` . If they are both zeros, they are computed from  ``ksize.width``  and  ``ksize.height`` , respectively, see  :func:`getGaussianKernel` . To fully control the result regardless of possible future modification of all this semantics, it is recommended to specify all of  ``ksize`` ,  ``sigmaX``  and  ``sigmaY``
    
    :param borderType: The pixel extrapolation method; see  :func:`borderInterpolate`

The function convolves the source image with the specified Gaussian kernel. In-place filtering is supported.

See also:
:func:`sepFilter2D`,:func:`filter2D`,:func:`blur`,:func:`boxFilter`,:func:`bilateralFilter`,:func:`medianBlur`
.. index:: getDerivKernels

getDerivKernels
-------------------
.. c:function:: void getDerivKernels( Mat\& kx, Mat\& ky, int dx, int dy, int ksize,                      bool normalize=false, int ktype=CV_32F )

    Returns filter coefficients for computing spatial image derivatives

    :param kx: The output matrix of row filter coefficients; will have type  ``ktype``
    
    :param ky: The output matrix of column filter coefficients; will have type  ``ktype``
    
    :param dx: The derivative order in respect with x

    :param dy: The derivative order in respect with y

    :param ksize: The aperture size. It can be  ``CV_SCHARR`` , 1, 3, 5 or 7

    :param normalize: Indicates, whether to normalize (scale down) the filter coefficients or not. In theory the coefficients should have the denominator  :math:`=2^{ksize*2-dx-dy-2}` . If you are going to filter floating-point images, you will likely want to use the normalized kernels. But if you compute derivatives of a 8-bit image, store the results in 16-bit image and wish to preserve all the fractional bits, you may want to set  ``normalize=false`` .

    :param ktype: The type of filter coefficients. It can be  ``CV_32f``  or  ``CV_64F``

The function computes and returns the filter coefficients for spatial image derivatives. When ``ksize=CV_SCHARR`` , the Scharr
:math:`3 \times 3` kernels are generated, see
:func:`Scharr` . Otherwise, Sobel kernels are generated, see
:func:`Sobel` . The filters are normally passed to
:func:`sepFilter2D` or to
:func:`createSeparableLinearFilter` .

.. index:: getGaussianKernel

getGaussianKernel
---------------------
.. c:function:: Mat getGaussianKernel( int ksize, double sigma, int ktype=CV_64F )

    Returns Gaussian filter coefficients

    :param ksize: The aperture size. It should be odd ( :math:`\texttt{ksize} \mod 2 = 1` ) and positive.

    :param sigma: The Gaussian standard deviation. If it is non-positive, it is computed from  ``ksize``  as  \ ``sigma = 0.3*(ksize/2 - 1) + 0.8``
    :param ktype: The type of filter coefficients. It can be  ``CV_32f``  or  ``CV_64F``

The function computes and returns the
:math:`\texttt{ksize} \times 1` matrix of Gaussian filter coefficients:

.. math::

    G_i= \alpha *e^{-(i-( \texttt{ksize} -1)/2)^2/(2* \texttt{sigma} )^2},

where
:math:`i=0..\texttt{ksize}-1` and
:math:`\alpha` is the scale factor chosen so that
:math:`\sum_i G_i=1` Two of such generated kernels can be passed to
:func:`sepFilter2D` or to
:func:`createSeparableLinearFilter` that will automatically detect that these are smoothing kernels and handle them accordingly. Also you may use the higher-level
:func:`GaussianBlur` .

See also:
:func:`sepFilter2D`,:func:`createSeparableLinearFilter`,:func:`getDerivKernels`,:func:`getStructuringElement`,:func:`GaussianBlur` .

.. index:: getKernelType

getKernelType
-----------------
.. c:function:: int getKernelType(const Mat\& kernel, Point anchor)

    Returns the kernel type

    :param kernel: 1D array of the kernel coefficients to analyze

    :param anchor: The anchor position within the kernel

The function analyzes the kernel coefficients and returns the corresponding kernel type:

    * **KERNEL_GENERAL** Generic kernel - when there is no any type of symmetry or other properties

    * **KERNEL_SYMMETRICAL** The kernel is symmetrical:  :math:`\texttt{kernel}_i == \texttt{kernel}_{ksize-i-1}`  and the anchor is at the center

    * **KERNEL_ASYMMETRICAL** The kernel is asymmetrical:  :math:`\texttt{kernel}_i == -\texttt{kernel}_{ksize-i-1}`  and the anchor is at the center

    * **KERNEL_SMOOTH** All the kernel elements are non-negative and sum to 1. E.g. the Gaussian kernel is both smooth kernel and symmetrical, so the function will return  ``KERNEL_SMOOTH | KERNEL_SYMMETRICAL``
    * **KERNEL_INTEGER** Al the kernel coefficients are integer numbers. This flag can be combined with  ``KERNEL_SYMMETRICAL``  or  ``KERNEL_ASYMMETRICAL``
    
.. index:: getStructuringElement

getStructuringElement
-------------------------
.. c:function:: Mat getStructuringElement(int shape, Size esize, Point anchor=Point(-1,-1))

    Returns the structuring element of the specified size and shape for morphological operations

    :param shape: The element shape, one of:

      * ``MORPH_RECT``         - rectangular structuring element

        .. math::

            E_{ij}=1

      * ``MORPH_ELLIPSE``         - elliptic structuring element, i.e. a filled ellipse inscribed into the rectangle ``Rect(0, 0, esize.width, 0.esize.height)``
    
      * ``MORPH_CROSS``         - cross-shaped structuring element:

        .. math::

            E_{ij} =  \fork{1}{if i=\texttt{anchor.y} or j=\texttt{anchor.x}}{0}{otherwise}

    :param esize: Size of the structuring element

    :param anchor: The anchor position within the element. The default value  :math:`(-1, -1)`  means that the anchor is at the center. Note that only the cross-shaped element's shape depends on the anchor position; in other cases the anchor just regulates by how much the result of the morphological operation is shifted

The function constructs and returns the structuring element that can be then passed to
:func:`createMorphologyFilter`,:func:`erode`,:func:`dilate` or
:func:`morphologyEx` . But also you can construct an arbitrary binary mask yourself and use it as the structuring element.

.. index:: medianBlur

medianBlur
--------------
.. c:function:: void medianBlur( const Mat\& src, Mat\& dst, int ksize )

    Smoothes image using median filter

    :param src: The source 1-, 3- or 4-channel image. When  ``ksize``  is 3 or 5, the image depth should be  ``CV_8U`` ,  ``CV_16U``  or  ``CV_32F`` . For larger aperture sizes it can only be  ``CV_8U``
    
    :param dst: The destination array; will have the same size and the same type as  ``src``
    
    :param ksize: The aperture linear size. It must be odd and more than 1, i.e. 3, 5, 7 ...

The function smoothes image using the median filter with
:math:`\texttt{ksize} \times \texttt{ksize}` aperture. Each channel of a multi-channel image is processed independently. In-place operation is supported.

See also:
:func:`bilateralFilter`,:func:`blur`,:func:`boxFilter`,:func:`GaussianBlur`
.. index:: morphologyEx

morphologyEx
----------------
.. c:function:: void morphologyEx( const Mat\& src, Mat\& dst,                    int op, const Mat\& element,                   Point anchor=Point(-1,-1), int iterations=1,                   int borderType=BORDER_CONSTANT,                   const Scalar\& borderValue=morphologyDefaultBorderValue() )

    Performs advanced morphological transformations

    :param src: Source image

    :param dst: Destination image. It will have the same size and the same type as  ``src``
    
    :param element: Structuring element

    :param op: Type of morphological operation, one of the following:

            * **MORPH_OPEN** opening

            * **MORPH_CLOSE** closing

            * **MORPH_GRADIENT** morphological gradient

            * **MORPH_TOPHAT** "top hat"

            * **MORPH_BLACKHAT** "black hat"

    :param iterations: Number of times erosion and dilation are applied

    :param borderType: The pixel extrapolation method; see  :func:`borderInterpolate`
    
    :param borderValue: The border value in case of a constant border. The default value has a special meaning, see  :func:`createMorphoogyFilter`

The function can perform advanced morphological transformations using erosion and dilation as basic operations.

Opening:

.. math::

    \texttt{dst} = \mathrm{open} ( \texttt{src} , \texttt{element} )= \mathrm{dilate} ( \mathrm{erode} ( \texttt{src} , \texttt{element} ))

Closing:

.. math::

    \texttt{dst} = \mathrm{close} ( \texttt{src} , \texttt{element} )= \mathrm{erode} ( \mathrm{dilate} ( \texttt{src} , \texttt{element} ))

Morphological gradient:

.. math::

    \texttt{dst} = \mathrm{morph\_grad} ( \texttt{src} , \texttt{element} )= \mathrm{dilate} ( \texttt{src} , \texttt{element} )- \mathrm{erode} ( \texttt{src} , \texttt{element} )

"Top hat":

.. math::

    \texttt{dst} = \mathrm{tophat} ( \texttt{src} , \texttt{element} )= \texttt{src} - \mathrm{open} ( \texttt{src} , \texttt{element} )

"Black hat":

.. math::

    \texttt{dst} = \mathrm{blackhat} ( \texttt{src} , \texttt{element} )= \mathrm{close} ( \texttt{src} , \texttt{element} )- \texttt{src}

Any of the operations can be done in-place.

See also:
:func:`dilate`,:func:`erode`,:func:`createMorphologyFilter`
.. index:: Laplacian

Laplacian
-------------
.. c:function:: void Laplacian( const Mat\& src, Mat\& dst, int ddepth,               int ksize=1, double scale=1, double delta=0,               int borderType=BORDER_DEFAULT )

    Calculates the Laplacian of an image

    :param src: Source image

    :param dst: Destination image; will have the same size and the same number of channels as  ``src``
    
    :param ddepth: The desired depth of the destination image

    :param ksize: The aperture size used to compute the second-derivative filters, see  :func:`getDerivKernels` . It must be positive and odd

    :param scale: The optional scale factor for the computed Laplacian values (by default, no scaling is applied, see  :func:`getDerivKernels` )

    :param delta: The optional delta value, added to the results prior to storing them in  ``dst``
    
    :param borderType: The pixel extrapolation method, see  :func:`borderInterpolate`

The function calculates the Laplacian of the source image by adding up the second x and y derivatives calculated using the Sobel operator:

.. math::

    \texttt{dst} =  \Delta \texttt{src} =  \frac{\partial^2 \texttt{src}}{\partial x^2} +  \frac{\partial^2 \texttt{src}}{\partial y^2}

This is done when ``ksize > 1`` . When ``ksize == 1`` , the Laplacian is computed by filtering the image with the following
:math:`3 \times 3` aperture:

.. math::

    \vecthreethree {0}{1}{0}{1}{-4}{1}{0}{1}{0}

See also:
:func:`Sobel`,:func:`Scharr`
.. index:: pyrDown

pyrDown
-----------
.. c:function:: void pyrDown( const Mat\& src, Mat\& dst, const Size\& dstsize=Size())

    Smoothes an image and downsamples it.

    :param src: The source image

    :param dst: The destination image. It will have the specified size and the same type as  ``src``
    
    :param dstsize: Size of the destination image. By default it is computed as  ``Size((src.cols+1)/2, (src.rows+1)/2)`` . But in any case the following conditions should be satisfied:

        .. math::

            \begin{array}{l}
            | \texttt{dstsize.width} *2-src.cols| \leq  2  \\ | \texttt{dstsize.height} *2-src.rows| \leq  2 \end{array}

The function performs the downsampling step of the Gaussian pyramid construction. First it convolves the source image with the kernel:

.. math::

    \frac{1}{16} \begin{bmatrix} 1 & 4 & 6 & 4 & 1  \\ 4 & 16 & 24 & 16 & 4  \\ 6 & 24 & 36 & 24 & 6  \\ 4 & 16 & 24 & 16 & 4  \\ 1 & 4 & 6 & 4 & 1 \end{bmatrix}

and then downsamples the image by rejecting even rows and columns.

.. index:: pyrUp

pyrUp
---------
.. c:function:: void pyrUp( const Mat\& src, Mat\& dst, const Size\& dstsize=Size())

    Upsamples an image and then smoothes it

    :param src: The source image

    :param dst: The destination image. It will have the specified size and the same type as  ``src``
    
    :param dstsize: Size of the destination image. By default it is computed as  ``Size(src.cols*2, (src.rows*2)`` . But in any case the following conditions should be satisfied:

        .. math::

            \begin{array}{l}
            | \texttt{dstsize.width} -src.cols*2| \leq  ( \texttt{dstsize.width}   \mod  2)  \\ | \texttt{dstsize.height} -src.rows*2| \leq  ( \texttt{dstsize.height}   \mod  2) \end{array}

The function performs the upsampling step of the Gaussian pyramid construction (it can actually be used to construct the Laplacian pyramid). First it upsamples the source image by injecting even zero rows and columns and then convolves the result with the same kernel as in
:func:`pyrDown` , multiplied by 4.

.. index:: sepFilter2D

sepFilter2D
---------------
.. c:function:: void sepFilter2D( const Mat\& src, Mat\& dst, int ddepth,                  const Mat\& rowKernel, const Mat\& columnKernel,                  Point anchor=Point(-1,-1),                  double delta=0, int borderType=BORDER_DEFAULT )

    Applies separable linear filter to an image

    :param src: The source image

    :param dst: The destination image; will have the same size and the same number of channels as  ``src``
    
    :param ddepth: The destination image depth

    :param rowKernel: The coefficients for filtering each row

    :param columnKernel: The coefficients for filtering each column

    :param anchor: The anchor position within the kernel; The default value  :math:`(-1, 1)`  means that the anchor is at the kernel center

    :param delta: The value added to the filtered results before storing them

    :param borderType: The pixel extrapolation method; see  :func:`borderInterpolate`

The function applies a separable linear filter to the image. That is, first, every row of ``src`` is filtered with 1D kernel ``rowKernel`` . Then, every column of the result is filtered with 1D kernel ``columnKernel`` and the final result shifted by ``delta`` is stored in ``dst`` .

See also:
:func:`createSeparableLinearFilter`,:func:`filter2D`,:func:`Sobel`,:func:`GaussianBlur`,:func:`boxFilter`,:func:`blur` .

.. index:: Sobel

Sobel
---------
.. c:function:: void Sobel( const Mat\& src, Mat\& dst, int ddepth,            int xorder, int yorder, int ksize=3,            double scale=1, double delta=0,            int borderType=BORDER_DEFAULT )

    Calculates the first, second, third or mixed image derivatives using an extended Sobel operator

    :param src: The source image

    :param dst: The destination image; will have the same size and the same number of channels as  ``src``
    
    :param ddepth: The destination image depth

    :param xorder: Order of the derivative x

    :param yorder: Order of the derivative y

    :param ksize: Size of the extended Sobel kernel, must be 1, 3, 5 or 7

    :param scale: The optional scale factor for the computed derivative values (by default, no scaling is applied, see  :func:`getDerivKernels` )

    :param delta: The optional delta value, added to the results prior to storing them in  ``dst``
    
    :param borderType: The pixel extrapolation method, see  :func:`borderInterpolate`

In all cases except 1, an
:math:`\texttt{ksize} \times
\texttt{ksize}` separable kernel will be used to calculate the
derivative. When
:math:`\texttt{ksize = 1}` , a
:math:`3 \times 1` or
:math:`1 \times 3` kernel will be used (i.e. no Gaussian smoothing is done). ``ksize = 1`` can only be used for the first or the second x- or y- derivatives.

There is also the special value ``ksize = CV_SCHARR`` (-1) that corresponds to a
:math:`3\times3` Scharr
filter that may give more accurate results than a
:math:`3\times3` Sobel. The Scharr
aperture is

.. math::

    \vecthreethree{-3}{0}{3}{-10}{0}{10}{-3}{0}{3}

for the x-derivative or transposed for the y-derivative.

The function calculates the image derivative by convolving the image with the appropriate kernel:

.. math::

    \texttt{dst} =  \frac{\partial^{xorder+yorder} \texttt{src}}{\partial x^{xorder} \partial y^{yorder}}

The Sobel operators combine Gaussian smoothing and differentiation,
so the result is more or less resistant to the noise. Most often,
the function is called with ( ``xorder`` = 1, ``yorder`` = 0, ``ksize`` = 3) or ( ``xorder`` = 0, ``yorder`` = 1, ``ksize`` = 3) to calculate the first x- or y- image
derivative. The first case corresponds to a kernel of:

.. math::

    \vecthreethree{-1}{0}{1}{-2}{0}{2}{-1}{0}{1}

and the second one corresponds to a kernel of:

.. math::

    \vecthreethree{-1}{-2}{-1}{0}{0}{0}{1}{2}{1}

See also:
:func:`Scharr`,:func:`Lapacian`,:func:`sepFilter2D`,:func:`filter2D`,:func:`GaussianBlur`
.. index:: Scharr

Scharr
----------
.. c:function:: void Scharr( const Mat\& src, Mat\& dst, int ddepth,            int xorder, int yorder,            double scale=1, double delta=0,            int borderType=BORDER_DEFAULT )

    Calculates the first x- or y- image derivative using Scharr operator

    :param src: The source image

    :param dst: The destination image; will have the same size and the same number of channels as  ``src``
    
    :param ddepth: The destination image depth

    :param xorder: Order of the derivative x

    :param yorder: Order of the derivative y

    :param scale: The optional scale factor for the computed derivative values (by default, no scaling is applied, see  :func:`getDerivKernels` )

    :param delta: The optional delta value, added to the results prior to storing them in  ``dst``
    
    :param borderType: The pixel extrapolation method, see  :func:`borderInterpolate`
    
The function computes the first x- or y- spatial image derivative using Scharr operator. The call

.. math::

    \texttt{Scharr(src, dst, ddepth, xorder, yorder, scale, delta, borderType)}

is equivalent to

.. math::

    \texttt{Sobel(src, dst, ddepth, xorder, yorder, CV\_SCHARR, scale, delta, borderType)} .

