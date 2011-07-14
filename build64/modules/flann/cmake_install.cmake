# Install script for directory: C:/git/opencv/opencv/modules/flann

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "C:/git/opencv/build64/install")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "main")
  IF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/git/opencv/build64/lib/Debug/opencv_flann231d.lib")
  ENDIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  IF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/git/opencv/build64/lib/Release/opencv_flann231.lib")
  ENDIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  IF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/git/opencv/build64/lib/MinSizeRel/opencv_flann231.lib")
  ENDIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  IF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/git/opencv/build64/lib/RelWithDebInfo/opencv_flann231.lib")
  ENDIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "main")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "main")
  IF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/git/opencv/build64/bin/Debug/opencv_flann231d.dll")
  ENDIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  IF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/git/opencv/build64/bin/Release/opencv_flann231.dll")
  ENDIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  IF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/git/opencv/build64/bin/MinSizeRel/opencv_flann231.dll")
  ENDIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  IF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/git/opencv/build64/bin/RelWithDebInfo/opencv_flann231.dll")
  ENDIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "main")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "main")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2/flann" TYPE FILE FILES
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/allocator.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/all_indices.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/any.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/autotuned_index.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/composite_index.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/config.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/defines.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/dist.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/dummy.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/dynamic_bitset.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/flann.hpp"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/flann_base.hpp"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/general.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/ground_truth.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/hdf5.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/heap.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/hierarchical_clustering_index.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/index_testing.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/kdtree_index.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/kdtree_single_index.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/kmeans_index.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/linear_index.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/logger.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/lsh_index.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/lsh_table.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/matrix.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/miniflann.hpp"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/nn_index.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/object_factory.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/params.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/random.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/result_set.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/sampling.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/saving.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/simplex_downhill.h"
    "C:/git/opencv/opencv/modules/flann/include/opencv2/flann/timer.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "main")

