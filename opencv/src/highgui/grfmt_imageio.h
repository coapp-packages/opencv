/*
 *  grfmt_imageio.h
 *  
 *
 *  Created by Morgan Conbere on 5/17/07.
 *
 */

#ifndef _GRFMT_IMAGEIO_H_
#define _GRFMT_IMAGEIO_H_

#ifdef HAVE_IMAGEIO

#include "grfmt_base.h"
#include <ApplicationServices/ApplicationServices.h>

namespace cv
{

class ImageIODecoder : public BaseImageDecoder
{
public:
    
    ImageIODecoder();
    ~ImageIODecoder();
    
    bool  readData( Mat& img );
    bool  readHeader();
    void  close();

    ImageDecoder newDecoder() const;

protected:
    
    CGImageRef imageRef;
};

class ImageIOEncoder : public BaseImageEncoder
{
public:
    ImageIOEncoder();
    ~ImageIOEncoder();

    bool  write( const String& filename,
        const Mat& img, const Vector<int>& params );

    ImageEncoder newEncoder() const;
};

#endif/*HAVE_IMAGEIO*/

#endif/*_GRFMT_IMAGEIO_H_*/
