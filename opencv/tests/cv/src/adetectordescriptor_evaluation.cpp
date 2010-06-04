/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
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
//   * The name of Intel Corporation may not be used to endorse or promote products
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

#include "cvtest.h"
#include <limits>
#include <cstdio>

using namespace std;
using namespace cv;

#define AFFINE_COVARIANT_VERSION

inline Point2f applyHomography( const Mat_<double>& H, const Point2f& pt )
{
    double z = H(2,0)*pt.x + H(2,1)*pt.y + H(2,2);
    if( z )
    {
        double w = 1./z;
        return Point2f( (H(0,0)*pt.x + H(0,1)*pt.y + H(0,2))*w, (H(1,0)*pt.x + H(1,1)*pt.y + H(1,2))*w );
    }
    return Point2f( numeric_limits<double>::max(), numeric_limits<double>::max() );
}

inline void linearizeHomographyAt( const Mat_<double>& H, const Point2f& pt, Mat_<double>& A )
{
    A.create(2,2);
    double p1 = H(0,0)*pt.x + H(0,1)*pt.y + H(0,2),
           p2 = H(1,0)*pt.x + H(1,1)*pt.y + H(1,2),
           p3 = H(2,0)*pt.x + H(2,1)*pt.y + H(2,2),
           p3_2 = p3*p3;
    if( p3 )
    {
        A(0,0) = H(0,0)/p3 - p1*H(2,0)/p3_2; // fxdx
        A(0,1) = H(0,1)/p3 - p1*H(2,1)/p3_2; // fxdy

        A(1,0) = H(1,0)/p3 - p2*H(2,0)/p3_2; // fydx
        A(1,1) = H(1,1)/p3 - p2*H(2,1)/p3_2; // fydx
    }
    else
        A.setTo(Scalar::all(numeric_limits<double>::max()));
}

#ifndef AFFINE_COVARIANT_VERSION
/****************************************************************************************\
*  1. Initial version of evaluating detectors. This version calculate repeatability      *
*     for scale invariant detectors (circular regions)                                   *
\****************************************************************************************/

// Find the key points located in the part of the scene present in both images
// and project keypoints2 on img1
void getCircularKeyPointsInCommonPart( const Mat& img1, const Mat img2, const Mat& H12,
                                       const vector<KeyPoint>& keypoints1, const vector<KeyPoint>& keypoints2,
                                       vector<KeyPoint>& ckeypoints1, vector<KeyPoint>& ckeypoints2t )
{
    assert( !img1.empty() && !img2.empty() );
    assert( !H12.empty() && H12.cols==3 && H12.rows==3 && H12.type()==CV_64FC1 );
    ckeypoints1.clear();
    ckeypoints2t.clear();

    Rect r1(0, 0, img1.cols, img1.rows), r2(0, 0, img2.cols, img2.rows);
    Mat H21; invert( H12, H21 );

    for( vector<KeyPoint>::const_iterator it = keypoints1.begin();
                 it != keypoints1.end(); ++it )
    {
        if( r2.contains(applyHomography(H12, it->pt)) )
            ckeypoints1.push_back(*it);
    }
    for( vector<KeyPoint>::const_iterator it = keypoints2.begin();
                 it != keypoints2.end(); ++it )
    {
        Point2f pt = applyHomography(H21, it->pt);
        if( r1.contains(pt) )
        {
            KeyPoint kp = *it;
            kp.pt = pt;
            Mat_<double> A, eval;
            linearizeHomographyAt(H21, it->pt, A);
            eigen(A, eval);
            assert( eval.type()==CV_64FC1 && eval.cols==1 && eval.rows==2 );
            kp.size *= sqrt(eval(0,0) * eval(1,0)) /*scale from linearized homography matrix*/;
            ckeypoints2t.push_back(kp);
        }
    }
}

// Locations p1 and p2 are repeated if ||p1 - H21*p2|| < 1.5 pixels.
// Regions are repeated if Es < 0.4 (Es differs for scale invariant and affine invarian detectors).
// For more details see "Scale&Affine Invariant Interest Point Detectors", Mikolajczyk, Schmid.
void evaluateScaleInvDetectors( const Mat& img1, const Mat img2, const Mat& H12,
                                const vector<KeyPoint>& keypoints1, const vector<KeyPoint>& keypoints2,
                                int& repeatingLocationCount, float& repeatingLocationRltv,
                                int& repeatingRegionCount, float& repeatingRegionRltv )
{
    const double locThreshold = 1.5,
                 regThreshold = 0.4;
    assert( !img1.empty() && !img2.empty() );
    assert( !H12.empty() && H12.cols==3 && H12.rows==3 && H12.type()==CV_64FC1 );

    Mat H21; invert( H12, H21 );

    vector<KeyPoint> ckeypoints1, ckeypoints2t;
    getCircularKeyPointsInCommonPart( img1, img2, H12, keypoints1, keypoints2, ckeypoints1, ckeypoints2t );

    vector<KeyPoint> *smallKPSet = &ckeypoints1, *bigKPSet = &ckeypoints2t;
    if( ckeypoints1.size() > ckeypoints2t.size() )
    {
        smallKPSet = &ckeypoints2t;
        bigKPSet = &ckeypoints1;
    }

    if( smallKPSet->size() == 0 )
    {
        repeatingLocationCount = repeatingRegionCount = -1;
        repeatingLocationRltv = repeatingRegionRltv = -1.f;
    }
    else
    {
        vector<bool> matchedMask( bigKPSet->size(), false);
        repeatingLocationCount = repeatingRegionCount = 0;
        for( vector<KeyPoint>::const_iterator skpIt = smallKPSet->begin(); skpIt != smallKPSet->end(); ++skpIt )
        {
            int nearestIdx = -1, bkpIdx = 0;
            double minDist = numeric_limits<double>::max();
            vector<KeyPoint>::const_iterator nearestBkp;
            for( vector<KeyPoint>::const_iterator bkpIt = bigKPSet->begin(); bkpIt != bigKPSet->end(); ++bkpIt, bkpIdx++ )
            {
                if( !matchedMask[bkpIdx] )
                {
                    Point p1(cvRound(skpIt->pt.x), cvRound(skpIt->pt.y)),
                          p2(cvRound(bkpIt->pt.x), cvRound(bkpIt->pt.y));
                    double dist = norm(p1 - p2);
                    if( dist < minDist )
                    {
                        nearestIdx = bkpIdx;
                        minDist = dist;
                        nearestBkp = bkpIt;
                    }
                }
            }
            if( minDist < locThreshold )
            {
                matchedMask[nearestIdx] = true;
                repeatingLocationCount++;
                double minRadius = min( skpIt->size, nearestBkp->size ),
                       maxRadius = max( skpIt->size, nearestBkp->size );
                double Es = abs(1 - (minRadius*minRadius)/(maxRadius*maxRadius));
                if( Es < regThreshold )
                    repeatingRegionCount++;
            }
        }
        repeatingLocationRltv = smallKPSet->size() ? (float)repeatingLocationCount / smallKPSet->size() : 0;
        repeatingRegionRltv = smallKPSet->size() ? (float)repeatingRegionCount / smallKPSet->size() : 0;
    }
}
#else
/****************************************************************************************\
*  2. Functions to evaluate affine covariant detectors and descriptors.                  *
\****************************************************************************************/
class EllipticKeyPoint
{
public:
    EllipticKeyPoint();
    EllipticKeyPoint( const Point2f& _center, const Scalar& _ellipse );

    static Mat_<double> getSecondMomentsMatrix( const Scalar& _ellipse );
    Mat_<double> getSecondMomentsMatrix() const;

    void calcProjection( const Mat_<double>& H, EllipticKeyPoint& projection ) const;

    Point2f center;
    Scalar ellipse; // 3 elements a, b, c: ax^2+2bxy+cy^2=1
    Size_<float> axes; // half lenght of elipse axes
    Size_<float> boundingBox; // half sizes of bounding box
};

EllipticKeyPoint::EllipticKeyPoint()
{
    *this = EllipticKeyPoint(Point2f(0,0), Scalar(1, 0, 1) );
}

EllipticKeyPoint::EllipticKeyPoint( const Point2f& _center, const Scalar& _ellipse )
{
    center = _center;
    ellipse = _ellipse;

    Mat_<double> M = getSecondMomentsMatrix(_ellipse), eval;
    eigen( M, eval );
    assert( eval.rows == 2 && eval.cols == 1 );
    axes.width = 1.f / sqrt(eval(0,0));
    axes.height = 1.f / sqrt(eval(1,0));

    float ac_b2 = ellipse[0]*ellipse[2] - ellipse[1]*ellipse[1];
    boundingBox.width = sqrt(ellipse[2]/ac_b2);
    boundingBox.height = sqrt(ellipse[0]/ac_b2);
}

Mat_<double> EllipticKeyPoint::getSecondMomentsMatrix( const Scalar& _ellipse )
{
    Mat_<double> M(2, 2);
    M(0,0) = _ellipse[0];
    M(1,0) = M(0,1) = _ellipse[1];
    M(1,1) = _ellipse[2];
    return M;
}

Mat_<double> EllipticKeyPoint::getSecondMomentsMatrix() const
{
    return getSecondMomentsMatrix(ellipse);
}

void EllipticKeyPoint::calcProjection( const Mat_<double>& H, EllipticKeyPoint& projection ) const
{
    Point2f dstCenter = applyHomography(H, center);

    Mat_<double> invM; invert(getSecondMomentsMatrix(), invM);
    Mat_<double> Aff; linearizeHomographyAt(H, center, Aff);
    Mat_<double> dstM; invert(Aff*invM*Aff.t(), dstM);

    projection = EllipticKeyPoint( dstCenter, Scalar(dstM(0,0), dstM(0,1), dstM(1,1)) );
}

void calcEllipticKeyPointProjections( const vector<EllipticKeyPoint>& src, const Mat_<double>& H, vector<EllipticKeyPoint>& dst )
{
    if( !src.empty() )
    {
        assert( !H.empty() && H.cols == 3 && H.rows == 3);
        dst.resize(src.size());
        vector<EllipticKeyPoint>::const_iterator srcIt = src.begin();
        vector<EllipticKeyPoint>::iterator       dstIt = dst.begin();
        for( ; srcIt != src.end(); ++srcIt, ++dstIt )
            srcIt->calcProjection(H, *dstIt);
    }
}

void transformToEllipticKeyPoints( const vector<KeyPoint>& src, vector<EllipticKeyPoint>& dst )
{
    if( !src.empty() )
    {
        dst.resize(src.size());
        for( size_t i = 0; i < src.size(); i++ )
        {
            float rad = src[i].size/2;
            assert( rad );
            float fac = 1.f/(rad*rad);
            dst[i] = EllipticKeyPoint( src[i].pt, Scalar(fac, 0, fac) );
        }
    }
}

void transformToKeyPoints( const vector<EllipticKeyPoint>& src, vector<KeyPoint>& dst )
{
    if( !src.empty() )
    {
        dst.resize(src.size());
        for( size_t i = 0; i < src.size(); i++ )
        {
            Size_<float> axes = src[i].axes;
            float rad = sqrt(axes.height*axes.width);
            dst[i] = KeyPoint(src[i].center, 2*rad );
        }
    }
}

void calcKeyPointProjections( const vector<KeyPoint>& src, const Mat_<double>& H, vector<KeyPoint>& dst )
{
    if(  !src.empty() )
    {
        assert( !H.empty() && H.cols == 3 && H.rows == 3);
        dst.resize(src.size());
        vector<KeyPoint>::const_iterator srcIt = src.begin();
        vector<KeyPoint>::iterator       dstIt = dst.begin();
        for( ; srcIt != src.end(); ++srcIt, ++dstIt )
        {
            Point2f dstPt = applyHomography(H, srcIt->pt);

            float srcSize2 = srcIt->size * srcIt->size;
            Mat_<double> invM; invert(EllipticKeyPoint::getSecondMomentsMatrix( Scalar(1./srcSize2, 0., 1./srcSize2)), invM);
            Mat_<double> Aff; linearizeHomographyAt(H, srcIt->pt, Aff);
            Mat_<double> dstM; invert(Aff*invM*Aff.t(), dstM);
            Mat_<double> eval; eigen( dstM, eval );
            assert( eval(0,0) && eval(1,0) );
            float dstSize = pow(1./(eval(0,0)*eval(1,0)), 0.25);

            // TODO: check angle projection
            float srcAngleRad = srcIt->angle*CV_PI/180;
            Point2f vec1(cos(srcAngleRad), sin(srcAngleRad)), vec2;
            vec2.x = Aff(0,0)*vec1.x + Aff(0,1)*vec1.y;
            vec2.y = Aff(1,0)*vec1.x + Aff(0,1)*vec1.y;
            float dstAngleGrad = fastAtan2(vec2.y, vec2.x);

            *dstIt = KeyPoint( dstPt, dstSize, dstAngleGrad, srcIt->response, srcIt->octave, srcIt->class_id );
        }
    }
}

void filterKeyPointsByImageSize( vector<KeyPoint>& keypoints, const Size& imgSize )
{
    if( !keypoints.empty() )
    {
        vector<KeyPoint> filtered;
        filtered.reserve(keypoints.size());
        Rect r(0, 0, imgSize.width, imgSize.height);
        vector<KeyPoint>::const_iterator it = keypoints.begin();
        for( int i = 0; it != keypoints.end(); ++it, i++ )
            if( r.contains(it->pt) )
                filtered.push_back(*it);
        keypoints.assign(filtered.begin(), filtered.end());
    }
}

/*
 * calulate ovelap errors
 */
void overlap( const vector<EllipticKeyPoint>& keypoints1, const vector<EllipticKeyPoint>& keypoints2t, bool commonPart,
              SparseMat_<float>& overlaps )
{
    overlaps.clear();
    if( keypoints1.empty() || keypoints2t.empty() )
        return;

    int size[] = { keypoints1.size(), keypoints2t.size() };
    overlaps.create( 2, size );

    for( size_t i1 = 0; i1 < keypoints1.size(); i1++ )
    {
        EllipticKeyPoint kp1 = keypoints1[i1];
        float maxDist = sqrt(kp1.axes.width*kp1.axes.height),
              fac = 30.f/maxDist;
        if( !commonPart)
            fac=3;

        maxDist = maxDist*4;
        fac = 1.0/(fac*fac);

        EllipticKeyPoint keypoint1a = EllipticKeyPoint( kp1.center, Scalar(fac*kp1.ellipse[0], fac*kp1.ellipse[1], fac*kp1.ellipse[2]) );

        for( size_t i2 = 0; i2 < keypoints2t.size(); i2++ )
        {
            EllipticKeyPoint kp2 = keypoints2t[i2];
            Point2f diff = kp2.center - kp1.center;

            if( norm(diff) < maxDist )
            {
                EllipticKeyPoint keypoint2a = EllipticKeyPoint( kp2.center, Scalar(fac*kp2.ellipse[0], fac*kp2.ellipse[1], fac*kp2.ellipse[2]) );
                //find the largest eigenvalue
                float maxx =  ceil(( keypoint1a.boundingBox.width > (diff.x+keypoint2a.boundingBox.width)) ?
                                     keypoint1a.boundingBox.width : (diff.x+keypoint2a.boundingBox.width));
                float minx = floor((-keypoint1a.boundingBox.width < (diff.x-keypoint2a.boundingBox.width)) ?
                                    -keypoint1a.boundingBox.width : (diff.x-keypoint2a.boundingBox.width));

                float maxy =  ceil(( keypoint1a.boundingBox.height > (diff.y+keypoint2a.boundingBox.height)) ?
                                     keypoint1a.boundingBox.height : (diff.y+keypoint2a.boundingBox.height));
                float miny = floor((-keypoint1a.boundingBox.height < (diff.y-keypoint2a.boundingBox.height)) ?
                                    -keypoint1a.boundingBox.height : (diff.y-keypoint2a.boundingBox.height));
                float mina = (maxx-minx) < (maxy-miny) ? (maxx-minx) : (maxy-miny) ;
                float dr = mina/50.0;
                float bua = 0, bna = 0;
                //compute the area
                for( float rx1 = minx; rx1 <= maxx; rx1+=dr )
                {
                    float rx2 = rx1-diff.x;
                    for( float ry1=miny; ry1<=maxy; ry1+=dr )
                    {
                        float ry2=ry1-diff.y;
                        //compute the distance from the ellipse center
                        float e1 = keypoint1a.ellipse[0]*rx1*rx1+2*keypoint1a.ellipse[1]*rx1*ry1+keypoint1a.ellipse[2]*ry1*ry1;
                        float e2 = keypoint2a.ellipse[0]*rx2*rx2+2*keypoint2a.ellipse[1]*rx2*ry2+keypoint2a.ellipse[2]*ry2*ry2;
                        //compute the area
                        if( e1<1 && e2<1 ) bna++;
                        if( e1<1 || e2<1 ) bua++;
                    }
                }
                if( bna > 0)
                    overlaps.ref(i1,i2) = 100.0*bna/bua;
            }
        }
    }
}

void filterEllipticKeyPointsByImageSize( vector<EllipticKeyPoint>& keypoints, const Size& imgSize )
{
    if( !keypoints.empty() )
    {
        vector<EllipticKeyPoint> filtered;
        filtered.reserve(keypoints.size());
        vector<EllipticKeyPoint>::const_iterator it = keypoints.begin();
        for( int i = 0; it != keypoints.end(); ++it, i++ )
        {
            if( it->center.x + it->boundingBox.width < imgSize.width &&
                it->center.x - it->boundingBox.width > 0 &&
                it->center.y + it->boundingBox.height < imgSize.height &&
                it->center.y - it->boundingBox.height > 0 )
                filtered.push_back(*it);
        }
        keypoints.assign(filtered.begin(), filtered.end());
    }
}

void getEllipticKeyPointsInCommonPart( vector<EllipticKeyPoint>& keypoints1, vector<EllipticKeyPoint>& keypoints2,
                              vector<EllipticKeyPoint>& keypoints1t, vector<EllipticKeyPoint>& keypoints2t,
                              Size& imgSize1, const Size& imgSize2 )
{
    filterEllipticKeyPointsByImageSize( keypoints1, imgSize1 );
    filterEllipticKeyPointsByImageSize( keypoints1t, imgSize2 );
    filterEllipticKeyPointsByImageSize( keypoints2, imgSize2 );
    filterEllipticKeyPointsByImageSize( keypoints2t, imgSize1 );
}

void calculateRepeatability( const vector<EllipticKeyPoint>& _keypoints1, const vector<EllipticKeyPoint>& _keypoints2,
                             const Mat& img1, const Mat& img2, const Mat& H1to2,
                             float& repeatability, int& correspondencesCount,
                             SparseMat_<uchar>* thresholdedOverlapMask=0 )
{
    vector<EllipticKeyPoint> keypoints1( _keypoints1.begin(), _keypoints1.end() ),
                             keypoints2( _keypoints2.begin(), _keypoints2.end() ),
                             keypoints1t( keypoints1.size() ),
                             keypoints2t( keypoints2.size() );

    // calculate projections of key points
    calcEllipticKeyPointProjections( keypoints1, H1to2, keypoints1t );
    Mat H2to1; invert(H1to2, H2to1);
    calcEllipticKeyPointProjections( keypoints2, H2to1, keypoints2t );

    bool ifEvaluateDetectors = !thresholdedOverlapMask; // == commonPart
    float overlapThreshold;
    if( ifEvaluateDetectors )
    {
        overlapThreshold = 100.f - 40.f;

        // remove key points from outside of the common image part
        Size sz1 = img1.size(), sz2 = img2.size();
        getEllipticKeyPointsInCommonPart( keypoints1, keypoints2, keypoints1t, keypoints2t, sz1, sz2 );
    }
    else
    {
        overlapThreshold = 100.f - 50.f;
    }
    int minCount = min( keypoints1.size(), keypoints2t.size() );

    // calculate overlap errors
    SparseMat_<float> overlaps;
    overlap( keypoints1, keypoints2t, ifEvaluateDetectors, overlaps );

    correspondencesCount = -1;
    repeatability = -1.f;
    const int* size = overlaps.size();
    if( !size || overlaps.nzcount() == 0 )
        return;

    if( ifEvaluateDetectors )
    {
        // regions one-to-one matching
        correspondencesCount = 0;
        SparseMat_<float> currOverlaps( 2, size );
        for( int y = 0; y < size[0]; y++ )
        {
            for( int x = 0; x < size[1]; x++ )
            {
                float val = overlaps(y,x);
                if ( val >= overlapThreshold )
                    currOverlaps.ref(y,x) = val;
            }
        }
        while( currOverlaps.nzcount() > 0 )
        {
            double maxOverlap = 0;
            int maxIdx[2];
            minMaxLoc( currOverlaps, 0, &maxOverlap, 0, maxIdx );
            for( size_t i1 = 0; i1 < keypoints1.size(); i1++ )
                currOverlaps.erase(i1, maxIdx[1]);
            for( size_t i2 = 0; i2 < keypoints2t.size(); i2++ )
                currOverlaps.erase(maxIdx[0], i2);
            correspondencesCount++;
        }
        repeatability = minCount ? (float)(correspondencesCount*100)/minCount : -1;
    }
    else
    {
        thresholdedOverlapMask->create( 2, size );
        for( int y = 0; y < size[0]; y++ )
        {
            for( int x = 0; x < size[1]; x++ )
            {
                float val = overlaps(y,x);
                if ( val >= overlapThreshold )
                    thresholdedOverlapMask->ref(y,x) = val;
            }
        }
    }
}


void evaluateDetectors( const vector<EllipticKeyPoint>& keypoints1, const vector<EllipticKeyPoint>& keypoints2,
                        const Mat& img1, const Mat& img2, const Mat& H1to2,
                        float& repeatability, int& correspCount )
{
    calculateRepeatability( keypoints1, keypoints2,
                            img1, img2, H1to2,
                            repeatability, correspCount );
}

inline float recall( int correctMatchCount, int correspondenceCount )
{
    return correspondenceCount ? (float)correctMatchCount / (float)correspondenceCount : -1;
}

inline float precision( int correctMatchCount, int falseMatchCount )
{
    return correctMatchCount + falseMatchCount ? (float)correctMatchCount / (float)(correctMatchCount + falseMatchCount) : -1;
}

void evaluateDescriptors( const vector<EllipticKeyPoint>& keypoints1, const vector<EllipticKeyPoint>& keypoints2,
                          const vector<int>& matches1to2,
                          const Mat& img1, const Mat& img2, const Mat& H1to2,
                          int& correctMatchCount, int& falseMatchCount, int& correspondenceCount )
{
    assert( !keypoints1.empty() && !keypoints2.empty() && !matches1to2.empty() );
    assert( keypoints1.size() == matches1to2.size() );

    float repeatability;
    int correspCount;
    SparseMat_<uchar> thresholdedOverlapMask; // thresholded allOverlapErrors
    calculateRepeatability( keypoints1, keypoints2,
                            img1, img2, H1to2,
                            repeatability, correspCount,
                            &thresholdedOverlapMask );
    correspondenceCount = thresholdedOverlapMask.nzcount();
    correctMatchCount = falseMatchCount = 0;
    for( size_t i1 = 0; i1 < matches1to2.size(); i1++ )
    {
        int i2 = matches1to2[i1];
        if( i2 > 0 )
        {
            if( thresholdedOverlapMask(i1, i2) )
                correctMatchCount++;
            else
                falseMatchCount++;
        }
    }
}

#endif
/****************************************************************************************\
*                                  Detectors evaluation                                 *
\****************************************************************************************/
const int DATASETS_COUNT = 8;
const int TEST_CASE_COUNT = 5;

const string IMAGE_DATASETS_DIR = "detectors_descriptors_evaluation/images_datasets/";
const string DETECTORS_DIR = "detectors_descriptors_evaluation/detectors/";
const string DESCRIPTORS_DIR = "detectors_descriptors_evaluation/descriptors/";
const string KEYPOINTS_DIR = "detectors_descriptors_evaluation/keypoints_datasets/";

const string PARAMS_POSTFIX = "_params.xml";
const string RES_POSTFIX = "_res.xml";

#ifndef AFFINE_COVARIANT_VERSION
const string RLC = "repeating_locations_count";
const string RLR = "repeating_locations_rltv";
const string RRC = "repeating_regions_count";
const string RRR = "repeating_regions_rltv";
#else
const string REPEAT = "repeatability";
const string CORRESP_COUNT = "correspondence_count";
#endif

string DATASET_NAMES[DATASETS_COUNT] = { "bark", "bikes", "boat", "graf", "leuven", "trees", "ubc", "wall"};

string DEFAULT_PARAMS = "default";

string IS_ACTIVE_PARAMS = "isActiveParams";


class BaseQualityTest : public CvTest
{
public:
    BaseQualityTest( const char* _algName, const char* _testName, const char* _testFuncs ) :
            CvTest( _testName, _testFuncs ), algName(_algName) {}

protected:
    virtual string getRunParamsFilename() const = 0;
    virtual string getResultsFilename() const = 0;

    virtual void validQualityClear( int datasetIdx ) = 0;
    virtual void calcQualityClear( int datasetIdx ) = 0;
    virtual void validQualityCreate( int datasetIdx ) = 0;
    virtual bool isValidQualityEmpty( int datasetIdx ) const = 0;
    virtual bool isCalcQualityEmpty( int datasetIdx ) const = 0;

    void readAllDatasetsRunParams();
    virtual void readDatasetRunParams( FileNode& fn, int datasetIdx ) = 0;
    void writeAllDatasetsRunParams() const;
    virtual void writeDatasetRunParams( FileStorage& fs, int datasetIdx ) const = 0;
    void setDefaultAllDatasetsRunParams();
    virtual void setDefaultDatasetRunParams( int datasetIdx ) = 0;
    virtual void readDefaultRunParams( FileNode &fn ) {};
    virtual void writeDefaultRunParams( FileStorage &fs ) const {};

    virtual void readResults();
    virtual void readResults( FileNode& fn, int datasetIdx, int caseIdx ) = 0;
    void writeResults() const;
    virtual void writeResults( FileStorage& fs, int datasetIdx, int caseIdx ) const = 0;

    bool readDataset( const string& datasetName, vector<Mat>& Hs, vector<Mat>& imgs );

    virtual void readAlgorithm( ) {};
    virtual void processRunParamsFile () {};
    virtual void runDatasetTest( const vector<Mat> &imgs, const vector<Mat> &Hs, int di, int &progress ) {};
    void run( int );

    virtual void processResults();
    virtual int processResults( int datasetIdx, int caseIdx ) = 0;

    string algName;
    bool isWriteParams, isWriteResults;
};

void BaseQualityTest::readAllDatasetsRunParams()
{
    string filename = getRunParamsFilename();
    FileStorage fs( filename, FileStorage::READ );
    if( !fs.isOpened() )
    {
        isWriteParams = true;
        setDefaultAllDatasetsRunParams();
        ts->printf(CvTS::LOG, "all runParams are default\n");
    }
    else
    {
        isWriteParams = false;
        FileNode topfn = fs.getFirstTopLevelNode();

        FileNode fn = topfn[DEFAULT_PARAMS];
        readDefaultRunParams(fn);

        for( int i = 0; i < DATASETS_COUNT; i++ )
        {
            FileNode fn = topfn[DATASET_NAMES[i]];
            if( fn.empty() )
            {
                ts->printf( CvTS::LOG, "%d-runParams is default\n", i);
                setDefaultDatasetRunParams(i);
            }
            else
                readDatasetRunParams(fn, i);
        }
    }
}

void BaseQualityTest::writeAllDatasetsRunParams() const
{
    string filename = getRunParamsFilename();
    FileStorage fs( filename, FileStorage::WRITE );
    if( fs.isOpened() )
    {
        fs << "run_params" << "{"; // top file node
        fs << DEFAULT_PARAMS << "{";
        writeDefaultRunParams(fs);
        fs << "}";
        for( int i = 0; i < DATASETS_COUNT; i++ )
        {
            fs << DATASET_NAMES[i] << "{";
            writeDatasetRunParams(fs, i);
            fs << "}";
        }
        fs << "}";
    }
    else
        ts->printf(CvTS::LOG, "file %s for writing run params can not be opened\n", filename.c_str() );
}

void BaseQualityTest::setDefaultAllDatasetsRunParams()
{
    for( int i = 0; i < DATASETS_COUNT; i++ )
        setDefaultDatasetRunParams(i);
}

bool BaseQualityTest::readDataset( const string& datasetName, vector<Mat>& Hs, vector<Mat>& imgs )
{
    Hs.resize( TEST_CASE_COUNT );
    imgs.resize( TEST_CASE_COUNT+1 );
    string dirname = string(ts->get_data_path()) + IMAGE_DATASETS_DIR + datasetName + "/";

    for( int i = 0; i < (int)Hs.size(); i++ )
    {
        stringstream filename; filename << "H1to" << i+2 << "p.xml";
        FileStorage fs( dirname + filename.str(), FileStorage::READ );
        if( !fs.isOpened() )
            return false;
        fs.getFirstTopLevelNode() >> Hs[i];
    }

    for( int i = 0; i < (int)imgs.size(); i++ )
    {
        stringstream filename; filename << "img" << i+1 << ".png";
        imgs[i] = imread( dirname + filename.str(), 0 );
        if( imgs[i].empty() )
            return false;
    }
    return true;
}

void BaseQualityTest::readResults()
{
    string filename = getResultsFilename();
    FileStorage fs( filename, FileStorage::READ );
    if( fs.isOpened() )
    {
        isWriteResults = false;
        FileNode topfn = fs.getFirstTopLevelNode();
        for( int di = 0; di < DATASETS_COUNT; di++ )
        {
            FileNode datafn = topfn[DATASET_NAMES[di]];
            if( datafn.empty() )
            {
                validQualityClear(di);
                ts->printf( CvTS::LOG, "results for %s dataset were not read\n",
                            DATASET_NAMES[di].c_str() );
            }
            else
            {
                validQualityCreate(di);
                for( int ci = 0; ci < TEST_CASE_COUNT; ci++ )
                {
                    stringstream ss; ss << "case" << ci;
                    FileNode casefn = datafn[ss.str()];
                    CV_Assert( !casefn.empty() );
                    readResults( casefn , di, ci );
                }
            }
        }
    }
    else
        isWriteResults = true;
}

void BaseQualityTest::writeResults() const
{
    string filename = getResultsFilename();;
    FileStorage fs( filename, FileStorage::WRITE );
    if( fs.isOpened() )
    {
        fs << "results" << "{";
        for( int di = 0; di < DATASETS_COUNT; di++ )
        {
            if( isCalcQualityEmpty(di) )
            {
                ts->printf(CvTS::LOG, "results on %s dataset were not write because of empty\n",
                    DATASET_NAMES[di].c_str());
            }
            else
            {
                fs << DATASET_NAMES[di] << "{";
                for( int ci = 0; ci < TEST_CASE_COUNT; ci++ )
                {
                    stringstream ss; ss << "case" << ci;
                    fs << ss.str() << "{";
                    writeResults( fs, di, ci );
                    fs << "}"; //ss.str()
                }
                fs << "}"; //DATASET_NAMES[di]
            }
        }
        fs << "}"; //results
    }
    else
        ts->printf(CvTS::LOG, "results were not written because file %s can not be opened\n", filename.c_str() );
}

void BaseQualityTest::processResults()
{
    if( isWriteParams )
        writeAllDatasetsRunParams();

    int res = CvTS::OK;
    if( isWriteResults )
        writeResults();
    else
    {
        for( int di = 0; di < DATASETS_COUNT; di++ )
        {
            if( isValidQualityEmpty(di) || isCalcQualityEmpty(di) )
                continue;

            ts->printf(CvTS::LOG, "\nDataset: %s\n", DATASET_NAMES[di].c_str() );

            for( int ci = 0; ci < TEST_CASE_COUNT; ci++ )
            {
                ts->printf(CvTS::LOG, "case%d\n", ci);
                int currRes = processResults( di, ci );
                res = currRes == CvTS::OK ? res : currRes;
            }
        }
    }

    if( res != CvTS::OK )
        ts->printf(CvTS::LOG, "BAD ACCURACY\n");
    ts->set_failed_test_info( res );
}

void BaseQualityTest::run ( int )
{
    readAlgorithm ();
    processRunParamsFile ();
    readResults();

    int notReadDatasets = 0;
    int progress = 0;

    FileStorage runParamsFS( getRunParamsFilename(), FileStorage::READ );
    isWriteParams = (! runParamsFS.isOpened());
    FileNode topfn = runParamsFS.getFirstTopLevelNode();
    FileNode defaultParams = topfn[DEFAULT_PARAMS];
    readDefaultRunParams (defaultParams);

    for(int di = 0; di < DATASETS_COUNT; di++ )
    {
        vector<Mat> imgs, Hs;
        if( !readDataset( DATASET_NAMES[di], Hs, imgs ) )
        {
            calcQualityClear (di);
            ts->printf( CvTS::LOG, "images or homography matrices of dataset named %s can not be read\n",
                        DATASET_NAMES[di].c_str());
            notReadDatasets++;
            continue;
        }

        FileNode fn = topfn[DATASET_NAMES[di]];
        readDatasetRunParams(fn, di);

        runDatasetTest (imgs, Hs, di, progress);
    }
    if( notReadDatasets == DATASETS_COUNT )
    {
        ts->printf(CvTS::LOG, "All datasets were not be read\n");
        ts->set_failed_test_info( CvTS::FAIL_INVALID_TEST_DATA );
    }
    else
        processResults();
    runParamsFS.release();
}



class DetectorQualityTest : public BaseQualityTest
{
public:
    DetectorQualityTest( const char* _detectorName, const char* _testName ) :
            BaseQualityTest( _detectorName, _testName, "quality-of-detector" )
    {
        validQuality.resize(DATASETS_COUNT);
        calcQuality.resize(DATASETS_COUNT);
        isSaveKeypoints.resize(DATASETS_COUNT);
        isActiveParams.resize(DATASETS_COUNT);
    }

protected:
    using BaseQualityTest::readResults;
    using BaseQualityTest::writeResults;
    using BaseQualityTest::processResults;

    virtual string getRunParamsFilename() const;
    virtual string getResultsFilename() const;

    virtual void validQualityClear( int datasetIdx );
    virtual void calcQualityClear( int datasetIdx );
    virtual void validQualityCreate( int datasetIdx );
    virtual bool isValidQualityEmpty( int datasetIdx ) const;
    virtual bool isCalcQualityEmpty( int datasetIdx ) const;

    virtual void readResults( FileNode& fn, int datasetIdx, int caseIdx );
    virtual void writeResults( FileStorage& fs, int datasetIdx, int caseIdx ) const;

    virtual void readDatasetRunParams( FileNode& fn, int datasetIdx );
    virtual void writeDatasetRunParams( FileStorage& fs, int datasetIdx ) const;
    virtual void setDefaultDatasetRunParams( int datasetIdx );
    virtual void readDefaultRunParams( FileNode &fn );
    virtual void writeDefaultRunParams( FileStorage &fs ) const;


    Ptr<FeatureDetector> specificDetector;
    Ptr<FeatureDetector> defaultDetector;
    void openToWriteKeypointsFile( FileStorage& fs, int datasetIdx );

    virtual void readAlgorithm( );
    virtual void processRunParamsFile () {};
    virtual void runDatasetTest( const vector<Mat> &imgs, const vector<Mat> &Hs, int di, int &progress );
    virtual int processResults( int datasetIdx, int caseIdx );

    struct Quality
    {
#ifndef AFFINE_COVARIANT_VERSION
        int repeatingLocationCount;
        float repeatingLocationRltv;
        int repeatingRegionCount;
        float repeatingRegionRltv;
#else
        float repeatability;
        int correspondenceCount;
#endif
    };
    vector<vector<Quality> > validQuality;
    vector<vector<Quality> > calcQuality;
    vector<bool> isSaveKeypoints;
    vector<bool> isActiveParams;
};

string DetectorQualityTest::getRunParamsFilename() const
{
     return string(ts->get_data_path()) + DETECTORS_DIR + algName + PARAMS_POSTFIX;
}

string DetectorQualityTest::getResultsFilename() const
{
    return string(ts->get_data_path()) + DETECTORS_DIR + algName + RES_POSTFIX;
}

void DetectorQualityTest::validQualityClear( int datasetIdx )
{
    validQuality[datasetIdx].clear();
}

void DetectorQualityTest::calcQualityClear( int datasetIdx )
{
    calcQuality[datasetIdx].clear();
}

void DetectorQualityTest::validQualityCreate( int datasetIdx )
{
    validQuality[datasetIdx].resize(TEST_CASE_COUNT);
}

bool DetectorQualityTest::isValidQualityEmpty( int datasetIdx ) const
{
    return validQuality[datasetIdx].empty();
}

bool DetectorQualityTest::isCalcQualityEmpty( int datasetIdx ) const
{
    return calcQuality[datasetIdx].empty();
}

void DetectorQualityTest::readResults( FileNode& fn, int datasetIdx, int caseIdx )
{
#ifndef AFFINE_COVARIANT_VERSION
    validQuality[datasetIdx][caseIdx].repeatingLocationCount = fn[RLC];
    validQuality[datasetIdx][caseIdx].repeatingLocationRltv = fn[RLR];
    validQuality[datasetIdx][caseIdx].repeatingRegionCount = fn[RRC];
    validQuality[datasetIdx][caseIdx].repeatingRegionRltv = fn[RRR];
#else
    validQuality[datasetIdx][caseIdx].repeatability = fn[REPEAT];
    validQuality[datasetIdx][caseIdx].correspondenceCount = fn[CORRESP_COUNT];
#endif
}

void DetectorQualityTest::writeResults( FileStorage& fs, int datasetIdx, int caseIdx ) const
{
#ifndef AFFINE_COVARIANT_VERSION
    fs << RLC << calcQuality[datasetIdx][caseIdx].repeatingLocationCount;
    fs << RLR << calcQuality[datasetIdx][caseIdx].repeatingLocationRltv;
    fs << RRC << calcQuality[datasetIdx][caseIdx].repeatingRegionCount;
    fs << RRR << calcQuality[datasetIdx][caseIdx].repeatingRegionRltv;
#else
    fs << REPEAT << calcQuality[datasetIdx][caseIdx].repeatability;
    fs << CORRESP_COUNT << calcQuality[datasetIdx][caseIdx].correspondenceCount;
#endif
}

void DetectorQualityTest::readDefaultRunParams (FileNode &fn)
{
    if (! fn.empty() )
    {
        defaultDetector->read (fn);
    }
}

void DetectorQualityTest::writeDefaultRunParams (FileStorage &fs) const
{
    defaultDetector->write (fs);
}

void DetectorQualityTest::readDatasetRunParams( FileNode& fn, int datasetIdx )
{
    if (! fn.empty())
    {
        isSaveKeypoints[datasetIdx] = (int)fn["isSaveKeypoints"] != 0;
        isActiveParams[datasetIdx] = (int)fn[IS_ACTIVE_PARAMS] != 0;
    }
    else
    {
        setDefaultDatasetRunParams(datasetIdx);
    }

    if (isActiveParams[datasetIdx] && !fn.empty())
    {
        specificDetector->read (fn);
    }
}

void DetectorQualityTest::writeDatasetRunParams( FileStorage& fs, int datasetIdx ) const
{
    fs << "isSaveKeypoints" << isSaveKeypoints[datasetIdx];
    fs << IS_ACTIVE_PARAMS << isActiveParams[datasetIdx];

//    if (!defaultDetector.empty())
//    {
//        defaultDetector->write (fs);
//    }
}

void DetectorQualityTest::setDefaultDatasetRunParams( int datasetIdx )
{
    isSaveKeypoints[datasetIdx] = false;
    isActiveParams[datasetIdx] = false;
}

void DetectorQualityTest::openToWriteKeypointsFile( FileStorage& fs, int datasetIdx )
{
    string filename = string(ts->get_data_path()) + KEYPOINTS_DIR + algName + "_"+
                      DATASET_NAMES[datasetIdx] + ".xml.gz" ;

    fs.open(filename, FileStorage::WRITE);
    if( !fs.isOpened() )
        ts->printf( CvTS::LOG, "keypoints can not be written in file %s because this file can not be opened\n",
                    filename.c_str());
}

inline void writeKeypoints( FileStorage& fs, const vector<KeyPoint>& keypoints, int imgIdx )
{
    if( fs.isOpened() )
    {
        stringstream imgName; imgName << "img" << imgIdx;
        write( fs, imgName.str(), keypoints );
    }
}

inline void readKeypoints( FileStorage& fs, vector<KeyPoint>& keypoints, int imgIdx )
{
    assert( fs.isOpened() );
    stringstream imgName; imgName << "img" << imgIdx;
    read( fs[imgName.str()], keypoints);
}

void DetectorQualityTest::readAlgorithm ()
{
    //TODO: use Factory Register when it will be implemented
    if (! algName.compare ("fast"))
    {
        defaultDetector = new FastFeatureDetector();
        specificDetector = new FastFeatureDetector();
    }
    else if (! algName.compare ("mser"))
    {
        defaultDetector = new MserFeatureDetector();
        specificDetector = new MserFeatureDetector();
    }
    else if (! algName.compare ("star"))
    {
        defaultDetector = new StarFeatureDetector();
        specificDetector = new StarFeatureDetector();
    }
    else if (! algName.compare ("sift"))
    {
        defaultDetector = new SiftFeatureDetector();
        specificDetector = new SiftFeatureDetector();
    }
    else if (! algName.compare ("surf"))
    {
        defaultDetector = new SurfFeatureDetector();
        specificDetector = new SurfFeatureDetector();
    }
    else
    {
        int maxCorners = 1500;
        double qualityLevel = 0.01;
        double minDistance = 2.0;
        int blockSize=3;

        if (! algName.compare ("gftt"))
        {
            bool useHarrisDetector = false;
            defaultDetector = new GoodFeaturesToTrackDetector (maxCorners, qualityLevel, minDistance, blockSize, useHarrisDetector);
            specificDetector = new GoodFeaturesToTrackDetector (maxCorners, qualityLevel, minDistance, blockSize, useHarrisDetector);
        }
        else if (! algName.compare ("harris"))
        {
            bool useHarrisDetector = true;
            defaultDetector = new GoodFeaturesToTrackDetector (maxCorners, qualityLevel, minDistance, blockSize, useHarrisDetector);
            specificDetector = new GoodFeaturesToTrackDetector (maxCorners, qualityLevel, minDistance, blockSize, useHarrisDetector);
        }
        else
        {
            ts->printf(CvTS::LOG, "Algorithm can not be read\n");
            ts->set_failed_test_info( CvTS::FAIL_GENERIC);
        }
    }
}

void DetectorQualityTest::runDatasetTest (const vector<Mat> &imgs, const vector<Mat> &Hs, int di, int &progress)
{
    Ptr<FeatureDetector> detector = isActiveParams[di] ? specificDetector : defaultDetector;
    FileStorage keypontsFS;
    if( isSaveKeypoints[di] )
        openToWriteKeypointsFile( keypontsFS, di );

    calcQuality[di].resize(TEST_CASE_COUNT);

    vector<KeyPoint> keypoints1; vector<EllipticKeyPoint> ekeypoints1;

    detector->detect( imgs[0], keypoints1 );
    writeKeypoints( keypontsFS, keypoints1, 0);
    transformToEllipticKeyPoints( keypoints1, ekeypoints1 );
    int progressCount = DATASETS_COUNT*TEST_CASE_COUNT;
    for( int ci = 0; ci < TEST_CASE_COUNT; ci++ )
    {
        progress = update_progress( progress, di*TEST_CASE_COUNT + ci, progressCount, 0 );
        vector<KeyPoint> keypoints2;
        detector->detect( imgs[ci+1], keypoints2 );
        writeKeypoints( keypontsFS, keypoints2, ci+1);
#ifndef AFFINE_COVARIANT_VERSION
        evaluateScaleInvDetectors( imgs[0], imgs[ci+1], Hs[ci], keypoints1, keypoints2,
            calcQuality[di][ci].repeatingLocationCount, calcQuality[di][ci].repeatingLocationRltv,
            calcQuality[di][ci].repeatingRegionCount, calcQuality[di][ci].repeatingRegionRltv );
#else
        vector<EllipticKeyPoint> ekeypoints2;
        transformToEllipticKeyPoints( keypoints2, ekeypoints2 );
        evaluateDetectors( ekeypoints1, ekeypoints2, imgs[0], imgs[ci], Hs[ci],
                           calcQuality[di][ci].repeatability, calcQuality[di][ci].correspondenceCount );
#endif
    }
}

void testLog( CvTS* ts, bool isBadAccuracy )
{
    if( isBadAccuracy )
        ts->printf(CvTS::LOG, " bad accuracy\n");
    else
        ts->printf(CvTS::LOG, "\n");
}

int DetectorQualityTest::processResults( int datasetIdx, int caseIdx )
{
    int res = CvTS::OK;

    Quality valid = validQuality[datasetIdx][caseIdx], calc = calcQuality[datasetIdx][caseIdx];

    bool isBadAccuracy;
    int countEps = 1;
    const float rltvEps = 0.001;
#ifndef AFFINE_COVARIANT_VERSION
    ts->printf(CvTS::LOG, "%s: calc=%d, valid=%d", RLC.c_str(), calc.repeatingLocationCount, valid.repeatingLocationCount );
    isBadAccuracy = valid.repeatingLocationCount - calc.repeatingLocationCount > countEps;
    testLog( ts, isBadAccuracy );
    res = isBadAccuracy ? CvTS::FAIL_BAD_ACCURACY : res;

    ts->printf(CvTS::LOG, "%s: calc=%f, valid=%f", RLR.c_str(), calc.repeatingLocationRltv, valid.repeatingLocationRltv );
    isBadAccuracy = valid.repeatingLocationRltv - calc.repeatingLocationRltv > rltvEps;
    testLog( ts, isBadAccuracy );
    res = isBadAccuracy ? CvTS::FAIL_BAD_ACCURACY : res;

    ts->printf(CvTS::LOG, "%s: calc=%d, valid=%d", RRC.c_str(), calc.repeatingRegionCount, valid.repeatingRegionCount );
    isBadAccuracy = valid.repeatingRegionCount - calc.repeatingRegionCount > countEps;
    testLog( ts, isBadAccuracy );
    res = isBadAccuracy ? CvTS::FAIL_BAD_ACCURACY : res;

    ts->printf(CvTS::LOG, "%s: calc=%f, valid=%f", RRR.c_str(), calc.repeatingRegionRltv, valid.repeatingRegionRltv );
    isBadAccuracy = valid.repeatingRegionRltv - calc.repeatingRegionRltv > rltvEps;
    testLog( ts, isBadAccuracy );
    res = isBadAccuracy ? CvTS::FAIL_BAD_ACCURACY : res;
#else
    ts->printf(CvTS::LOG, "%s: calc=%f, valid=%f", REPEAT.c_str(), calc.repeatability, valid.repeatability );
    isBadAccuracy = valid.repeatability - calc.repeatability > rltvEps;
    testLog( ts, isBadAccuracy );
    res = isBadAccuracy ? CvTS::FAIL_BAD_ACCURACY : res;

    ts->printf(CvTS::LOG, "%s: calc=%d, valid=%d", CORRESP_COUNT.c_str(), calc.correspondenceCount, valid.correspondenceCount );
    isBadAccuracy = valid.correspondenceCount - calc.correspondenceCount > countEps;
    testLog( ts, isBadAccuracy );
    res = isBadAccuracy ? CvTS::FAIL_BAD_ACCURACY : res;
#endif
    return res;
}

DetectorQualityTest fastDetectorQuality = DetectorQualityTest( "fast", "quality-detector-fast" );
DetectorQualityTest gfttDetectorQuality = DetectorQualityTest( "gftt", "quality-detector-gftt" );
DetectorQualityTest harrisDetectorQuality = DetectorQualityTest( "harris", "quality-detector-harris" );
DetectorQualityTest mserDetectorQuality = DetectorQualityTest( "mser", "quality-detector-mser" );
DetectorQualityTest starDetectorQuality = DetectorQualityTest( "star", "quality-detector-star" );
DetectorQualityTest siftDetectorQuality = DetectorQualityTest( "sift", "quality-detector-sift" );
DetectorQualityTest surfDetectorQuality = DetectorQualityTest( "surf", "quality-detector-surf" );

/****************************************************************************************\
*                                  Descriptors evaluation                                 *
\****************************************************************************************/

const string RECALL = "recall";
const string PRECISION = "precision";

const string KEYPOINTS_FILENAME = "keypointsFilename";
const string PROJECT_KEYPOINTS_FROM_1IMAGE = "projectKeypointsFrom1Image";
const string MATCH_FILTER = "matchFilter";
const string RUN_PARAMS_IS_IDENTICAL = "runParamsIsIdentical";

const string ONE_WAY_TRAIN_DIR = "detectors_descriptors_evaluation/one_way_train_images/";
const string ONE_WAY_IMAGES_LIST = "one_way_train_images.txt";

class DescriptorQualityTest : public BaseQualityTest
{
public:
    enum{ NO_MATCH_FILTER = 0 };
    DescriptorQualityTest( const char* _descriptorName, const char* _testName ) :
            BaseQualityTest( _descriptorName, _testName, "quality-of-descriptor" )
    {
        validQuality.resize(DATASETS_COUNT);
        calcQuality.resize(DATASETS_COUNT);
        commRunParams.resize(DATASETS_COUNT);
    }

protected:
    using BaseQualityTest::readResults;
    using BaseQualityTest::writeResults;
    using BaseQualityTest::processResults;

    virtual string getRunParamsFilename() const;
    virtual string getResultsFilename() const;

    virtual void validQualityClear( int datasetIdx );
    virtual void calcQualityClear( int datasetIdx );
    virtual void validQualityCreate( int datasetIdx );
    virtual bool isValidQualityEmpty( int datasetIdx ) const;
    virtual bool isCalcQualityEmpty( int datasetIdx ) const;

    virtual void readResults( FileNode& fn, int datasetIdx, int caseIdx );
    virtual void writeResults( FileStorage& fs, int datasetIdx, int caseIdx ) const;

    virtual void readDatasetRunParams( FileNode& fn, int datasetIdx ); //
    virtual void writeDatasetRunParams( FileStorage& fs, int datasetIdx ) const;
    virtual void setDefaultDatasetRunParams( int datasetIdx );
    virtual void readDefaultRunParams( FileNode &fn );
    virtual void writeDefaultRunParams( FileStorage &fs ) const;

    virtual void readAlgorithm( );
    virtual void processRunParamsFile () {};
    virtual void runDatasetTest( const vector<Mat> &imgs, const vector<Mat> &Hs, int di, int &progress );

    virtual int processResults( int datasetIdx, int caseIdx );

    struct Quality
    {
        float recall;
        float precision;
    };
    vector<vector<Quality> > validQuality;
    vector<vector<Quality> > calcQuality;

    struct CommonRunParams
    {
        string keypontsFilename;
        bool projectKeypointsFrom1Image;
        int matchFilter; // not used now
        bool isActiveParams;
    };
    vector<CommonRunParams> commRunParams;

    Ptr<GenericDescriptorMatch> specificDescMatch;
    Ptr<GenericDescriptorMatch> defaultDescMatch;
};

string DescriptorQualityTest::getRunParamsFilename() const
{
    return string(ts->get_data_path()) + DESCRIPTORS_DIR + algName + PARAMS_POSTFIX;
}

string DescriptorQualityTest::getResultsFilename() const
{
    return string(ts->get_data_path()) + DESCRIPTORS_DIR + algName + RES_POSTFIX;
}

void DescriptorQualityTest::validQualityClear( int datasetIdx )
{
    validQuality[datasetIdx].clear();
}

void DescriptorQualityTest::calcQualityClear( int datasetIdx )
{
    calcQuality[datasetIdx].clear();
}

void DescriptorQualityTest::validQualityCreate( int datasetIdx )
{
    validQuality[datasetIdx].resize(TEST_CASE_COUNT);
}

bool DescriptorQualityTest::isValidQualityEmpty( int datasetIdx ) const
{
    return validQuality[datasetIdx].empty();
}

bool DescriptorQualityTest::isCalcQualityEmpty( int datasetIdx ) const
{
    return calcQuality[datasetIdx].empty();
}

void DescriptorQualityTest::readResults( FileNode& fn, int datasetIdx, int caseIdx )
{
    validQuality[datasetIdx][caseIdx].recall = fn[RECALL];
    validQuality[datasetIdx][caseIdx].precision = fn[PRECISION];
}

void DescriptorQualityTest::writeResults( FileStorage& fs, int datasetIdx, int caseIdx ) const
{
    fs << RECALL << calcQuality[datasetIdx][caseIdx].recall;
    fs << PRECISION << calcQuality[datasetIdx][caseIdx].precision;
}

void DescriptorQualityTest::readDefaultRunParams (FileNode &fn)
{
    if (! fn.empty() )
    {
        defaultDescMatch->read (fn);
    }
}

void DescriptorQualityTest::writeDefaultRunParams (FileStorage &fs) const
{
    defaultDescMatch->write (fs);
}

void DescriptorQualityTest::readDatasetRunParams( FileNode& fn, int datasetIdx )
{
    if (! fn.empty())
    {
        commRunParams[datasetIdx].keypontsFilename = (string)fn[KEYPOINTS_FILENAME];
        commRunParams[datasetIdx].projectKeypointsFrom1Image = (int)fn[PROJECT_KEYPOINTS_FROM_1IMAGE] != 0;
        commRunParams[datasetIdx].matchFilter = (int)fn[MATCH_FILTER];
        commRunParams[datasetIdx].isActiveParams = (int)fn[IS_ACTIVE_PARAMS];
    }
    else
    {
        setDefaultDatasetRunParams(datasetIdx);
    }

    if (commRunParams[datasetIdx].isActiveParams && !fn.empty())
    {
        specificDescMatch->read (fn);
    }
}

void DescriptorQualityTest::writeDatasetRunParams( FileStorage& fs, int datasetIdx ) const
{
    fs << KEYPOINTS_FILENAME << commRunParams[datasetIdx].keypontsFilename;
    fs << PROJECT_KEYPOINTS_FROM_1IMAGE << commRunParams[datasetIdx].projectKeypointsFrom1Image;
    fs << MATCH_FILTER << commRunParams[datasetIdx].matchFilter;
    fs << IS_ACTIVE_PARAMS << commRunParams[datasetIdx].isActiveParams;

//    if (!defaultDescMatch.empty())
//    {
//        defaultDescMatch->write (fs);
//    }
}

void DescriptorQualityTest::setDefaultDatasetRunParams( int datasetIdx )
{
    commRunParams[datasetIdx].keypontsFilename = "surf_" + DATASET_NAMES[datasetIdx] + ".xml.gz";
    commRunParams[datasetIdx].projectKeypointsFrom1Image = true;
    commRunParams[datasetIdx].matchFilter = NO_MATCH_FILTER;
    commRunParams[datasetIdx].isActiveParams = false;
}


void DescriptorQualityTest::readAlgorithm( )
{
    //TODO: use Factory Register when it will be implemented
    if (! algName.compare ("sift"))
    {
        SiftDescriptorExtractor extractor;
        BruteForceMatcher<L2<float> > matcher;
        defaultDescMatch = new VectorDescriptorMatch<SiftDescriptorExtractor, BruteForceMatcher<L2<float> > >(extractor, matcher);
        specificDescMatch = new VectorDescriptorMatch<SiftDescriptorExtractor, BruteForceMatcher<L2<float> > >(extractor, matcher);
    }
    else if (! algName.compare ("surf"))
    {
        SurfDescriptorExtractor extractor;
        BruteForceMatcher<L2<float> > matcher;
        defaultDescMatch = new VectorDescriptorMatch<SurfDescriptorExtractor, BruteForceMatcher<L2<float> > >(extractor, matcher);
        specificDescMatch = new VectorDescriptorMatch<SurfDescriptorExtractor, BruteForceMatcher<L2<float> > >(extractor, matcher);
    }
    else if (! algName.compare ("one_way"))
    {
        defaultDescMatch = new OneWayDescriptorMatch ();
        specificDescMatch = new OneWayDescriptorMatch ();
    }
    else
    {
        ts->printf(CvTS::LOG, "Algorithm can not be read\n");
        ts->set_failed_test_info( CvTS::FAIL_GENERIC);
    }
}

void DescriptorQualityTest::runDatasetTest (const vector<Mat> &imgs, const vector<Mat> &Hs, int di, int &progress)
{
    FileStorage keypontsFS( string(ts->get_data_path()) + KEYPOINTS_DIR + commRunParams[di].keypontsFilename,
                                    FileStorage::READ );
    if( !keypontsFS.isOpened())
    {
       calcQuality[di].clear();
       ts->printf( CvTS::LOG, "keypoints from file %s can not be read\n", commRunParams[di].keypontsFilename.c_str() );
       return;
    }

    Ptr<GenericDescriptorMatch> descMatch = commRunParams[di].isActiveParams ? specificDescMatch : defaultDescMatch;
    calcQuality[di].resize(TEST_CASE_COUNT);

    vector<KeyPoint> keypoints1; vector<EllipticKeyPoint> ekeypoints1;
    readKeypoints( keypontsFS, keypoints1, 0);
    transformToEllipticKeyPoints( keypoints1, ekeypoints1 );

    int progressCount = DATASETS_COUNT*TEST_CASE_COUNT;
    for( int ci = 0; ci < TEST_CASE_COUNT; ci++ )
    {
        progress = update_progress( progress, di*TEST_CASE_COUNT + ci, progressCount, 0 );

        vector<KeyPoint> keypoints2;
        vector<EllipticKeyPoint> ekeypoints2;
        if( commRunParams[di].projectKeypointsFrom1Image )
        {
            // TODO need to test function calcKeyPointProjections
            calcKeyPointProjections( keypoints1, Hs[ci], keypoints2 );
            filterKeyPointsByImageSize( keypoints2,  imgs[ci+1].size() );
        }
        else
            readKeypoints( keypontsFS, keypoints2, ci+1 );
        transformToEllipticKeyPoints( keypoints2, ekeypoints2 );
        descMatch->add( imgs[ci+1], keypoints2 );
        vector<int> matches1to2;
        descMatch->match( imgs[0], keypoints1, matches1to2 );
        // TODO if( commRunParams[di].matchFilter )
        int correctMatchCount, falseMatchCount, correspCount;
        evaluateDescriptors( ekeypoints1, ekeypoints2, matches1to2, imgs[0], imgs[ci+1], Hs[ci],
                             correctMatchCount, falseMatchCount, correspCount );
        calcQuality[di][ci].recall = recall( correctMatchCount, correspCount );
        calcQuality[di][ci].precision = precision( correctMatchCount, falseMatchCount );
        descMatch->clear ();
    }
}

int DescriptorQualityTest::processResults( int datasetIdx, int caseIdx )
{
    int res = CvTS::OK;
    Quality valid = validQuality[datasetIdx][caseIdx], calc = calcQuality[datasetIdx][caseIdx];

    bool isBadAccuracy;
    const float rltvEps = 0.001;
    ts->printf(CvTS::LOG, "%s: calc=%f, valid=%f", RECALL.c_str(), calc.recall, valid.recall );
    isBadAccuracy = valid.recall - calc.recall > rltvEps;
    testLog( ts, isBadAccuracy );
    res = isBadAccuracy ? CvTS::FAIL_BAD_ACCURACY : res;

    ts->printf(CvTS::LOG, "%s: calc=%f, valid=%f", PRECISION.c_str(), calc.precision, valid.precision );
    isBadAccuracy = valid.precision - calc.precision > rltvEps;
    testLog( ts, isBadAccuracy );
    res = isBadAccuracy ? CvTS::FAIL_BAD_ACCURACY : res;

    return res;
}

DescriptorQualityTest siftDescriptorQuality = DescriptorQualityTest( "sift", "quality-descriptor-sift" );
DescriptorQualityTest surfDescriptorQuality = DescriptorQualityTest( "surf", "quality-descriptor-surf" );

//--------------------------------- One Way descriptor test --------------------------------------------
class OneWayDescriptorQualityTest : public DescriptorQualityTest
{
public:
    OneWayDescriptorQualityTest() :
        DescriptorQualityTest("one_way", "quality-descriptor-one-way")
    {
    }
protected:
    virtual void processRunParamsFile ();
};

void OneWayDescriptorQualityTest::processRunParamsFile ()
{
    string filename = getRunParamsFilename();
    FileStorage fs = FileStorage (filename, FileStorage::READ);
    FileNode fn = fs.getFirstTopLevelNode();
    fn = fn[DEFAULT_PARAMS];

    OneWayDescriptorMatch *match = new OneWayDescriptorMatch ();
    match->read (fn);

    string pcaFilename = string(ts->get_data_path()) + (string)fn["pcaFilename"];
    string trainPath = string(ts->get_data_path()) + (string)fn["trainPath"];
    string trainImagesList = (string)fn["trainImagesList"];

    if (trainImagesList.length () == 0 )
    {
        return;
        fs.release ();
    }
    fs.release ();

    readAllDatasetsRunParams();

    OneWayDescriptorBase *base = new OneWayDescriptorBase(match->getParams().patchSize, match->getParams().poseCount, pcaFilename,
                                               trainPath, trainImagesList);

    match->initialize( match->getParams(), base );

    defaultDescMatch = match;
    writeAllDatasetsRunParams();
}


OneWayDescriptorQualityTest oneWayDescriptorQuality;
