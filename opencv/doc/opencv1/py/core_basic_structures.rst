Basic Structures
================

.. highlight:: python



.. index:: CvPoint

.. _CvPoint:

CvPoint
-------



.. class:: CvPoint



2D point with integer coordinates (usually zero-based).

2D point, represented as a tuple 
``(x, y)``
, where x and y are integers.

.. index:: CvPoint2D32f

.. _CvPoint2D32f:

CvPoint2D32f
------------



.. class:: CvPoint2D32f



2D point with floating-point coordinates

2D point, represented as a tuple 
``(x, y)``
, where x and y are floats.

.. index:: CvPoint3D32f

.. _CvPoint3D32f:

CvPoint3D32f
------------



.. class:: CvPoint3D32f



3D point with floating-point coordinates

3D point, represented as a tuple 
``(x, y, z)``
, where x, y and z are floats.

.. index:: CvPoint2D64f

.. _CvPoint2D64f:

CvPoint2D64f
------------



.. class:: CvPoint2D64f



2D point with double precision floating-point coordinates

2D point, represented as a tuple 
``(x, y)``
, where x and y are floats.

.. index:: CvPoint3D64f

.. _CvPoint3D64f:

CvPoint3D64f
------------



.. class:: CvPoint3D64f



3D point with double precision floating-point coordinates

3D point, represented as a tuple 
``(x, y, z)``
, where x, y and z are floats.

.. index:: CvSize

.. _CvSize:

CvSize
------



.. class:: CvSize



Pixel-accurate size of a rectangle.

Size of a rectangle, represented as a tuple 
``(width, height)``
, where width and height are integers.

.. index:: CvSize2D32f

.. _CvSize2D32f:

CvSize2D32f
-----------



.. class:: CvSize2D32f



Sub-pixel accurate size of a rectangle.

Size of a rectangle, represented as a tuple 
``(width, height)``
, where width and height are floats.

.. index:: CvRect

.. _CvRect:

CvRect
------



.. class:: CvRect



Offset (usually the top-left corner) and size of a rectangle.

Rectangle, represented as a tuple 
``(x, y, width, height)``
, where all are integers.

.. index:: CvScalar

.. _CvScalar:

CvScalar
--------



.. class:: CvScalar



A container for 1-,2-,3- or 4-tuples of doubles.

CvScalar is always represented as a 4-tuple.




.. doctest::


    
    >>> import cv
    >>> cv.Scalar(1, 2, 3, 4)
    (1.0, 2.0, 3.0, 4.0)
    >>> cv.ScalarAll(7)
    (7.0, 7.0, 7.0, 7.0)
    >>> cv.RealScalar(7)
    (7.0, 0.0, 0.0, 0.0)
    >>> cv.RGB(17, 110, 255)
    (255.0, 110.0, 17.0, 0.0)
    

..


.. index:: CvTermCriteria

.. _CvTermCriteria:

CvTermCriteria
--------------



.. class:: CvTermCriteria



Termination criteria for iterative algorithms.

Represented by a tuple 
``(type, max_iter, epsilon)``
.



    
    
    .. attribute:: type
    
    
    
        ``CV_TERMCRIT_ITER`` ,  ``CV_TERMCRIT_EPS``  or  ``CV_TERMCRIT_ITER | CV_TERMCRIT_EPS`` 
    
    
    
    .. attribute:: max_iter
    
    
    
        Maximum number of iterations 
    
    
    
    .. attribute:: epsilon
    
    
    
        Required accuracy 
    
    
    



::


    
    (cv.CV_TERMCRIT_ITER, 10, 0)                         # terminate after 10 iterations
    (cv.CV_TERMCRIT_EPS, 0, 0.01)                        # terminate when epsilon reaches 0.01
    (cv.CV_TERMCRIT_ITER | cv.CV_TERMCRIT_EPS, 10, 0.01) # terminate as soon as either condition is met
    

..


.. index:: CvMat

.. _CvMat:

CvMat
-----



.. class:: CvMat



A multi-channel 2D matrix.  Created by
:ref:`CreateMat`
,
:ref:`LoadImageM`
,
:ref:`CreateMatHeader`
,
:ref:`fromarray`
.



    
    
    .. attribute:: type
    
    
    
        A CvMat signature containing the type of elements and flags, int 
    
    
    
    .. attribute:: step
    
    
    
        Full row length in bytes, int 
    
    
    
    .. attribute:: rows
    
    
    
        Number of rows, int 
    
    
    
    .. attribute:: cols
    
    
    
        Number of columns, int 
    
    
    
    .. method:: tostring() -> str
    
    
    
        Returns the contents of the CvMat as a single string. 
    
    
    

.. index:: CvMatND

.. _CvMatND:

CvMatND
-------



.. class:: CvMatND



Multi-dimensional dense multi-channel array.



    
    
    .. attribute:: type
    
    
    
        A CvMatND signature combining the type of elements and flags, int 
    
    
    
    .. method:: tostring() -> str
    
    
    
        Returns the contents of the CvMatND as a single string. 
    
    
    

.. index:: IplImage

.. _IplImage:

IplImage
--------



.. class:: IplImage



The 
:ref:`IplImage`
object was inherited from the Intel Image Processing
Library, in which the format is native. OpenCV only supports a subset
of possible 
:ref:`IplImage`
formats.



    
    
    .. attribute:: nChannels
    
    
    
        Number of channels, int. 
    
    
    
    .. attribute:: width
    
    
    
        Image width in pixels 
    
    
    
    .. attribute:: height
    
    
    
        Image height in pixels 
    
    
    
    .. attribute:: depth
    
    
    
        Pixel depth in bits. The supported depths are: 
        
            
            .. attribute:: IPL_DEPTH_8U
            
            
            
                Unsigned 8-bit integer 
            
            
            .. attribute:: IPL_DEPTH_8S
            
            
            
                Signed 8-bit integer 
            
            
            .. attribute:: IPL_DEPTH_16U
            
            
            
                Unsigned 16-bit integer 
            
            
            .. attribute:: IPL_DEPTH_16S
            
            
            
                Signed 16-bit integer 
            
            
            .. attribute:: IPL_DEPTH_32S
            
            
            
                Signed 32-bit integer 
            
            
            .. attribute:: IPL_DEPTH_32F
            
            
            
                Single-precision floating point 
            
            
            .. attribute:: IPL_DEPTH_64F
            
            
            
                Double-precision floating point 
            
            
    
    
    
    .. attribute:: origin
    
    
    
        0 - top-left origin, 1 - bottom-left origin (Windows bitmap style) 
    
    
    
    .. method:: tostring() -> str
    
    
    
        Returns the contents of the CvMatND as a single string. 
    
    
    

.. index:: CvArr

.. _CvArr:

CvArr
-----



.. class:: CvArr



Arbitrary array

``CvArr``
is used 
*only*
as a function parameter to specify that the parameter can be:


    

* an :ref:`IplImage`
    

* a :ref:`CvMat`
    

* any other type that exports the `array interface <http://docs.scipy.org/doc/numpy/reference/arrays.interface.html>`_
    
    
