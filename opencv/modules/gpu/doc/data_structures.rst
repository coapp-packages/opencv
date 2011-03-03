Data Structures
===============

.. highlight:: cpp

.. index:: gpu::DevMem2D\_

.. _gpu::DevMem2D_:

gpu::DevMem2D\_
---------------
.. c:type:: gpu::DevMem2D\_

This is a simple lightweight class that encapsulate pitched memory on GPU. It is intended to pass to nvcc-compiled code, i.e. CUDA kernels. So it is used internally by OpenCV and by users writes own device code. Its members can be called both from host and from device code. ::

    template <typename T> struct DevMem2D_
    {
        int cols;
        int rows;
        T* data;
        size_t step;

        DevMem2D_() : cols(0), rows(0), data(0), step(0){};
        DevMem2D_(int rows, int cols, T *data, size_t step);

        template <typename U>
        explicit DevMem2D_(const DevMem2D_<U>& d);

        typedef T elem_type;
        enum { elem_size = sizeof(elem_type) };

        __CV_GPU_HOST_DEVICE__ size_t elemSize() const;

        /* returns pointer to the beggining of given image row */
        __CV_GPU_HOST_DEVICE__ T* ptr(int y = 0);
        __CV_GPU_HOST_DEVICE__ const T* ptr(int y = 0) const;
    };


.. index:: gpu::PtrStep\_

.. gpu::PtrStep\_:

gpu::PtrStep\_
--------------
.. c:type:: gpu::PtrStep\_

This is structure is similar to DevMem2D\_ but contains only pointer and row step. Width and height fields are excluded due to performance reasons. The structure is for internal use or for users who write own device code. ::

    template<typename T> struct PtrStep_
    {
            T* data;
            size_t step;

            PtrStep_();
            PtrStep_(const DevMem2D_<T>& mem);

            typedef T elem_type;
            enum { elem_size = sizeof(elem_type) };

            __CV_GPU_HOST_DEVICE__ size_t elemSize() const;
            __CV_GPU_HOST_DEVICE__ T* ptr(int y = 0);
            __CV_GPU_HOST_DEVICE__ const T* ptr(int y = 0) const;
    };


.. index:: gpu::PtrElemStrp\_

.. gpu::PtrElemStrp\_:

gpu::PtrElemStrp\_
------------------
.. c:type:: gpu::PtrElemStrp\_

This is structure is similar to DevMem2D_but contains only pointer and row step in elements. Width and height fields are excluded due to performance reasons. This class is can only be constructed if sizeof(T) is a multiple of 256. The structure is for internal use or for users who write own device code. ::

    template<typename T> struct PtrElemStep_ : public PtrStep_<T>
    {
            PtrElemStep_(const DevMem2D_<T>& mem);
            __CV_GPU_HOST_DEVICE__ T* ptr(int y = 0);
            __CV_GPU_HOST_DEVICE__ const T* ptr(int y = 0) const;
    };


.. index:: gpu::GpuMat

.. gpu::GpuMat:

gpu::GpuMat
-----------
.. c:type:: gpu::GpuMat

The base storage class for GPU memory with reference counting. Its interface is almost
:func:`Mat` interface with some limitations, so using it won't be a problem. The limitations are no arbitrary dimensions support (only 2D), no functions that returns references to its data (because references on GPU are not valid for CPU), no expression templates technique support. Because of last limitation please take care with overloaded matrix operators - they cause memory allocations. The GpuMat class is convertible to
and
so it can be passed to directly to kernel.

**Please note:**
In contrast with
:func:`Mat` , In most cases ``GpuMat::isContinuous() == false`` , i.e. rows are aligned to size depending on hardware. Also single row GpuMat is always a continuous matrix. ::

    class CV_EXPORTS GpuMat
    {
    public:
            //! default constructor
            GpuMat();

            GpuMat(int rows, int cols, int type);
            GpuMat(Size size, int type);

            .....

            //! builds GpuMat from Mat. Perfom blocking upload to device.
            explicit GpuMat (const Mat& m);

            //! returns lightweight DevMem2D_ structure for passing
            //to nvcc-compiled code. Contains size, data ptr and step.
            template <class T> operator DevMem2D_<T>() const;
            template <class T> operator PtrStep_<T>() const;

            //! pefroms blocking upload data to GpuMat.
            void upload(const cv::Mat& m);
            void upload(const CudaMem& m, Stream& stream);

            //! downloads data from device to host memory. Blocking calls.
            operator Mat() const;
            void download(cv::Mat& m) const;

            //! download async
            void download(CudaMem& m, Stream& stream) const;
    };


**Please note:**
Is it a bad practice to leave static or global GpuMat variables allocated, i.e. to rely on its destructor. That is because destruction order of such variables and CUDA context is undefined and GPU memory release function returns error if CUDA context has been destroyed before.

See also:
:func:`Mat`

.. index:: gpu::CudaMem

gpu::CudaMem
------------
.. c:type:: gpu::CudaMem

This is a class with reference counting that wraps special memory type allocation functions from CUDA. Its interface is also
:func:`Mat` -like but with additional memory type parameter:

* ``ALLOC_PAGE_LOCKED``     Set page locked memory type, used commonly for fast and asynchronous upload/download data from/to GPU.

* ``ALLOC_ZEROCOPY``     Specifies zero copy memory allocation, i.e. with possibility to map host memory to GPU address space if supported.

* ``ALLOC_WRITE_COMBINED``     Sets write combined buffer which is not cached by CPU. Such buffers are used to supply GPU with data when GPU only reads it. The advantage is better CPU cache utilization.

Please note that allocation size of such memory types is usually limited. For more details please see "CUDA 2.2 Pinned Memory APIs" document or "CUDA_C Programming Guide". ::

    class CV_EXPORTS CudaMem
    {
    public:
            enum  { ALLOC_PAGE_LOCKED = 1, ALLOC_ZEROCOPY = 2,
                     ALLOC_WRITE_COMBINED = 4 };

            CudaMem(Size size, int type, int alloc_type = ALLOC_PAGE_LOCKED);

            //! creates from cv::Mat with coping data
            explicit CudaMem(const Mat& m, int alloc_type = ALLOC_PAGE_LOCKED);

             ......

            void create(Size size, int type, int alloc_type = ALLOC_PAGE_LOCKED);

            //! returns matrix header with disabled ref. counting for CudaMem data.
            Mat createMatHeader() const;
            operator Mat() const;

            //! maps host memory into device address space
            GpuMat createGpuMatHeader() const;
            operator GpuMat() const;

            //if host memory can be mapperd to gpu address space;
            static bool canMapHostMemory();

            int alloc_type;
    };


.. index:: gpu::CudaMem::createMatHeader

gpu::CudaMem::createMatHeader
---------------------------------

.. cpp:function:: Mat CudaMem::createMatHeader() const

.. cpp:function:: CudaMem::operator Mat() const

    Creates header without reference counting to CudaMem data.

.. index:: gpu::CudaMem::createGpuMatHeader

gpu::CudaMem::createGpuMatHeader
------------------------------------
:func:`gpu::GpuMat` ``_``
.. c:function:: GpuMat CudaMem::createGpuMatHeader() const

.. c:function:: CudaMem::operator GpuMat() const

    Maps CPU memory to GPU address space and creates header without reference counting for it. This can be done only if memory was allocated with ALLOCZEROCOPYflag and if it is supported by hardware (laptops often share video and CPU memory, so address spaces can be mapped, and that eliminates extra copy).

.. index:: gpu::CudaMem::canMapHostMemory

gpu::CudaMem::canMapHostMemory
----------------------------------
.. c:function:: static bool CudaMem::canMapHostMemory()

    Returns true if the current hardware supports address space mapping and ALLOCZEROCOPYmemory allocation

.. index:: gpu::Stream

.. _gpu::Stream:

gpu::Stream
-----------
.. c:type:: gpu::Stream

This class encapsulated queue of the asynchronous calls. Some functions have overloads with additional
:func:`gpu::Stream` parameter. The overloads do initialization work (allocate output buffers, upload constants, etc.), start GPU kernel and return before results are ready. A check if all operation are complete can be performed via
:func:`gpu::Stream::queryIfComplete()` .  Asynchronous upload/download have to be performed from/to page-locked buffers, i.e. using
:func:`gpu::CudaMem` or
:func:`Mat` header that points to a region of
:func:`gpu::CudaMem` .

**Please note the limitation**
: currently it is not guaranteed that all will work properly if one operation will be enqueued twice with different data. Some functions use constant GPU memory and next call may update the memory before previous has been finished. But calling asynchronously different operations is safe because each operation has own constant buffer. Memory copy/upload/download/set operations to buffers hold by user are also safe. ::

    class CV_EXPORTS Stream
    {
    public:
            Stream();
            ~Stream();

            Stream(const Stream&);
            Stream& operator=(const Stream&);

            bool queryIfComplete();
            void waitForCompletion();

            //! downloads asynchronously.
            // Warning! cv::Mat must point to page locked memory
                     (i.e. to CudaMem data or to its subMat)
            void enqueueDownload(const GpuMat& src, CudaMem& dst);
            void enqueueDownload(const GpuMat& src, Mat& dst);

            //! uploads asynchronously.
            // Warning! cv::Mat must point to page locked memory
                     (i.e. to CudaMem data or to its ROI)
            void enqueueUpload(const CudaMem& src, GpuMat& dst);
            void enqueueUpload(const Mat& src, GpuMat& dst);

            void enqueueCopy(const GpuMat& src, GpuMat& dst);

            void enqueueMemSet(const GpuMat& src, Scalar val);
            void enqueueMemSet(const GpuMat& src, Scalar val, const GpuMat& mask);

            // converts matrix type, ex from float to uchar depending on type
            void enqueueConvert(const GpuMat& src, GpuMat& dst, int type,
                    double a = 1, double b = 0);
    };


.. index:: gpu::Stream::queryIfComplete

gpu::Stream::queryIfComplete
--------------------------------
.. c:function:: bool Stream::queryIfComplete()

    Returns true if the current stream queue is finished, otherwise false.

.. index:: gpu::Stream::waitForCompletion

gpu::Stream::waitForCompletion
----------------------------------
.. c:function:: void Stream::waitForCompletion()

    Blocks until all operations in the stream are complete.

.. index:: gpu::StreamAccessor

.. _gpu::StreamAccessor:

gpu::StreamAccessor
-------------------
.. c:type:: gpu::StreamAccessor

This class provides possibility to get ``cudaStream_t`` from
:func:`gpu::Stream` . This class is declared in ``stream_accessor.hpp`` because that is only public header that depend on Cuda Runtime API. Including it will bring the dependency to your code. ::

    struct StreamAccessor
    {
            CV_EXPORTS static cudaStream_t getStream(const Stream& stream);
    };


.. index:: gpu::createContinuous

gpu::createContinuous
-------------------------
.. c:function:: void createContinuous(int rows, int cols, int type, GpuMat\& m)

    Creates continuous matrix in GPU memory.

    :param rows: Row count.

    :param cols: Column count.

    :param type: Type of the matrix.

    :param m: Destination matrix. Will be only reshaped if it has proper type and area ( ``rows``   :math:`\times`   ``cols`` ).

Also the following wrappers are available:

.. c:function:: GpuMat createContinuous(int rows, int cols, int type)

.. c:function:: void createContinuous(Size size, int type, GpuMat\& m)

.. c:function:: GpuMat createContinuous(Size size, int type)

Matrix is called continuous if its elements are stored continuously, i.e. wuthout gaps in the end of each row.

.. index:: gpu::ensureSizeIsEnough

gpu::ensureSizeIsEnough
---------------------------
.. c:function:: void ensureSizeIsEnough(int rows, int cols, int type, GpuMat\& m)

    Ensures that size of matrix is big enough and matrix has proper type. The function doesn't reallocate memory if the  matrix has proper attributes already.

    :param rows: Minimum desired number of rows.

    :param cols: Minimum desired number of cols.

    :param type: Desired matrix type.

    :param m: Destination matrix.

Also the following wrapper is available:

.. c:function:: void ensureSizeIsEnough(Size size, int type, GpuMat\& m)

