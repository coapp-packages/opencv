# Install script for directory: C:/git/opencv/opencv/doc

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
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/doc" TYPE FILE FILES
    "C:/git/opencv/opencv/doc/haartraining.htm"
    "C:/git/opencv/opencv/doc/check_docs_whitelist.txt"
    "C:/git/opencv/opencv/doc/CMakeLists.txt"
    "C:/git/opencv/opencv/doc/license.txt"
    "C:/git/opencv/opencv/doc/packaging.txt"
    "C:/git/opencv/opencv/doc/opencv.jpg"
    "C:/git/opencv/opencv/doc/acircles_pattern.png"
    "C:/git/opencv/opencv/doc/opencv-logo.png"
    "C:/git/opencv/opencv/doc/opencv-logo2.png"
    "C:/git/opencv/opencv/doc/pattern.png"
    "C:/git/opencv/opencv/doc/opencv2refman.pdf"
    "C:/git/opencv/opencv/doc/opencv_cheatsheet.pdf"
    "C:/git/opencv/opencv/doc/opencv_tutorials.pdf"
    "C:/git/opencv/opencv/doc/opencv_user.pdf"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "main")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "main")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/doc/vidsurv" TYPE FILE FILES
    "C:/git/opencv/opencv/doc/vidsurv/Blob_Tracking_Modules.doc"
    "C:/git/opencv/opencv/doc/vidsurv/Blob_Tracking_Tests.doc"
    "C:/git/opencv/opencv/doc/vidsurv/TestSeq.doc"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "main")

