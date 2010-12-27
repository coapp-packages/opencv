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

#include "precomp.hpp"

using namespace cv;

/*
 *  SimpleBlobDetector
 */
SimpleBlobDetector::Params::Params()
{
  thresholdStep = 10;
  minThreshold = 50;
  maxThreshold = 220;
  maxCentersDist = 10;
  defaultKeypointSize = 1;
  minRepeatability = 2;
  computeRadius = true;
  filterByColor = true;
  blobColor = 0;

  isGrayscaleCentroid = false;
  centroidROIMargin = 2;

  filterByArea = true;
  minArea = 25;
  maxArea = 5000;

  filterByInertia = true;
  //minInertiaRatio = 0.6;
  minInertiaRatio = 0.1f;

  filterByConvexity = true;
  //minConvexity = 0.8;
  minConvexity = 0.95f;

  filterByCircularity = false;
  minCircularity = 0.8f;
}

SimpleBlobDetector::SimpleBlobDetector(const SimpleBlobDetector::Params &parameters) :
  params(parameters)
{
}

Point2d SimpleBlobDetector::computeGrayscaleCentroid(const Mat &image, const vector<Point> &contour) const
{
  Rect rect = boundingRect(Mat(contour));
  rect.x -= params.centroidROIMargin;
  rect.y -= params.centroidROIMargin;
  rect.width += 2 * params.centroidROIMargin;
  rect.height += 2 * params.centroidROIMargin;

  rect.x = rect.x < 0 ? 0 : rect.x;
  rect.y = rect.y < 0 ? 0 : rect.y;
  rect.width = rect.x + rect.width < image.cols ? rect.width : image.cols - rect.x;
  rect.height = rect.y + rect.height < image.rows ? rect.height : image.rows - rect.y;

  Mat roi = image(rect);
  assert( roi.type() == CV_8UC1 );

  Mat invRoi = 255 - roi;
  invRoi.convertTo(invRoi, CV_32FC1);
  invRoi = invRoi.mul(invRoi);

  Moments moms = moments(invRoi);

  Point2d tl = rect.tl();
  Point2d roiCentroid(moms.m10 / moms.m00, moms.m01 / moms.m00);

  Point2d centroid = tl + roiCentroid;
  return centroid;
}

void SimpleBlobDetector::findBlobs(const cv::Mat &image, const cv::Mat &binaryImage, vector<Center> &centers) const
{
  centers.clear();

  vector<vector<Point> > contours;
  Mat tmpBinaryImage = binaryImage.clone();
  findContours(tmpBinaryImage, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

  //Mat keypointsImage;
  //cvtColor( binaryImage, keypointsImage, CV_GRAY2RGB );

  //Mat contoursImage;
  //cvtColor( binaryImage, contoursImage, CV_GRAY2RGB );
  //drawContours( contoursImage, contours, -1, Scalar(0,255,0) );
  //imshow("contours", contoursImage );

  for (size_t contourIdx = 0; contourIdx < contours.size(); contourIdx++)
  {
    Center center;
    center.confidence = 1;
    Moments moms = moments(Mat(contours[contourIdx]));
    if (params.filterByArea)
    {
      double area = moms.m00;
      if (area < params.minArea || area > params.maxArea)
        continue;
    }

    if (params.filterByCircularity)
    {
      double area = moms.m00;
      double perimeter = arcLength(Mat(contours[contourIdx]), true);
      double ratio = 4 * CV_PI * area / (perimeter * perimeter);
      if (ratio < params.minCircularity)
        continue;
    }

    if (params.filterByInertia)
    {
      double denominator = sqrt(pow(2 * moms.mu11, 2) + pow(moms.mu20 - moms.mu02, 2));
      const double eps = 1e-2;
      double ratio;
      if (denominator > eps)
      {
        double cosmin = (moms.mu20 - moms.mu02) / denominator;
        double sinmin = 2 * moms.mu11 / denominator;
        double cosmax = -cosmin;
        double sinmax = -sinmin;

        double imin = 0.5 * (moms.mu20 + moms.mu02) - 0.5 * (moms.mu20 - moms.mu02) * cosmin - moms.mu11 * sinmin;
        double imax = 0.5 * (moms.mu20 + moms.mu02) - 0.5 * (moms.mu20 - moms.mu02) * cosmax - moms.mu11 * sinmax;
        ratio = imin / imax;
      }
      else
      {
        ratio = 1;
      }

      if (ratio < params.minInertiaRatio)
        continue;

      center.confidence = ratio * ratio;
    }

    if (params.filterByConvexity)
    {
      vector<Point> hull;
      convexHull(Mat(contours[contourIdx]), hull);
      double area = contourArea(Mat(contours[contourIdx]));
      double hullArea = contourArea(Mat(hull));
      double ratio = area / hullArea;
      if (ratio < params.minConvexity)
        continue;
    }

    if (params.isGrayscaleCentroid)
      center.location = computeGrayscaleCentroid(image, contours[contourIdx]);
    else
      center.location = Point2d(moms.m10 / moms.m00, moms.m01 / moms.m00);

    if (params.filterByColor)
    {
      if (binaryImage.at<uchar> (cvRound(center.location.y), cvRound(center.location.x)) != params.blobColor)
        continue;
    }

    if (params.computeRadius)
    {
      vector<double> dists;
      for (size_t pointIdx = 0; pointIdx < contours[contourIdx].size(); pointIdx++)
      {
        Point2d pt = contours[contourIdx][pointIdx];
        dists.push_back(norm(center.location - pt));
      }
      std::sort(dists.begin(), dists.end());
      center.radius = (dists[(dists.size() - 1) / 2] + dists[dists.size() / 2]) / 2.;
    }

    centers.push_back(center);

    //circle( keypointsImage, center.location, 1, Scalar(0,0,255), 1 );
  }
  //imshow("bk", keypointsImage );
  //waitKey();
}

void SimpleBlobDetector::detectImpl(const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints, const cv::Mat& mask) const
{
  keypoints.clear();
  Mat grayscaleImage;
  if (image.channels() == 3)
    cvtColor(image, grayscaleImage, CV_BGR2GRAY);
  else
    grayscaleImage = image;

  vector<vector<Center> > centers;
  for (double thresh = params.minThreshold; thresh < params.maxThreshold; thresh += params.thresholdStep)
  {
    Mat binarizedImage;
    threshold(grayscaleImage, binarizedImage, thresh, 255, THRESH_BINARY);

    //Mat keypointsImage;
    //cvtColor( binarizedImage, keypointsImage, CV_GRAY2RGB );

    vector<Center> curCenters;
    findBlobs(grayscaleImage, binarizedImage, curCenters);
    for (size_t i = 0; i < curCenters.size(); i++)
    {
      //circle(keypointsImage, curCenters[i].location, 1, Scalar(0,0,255),-1);

      bool isNew = true;
      for (size_t j = 0; j < centers.size(); j++)
      {
        double dist = norm(centers[j][0].location - curCenters[i].location);
        if (params.computeRadius)
          isNew = dist >= centers[j][0].radius && dist >= curCenters[i].radius && dist >= params.maxCentersDist;
        else
          isNew = dist >= params.maxCentersDist;
        if (!isNew)
        {
          centers[j].push_back(curCenters[i]);
          //          if( centers[j][0].radius < centers[j][ centers[j].size()-1 ].radius )
          //          {
          //            std::swap( centers[j][0], centers[j][ centers[j].size()-1 ] );
          //          }
          break;
        }
      }
      if (isNew)
      {
        centers.push_back(vector<Center> (1, curCenters[i]));
      }
    }
    //imshow("binarized", keypointsImage );
    //waitKey();
  }

  for (size_t i = 0; i < centers.size(); i++)
  {
    if (centers[i].size() < params.minRepeatability)
      continue;
    Point2d sumPoint(0, 0);
    double normalizer = 0;
    for (size_t j = 0; j < centers[i].size(); j++)
    {
      sumPoint += centers[i][j].confidence * centers[i][j].location;
      normalizer += centers[i][j].confidence;
    }
    sumPoint *= (1. / normalizer);
    KeyPoint kpt(sumPoint, params.defaultKeypointSize);
    keypoints.push_back(kpt);
  }
}
