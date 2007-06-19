/*
 *  grfmt_imageio.cpp
 *  
 *
 *  Created by Morgan Conbere on 5/17/07.
 *
 */

#include "_highgui.h"

#ifdef HAVE_IMAGEIO

#include "grfmt_imageio.h"


// ImageIO filter factory

GrFmtImageIO::GrFmtImageIO()
{
    m_sign_len = 0;
    m_signature = NULL;
    m_description = "Apple ImageIO (*.bmp;*.dib;*.exr;*.jpeg;*.jpg;*.jpe;*.jp2;*.pdf;*.png;*.tiff;*.tif)";
}


GrFmtImageIO::~GrFmtImageIO()
{
}


bool  GrFmtImageIO::CheckFile( const char* filename )
{
    if( !filename ) return false;
    
    // If a CFImageRef can be retrieved from an image file, it is 
    // readable by ImageIO.  Effectively this is using ImageIO
    // to check the signatures and determine the file format for us.
    CFURLRef imageURLRef = CFURLCreateFromFileSystemRepresentation( NULL,
                                                                    (const UInt8*)filename,
                                                                    strlen( filename ),
                                                                    false );
    if( !imageURLRef ) return false;
    
    CGImageSourceRef sourceRef = CGImageSourceCreateWithURL( imageURLRef, NULL );
    CFRelease( imageURLRef );
    if( !sourceRef ) return false;
    
    CGImageRef imageRef = CGImageSourceCreateImageAtIndex( sourceRef, 0, NULL );
    CFRelease( sourceRef );
    if( !imageRef ) return false;
    
    return true;
}


GrFmtReader* GrFmtImageIO::NewReader( const char* filename )
{
    return new GrFmtImageIOReader( filename );
}


GrFmtWriter* GrFmtImageIO::NewWriter( const char* filename )
{
    return new GrFmtImageIOWriter( filename );
}


/////////////////////// GrFmtImageIOReader ///////////////////

GrFmtImageIOReader::GrFmtImageIOReader( const char* filename ) : GrFmtReader( filename )
{
    // Nothing to do here
}


GrFmtImageIOReader::~GrFmtImageIOReader()
{
    Close();
}


void  GrFmtImageIOReader::Close()
{
    CGImageRelease( imageRef );
    
    GrFmtReader::Close();
}


bool  GrFmtImageIOReader::ReadHeader()
{
    CFURLRef         imageURLRef;
    CGImageSourceRef sourceRef;
    imageRef = NULL;
    
    imageURLRef = CFURLCreateFromFileSystemRepresentation( NULL,
                                                           (const UInt8*)m_filename,
                                                           strlen(m_filename),
                                                           false );
    
    sourceRef = CGImageSourceCreateWithURL( imageURLRef, NULL );
    CFRelease( imageURLRef );
    if ( !sourceRef )
        return false;
    
    imageRef = CGImageSourceCreateImageAtIndex( sourceRef, 0, NULL );
    CFRelease( sourceRef );
    if( !imageRef )
        return false;
    
    m_width = CGImageGetWidth( imageRef );
    m_height = CGImageGetHeight( imageRef );
    
    CGColorSpaceRef colorSpace = CGImageGetColorSpace( imageRef );
    if( !colorSpace )
        return false;
    
    m_iscolor = ( CGColorSpaceGetNumberOfComponents( colorSpace ) > 1 );
    
    CFRelease( colorSpace );
    
    return true;
}


bool  GrFmtImageIOReader::ReadData( uchar* data, int step, int color )
{
    int bpp; // Bytes per pixel
    
    // Set color to either CV_IMAGE_LOAD_COLOR or CV_IMAGE_LOAD_GRAYSCALE if unchanged
    color = color > 0 || ( m_iscolor && color < 0 );
    
    // Get Height, Width, and color information
    if( !ReadHeader() )
        return false;
    
    CGContextRef     context = NULL; // The bitmap context
    CGColorSpaceRef  colorSpace = NULL;
    uchar*           bitmap = NULL;
    CGImageAlphaInfo alphaInfo;
    
    // CoreGraphics will take care of converting to grayscale and back as long as the 
    // appropriate colorspace is set
    if( color == CV_LOAD_IMAGE_GRAYSCALE )
    {
        colorSpace = CGColorSpaceCreateDeviceGray();
        bpp = 1;
        alphaInfo = kCGImageAlphaNone;
    }
    else if( color == CV_LOAD_IMAGE_COLOR )
    {
        colorSpace = CGColorSpaceCreateDeviceRGB();
        bpp = 4; /* CG only has 8 and 32 bit color spaces, so we waste a byte */
        alphaInfo = kCGImageAlphaNoneSkipLast;
    }
    if( !colorSpace )
        return false;
    
    bitmap = (uchar*)malloc( bpp * m_height * m_width );
    if( !bitmap )
    {
        CGColorSpaceRelease( colorSpace );
        return false;
    }
    
    context = CGBitmapContextCreate( (void *)bitmap,
                                     m_width,        /* width */
                                     m_height,       /* height */
                                     m_bit_depth,    /* bit depth */
                                     bpp * m_width,  /* bytes per row */ 
                                     colorSpace,     /* color space */
                                     alphaInfo);
    
    CGColorSpaceRelease( colorSpace );
    if( !context )
    {
        free( bitmap );
        return false;
    }
    
    // Copy the image data into the bitmap region
    CGRect rect = {{0,0},{m_width,m_height}};
    CGContextDrawImage( context, rect, imageRef );
    
    uchar* bitdata = (uchar*)CGBitmapContextGetData( context );
    if( !bitdata )
    {
        free( bitmap);
        CGContextRelease( context );
        return false;
    }
    
    // Move the bitmap (in RGB) into data (in BGR)
    int dataIndex = 0;
    int bitmapIndex = 0;
    
    // We make the assumption that the step is the number of colors * the width
    assert( ( color ? 3 : 1 )*m_width == step );
    
    if( color == CV_LOAD_IMAGE_COLOR ) {
        for( int i = 0; i < m_width * m_height; ++i) {
            // Blue channel
            data[dataIndex + 0] = bitdata[bitmapIndex + 2];
            // Green channel
            data[dataIndex + 1] = bitdata[bitmapIndex + 1];
            // Red channel
            data[dataIndex + 2] = bitdata[bitmapIndex + 0];
            
            dataIndex += 3;
            bitmapIndex += bpp;
        }
    }
    else if( color == CV_LOAD_IMAGE_GRAYSCALE )
    {
        // the bitmap representation is exactly what we want in data
        memcpy( data, bitmap, m_width*m_height );
    }
    
    free( bitmap );
    CGContextRelease( context );
    return true;
}


/////////////////////// GrFmtImageIOWriter ///////////////////

GrFmtImageIOWriter::GrFmtImageIOWriter( const char* filename ) : GrFmtWriter( filename )
{
    // Nothing to do here
}


GrFmtImageIOWriter::~GrFmtImageIOWriter()
{
    // Nothing to do here
}


static
CFStringRef  FilenameToUTI( const char* filename )
{
    const char* ext = filename;
    for(;;)
    {
        const char* temp = strchr( ext + 1, '.' );
        if( !temp ) break;
        ext = temp;
    }
    
    CFStringRef imageUTI = NULL;
    
    if( !strcmp(ext, ".bmp") || !strcmp(ext, ".dib") )
        imageUTI = CFSTR( "com.microsoft.bmp" );
    else if( !strcmp(ext, ".exr") )
        imageUTI = CFSTR( "com.ilm.openexr-image" );
    else if( !strcmp(ext, ".jpeg") || !strcmp(ext, ".jpg") || !strcmp(ext, ".jpe") )
        imageUTI = CFSTR( "public.jpeg" );
    else if( !strcmp(ext, ".jp2") )
        imageUTI = CFSTR( "public.jpeg-2000" );
    else if( !strcmp(ext, ".pdf") )
        imageUTI = CFSTR( "com.adobe.pdf" );
    else if( !strcmp(ext, ".png") )
        imageUTI = CFSTR( "public.png" );
    else if( !strcmp(ext, ".tiff") || !strcmp(ext, ".tif") )
        imageUTI = CFSTR( "public.tiff" );
    
    return imageUTI;
}


bool  GrFmtImageIOWriter::WriteImage( const uchar* data, int step,
                                      int width, int height, int /*depth*/, int _channels )
{
    // Determine the appropriate UTI based on the filename extension
    CFStringRef imageUTI = FilenameToUTI( m_filename );
    
    // Determine the Bytes Per Pixel
    int bpp = (_channels == 1) ? 1 : 4;
    
    // Write the data into a bitmap context
    CGContextRef context;
    CGColorSpaceRef colorSpace;
    uchar* bitmapData = NULL;
    
    if( bpp == 1 )
        colorSpace = CGColorSpaceCreateWithName( kCGColorSpaceGenericGray );
    else if( bpp == 4 )
        colorSpace = CGColorSpaceCreateWithName( kCGColorSpaceGenericRGB );
    if( !colorSpace )
        return false;
    
    bitmapData = (uchar*)malloc( bpp * height * width );
    if( !bitmapData )
    {
        CGColorSpaceRelease( colorSpace );
        return false;
    }
    
    context = CGBitmapContextCreate( bitmapData,
                                     width,
                                     height,
                                     8,
                                     bpp * width,
                                     colorSpace,
                                     (bpp == 1) ? kCGImageAlphaNone :
                                     kCGImageAlphaNoneSkipLast );
    CGColorSpaceRelease( colorSpace );
    if( !context )
    {
        free( bitmapData );
        return false;
    }
    
    // Copy pixel information from data into bitmapData
    if( bpp == 4 )
    {
        int dataIndex = 0;
        int bitmapIndex = 0;
        for( int i = 0; i < width * height; ++i)
        {
            // Blue channel
            bitmapData[bitmapIndex + 2] = data[dataIndex + 0];
            // Green channel
            bitmapData[bitmapIndex + 1] = data[dataIndex + 1];
            // Red channel
            bitmapData[bitmapIndex + 0] = data[dataIndex + 2];
            
            dataIndex += 3;
            bitmapIndex += bpp;
        }
    }
    else if( bpp == 1 )
    {
        // the bitmap representation is exactly what we want in data
        memcpy( bitmapData, data, width * height );
    }
    
    // Turn the bitmap context into an imageRef
    CGImageRef imageRef = CGBitmapContextCreateImage( context );
    CGContextRelease( context );
    if( !imageRef )
    {
        free( bitmapData );
        return false;
    }
    
    // Write the imageRef to a file based on the UTI
    CFURLRef imageURLRef = CFURLCreateFromFileSystemRepresentation( NULL,
                                                                    (const UInt8*)m_filename,
                                                                    strlen(m_filename),
                                                                    false );
    if( !imageURLRef )
    {
        CGImageRelease( imageRef );
        free( bitmapData );
        return false;
    }
    
    CGImageDestinationRef destRef = CGImageDestinationCreateWithURL( imageURLRef,
                                                                     imageUTI, 
                                                                     1,
                                                                     NULL);        
    CFRelease( imageURLRef );
    if( !destRef )
    {
        CGImageRelease( imageRef );
        free( bitmapData );
        return false;
    }
    
    CGImageDestinationAddImage(destRef, imageRef, NULL);
    if( CGImageDestinationFinalize(destRef) )
        return false;
    
    CFRelease( destRef );
    CGImageRelease( imageRef );    
    free( bitmapData );
    
    return true;
}

#endif /* HAVE_IMAGEIO */
