GPU module introduction
=======================

.. highlight:: cpp

General information
-------------------

The OpenCV GPU module is a set of classes and functions to utilize GPU computational capabilities. It is implemented using NVidia CUDA Runtime API, so only the NVidia GPUs are supported. It includes utility functions, low-level vision primitives as well as high-level algorithms. The utility functions and low-level primitives provide a powerful infrastructure for developing fast vision algorithms taking advantage of GPU. Whereas the high-level functionality includes some state-of-the-art algorithms (such as stereo correspondence, face and people detectors etc.), ready to be used by the application developers.

The GPU module is designed as host-level API, i.e. if a user has pre-compiled OpenCV GPU binaries, it is not necessary to have Cuda Toolkit installed or write any extra code to make use of the GPU.

The GPU module depends on the Cuda Toolkit and NVidia Performance Primitives library (NPP). Make sure you have the latest versions of those. The two libraries can be downloaded from NVidia site for all supported platforms. To compile OpenCV GPU module you will need a compiler compatible with Cuda Runtime Toolkit.

OpenCV GPU module is designed for ease of use and does not require any knowledge of Cuda. Though, such a knowledge will certainly be useful in non-trivial cases, or when you want to get the highest performance. It is helpful to have understanding of the costs of various operations, what the GPU does, what are the preferred data formats etc. The GPU module is an effective instrument for quick implementation of GPU-accelerated computer vision algorithms. However, if you algorithm involves many simple operations, then for the best possible performance you may still need to write your own kernels, to avoid extra write and read operations on the intermediate results.

To enable CUDA support, configure OpenCV using CMake with ``WITH_CUDA=ON`` . When the flag is set and if CUDA is installed, the full-featured OpenCV GPU module will be built. Otherwise, the module will still be built, but at runtime all functions from the module will throw :c:type:`Exception` with ``CV_GpuNotSupported`` error code, except for :cpp:func:`gpu::getCudaEnabledDeviceCount`. The latter function will return zero GPU count in this case. Building OpenCV without CUDA support does not perform device code compilation, so it does not require Cuda Toolkit installed. Therefore, using :cpp:func:`gpu::getCudaEnabledDeviceCount` function it is possible to implement a high-level algorithm that will detect GPU presence at runtime and choose the appropriate implementation (CPU or GPU) accordingly.

Compilation for different NVidia platforms.
-------------------------------------------

NVidia compiler allows generating binary code (cubin and fatbin) and intermediate code (PTX). Binary code often implies a specific GPU architecture and generation, so the compatibility with other GPUs is not guaranteed. PTX is targeted for a virtual platform, which is defined entirely by the set of capabilities, or features. Depending on the virtual platform chosen, some of the instructions will be emulated or disabled, even if the real hardware supports all the features.

On first call, the PTX code is compiled to binary code for the particular GPU using JIT compiler. When the target GPU has lower "compute capability" (CC) than the PTX code, JIT fails.

By default, the OpenCV GPU module includes:

* Binaries for compute capabilities 1.1, 1.2, 1.3 and 2.0 (controlled by ``CUDA_ARCH_BIN`` in CMake)

* PTX code for compute capabilities 1.1 and 1.3 (controlled by ``CUDA_ARCH_PTX`` in CMake)

That means for devices with CC 1.1, 1.2, 1.3 and 2.0 binary images are ready to run. For all newer platforms the PTX code for 1.3 is JIT'ed to a binary image. For devices with CC 1.0 no code is available and the functions will throw
:c:type:`Exception`. For platforms where JIT compilation is performed first run will be slow.

If you happen to have GPU with CC 1.0, the GPU module can still be compiled on it and most of the functions will run just fine on such card. Simply add "1.0" to the list of binaries, for example, ``CUDA_ARCH_BIN="1.0 1.3 2.0"``. The functions that can not be run on CC 1.0 GPUs will throw an exception.

You can always determine at runtime whether OpenCV GPU built binaries (or PTX code) are compatible with your GPU. The function :cpp:func:`gpu::DeviceInfo::isCompatible` return the compatibility status (true/false).

Threading and multi-threading.
------------------------------

OpenCV GPU module follows Cuda Runtime API conventions regarding the multi-threaded programming. That is, on first the API call a Cuda context is created implicitly, attached to the current CPU thread and then is used as the thread's "current" context. All further operations, such as memory allocation, GPU code compilation, will be associated with the context and the thread. Because any other thread is not attached to the context, memory (and other resources) allocated in the first thread can not be accessed by the other thread. Instead, for this other thread Cuda will create another context associated with it. In short, by default different threads do not share resources.

But such limitation can be removed using Cuda Driver API (version 3.1 or later). User can retrieve context reference for one thread, attach it to another thread and make it "current" for that thread. Then the threads can share memory and other resources. It is also possible to create a context explicitly before calling any GPU code and attach it to all the threads that you want to share the resources.

Also it is possible to create context explicitly using Cuda Driver API, attach and make "current" for all necessary threads. Cuda Runtime API (and OpenCV functions respectively) will pick up it.

Multi-GPU
---------

In the current version each of the OpenCV GPU algorithms can use only a single GPU. So, to utilize multiple GPUs, user has to manually distribute the work between the GPUs. Here are the two ways of utilizing multiple GPUs:

* If you only use synchronous functions, first, create several CPU threads (one per each GPU) and from within each thread create CUDA context for the corresponding GPU using :cpp:func:`gpu::setDevice` or Driver API. That's it. Now each of the threads will use the associated GPU.

* In case of asynchronous functions, it is possible to create several Cuda contexts associated with different GPUs but attached to one CPU thread. This can be done only by Driver API. Within the thread you can switch from one GPU to another by making the corresponding context "current". With non-blocking GPU calls managing algorithm is clear.

While developing algorithms for multiple GPUs a data passing overhead have to be taken into consideration. For primitive functions and for small images it can be significant and eliminate all the advantages of having multiple GPUs. But for high level algorithms Multi-GPU acceleration may be suitable. For example, Stereo Block Matching algorithm has been successfully parallelized using the following algorithm:

* Each image of the stereo pair is split into two horizontal overlapping stripes.
* Each pair of stripes (from the left and the right images) has been processed on a separate Fermi GPU
* The results are merged into the single disparity map.

With this scheme dual GPU gave 180 % performance increase comparing to the single Fermi GPU. The source code of the example is available at https://code.ros.org/svn/opencv/trunk/opencv/examples/gpu/.
