# Install script for directory: C:/git/opencv/opencv/modules

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

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("C:/git/opencv/build64/modules/calib3d/cmake_install.cmake")
  INCLUDE("C:/git/opencv/build64/modules/core/cmake_install.cmake")
  INCLUDE("C:/git/opencv/build64/modules/features2d/cmake_install.cmake")
  INCLUDE("C:/git/opencv/build64/modules/flann/cmake_install.cmake")
  INCLUDE("C:/git/opencv/build64/modules/ts/cmake_install.cmake")
  INCLUDE("C:/git/opencv/build64/modules/highgui/cmake_install.cmake")
  INCLUDE("C:/git/opencv/build64/modules/imgproc/cmake_install.cmake")
  INCLUDE("C:/git/opencv/build64/modules/legacy/cmake_install.cmake")
  INCLUDE("C:/git/opencv/build64/modules/contrib/cmake_install.cmake")
  INCLUDE("C:/git/opencv/build64/modules/ml/cmake_install.cmake")
  INCLUDE("C:/git/opencv/build64/modules/objdetect/cmake_install.cmake")
  INCLUDE("C:/git/opencv/build64/modules/video/cmake_install.cmake")
  INCLUDE("C:/git/opencv/build64/modules/traincascade/cmake_install.cmake")
  INCLUDE("C:/git/opencv/build64/modules/haartraining/cmake_install.cmake")
  INCLUDE("C:/git/opencv/build64/modules/gpu/cmake_install.cmake")
  INCLUDE("C:/git/opencv/build64/modules/stitching/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

