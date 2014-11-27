#ifdef __GDCM__

#include <vglGdcmIo.h>
#include <vglContext.h>
#include <gdcmImageReader.h>
#include <gdcmImage.h>
#include <gdcmWriter.h>
#include <gdcmAttribute.h>
#include <gdcmImageWriter.h>
#include <gdcmImageChangeTransferSyntax.h>
#include "gdcmImageChangeTransferSyntax.h"
#include "gdcmImageChangePhotometricInterpretation.h"
#include "gdcmPhotometricInterpretation.h"

#include <iostream>
#include <fstream>

/** \brief Convert depth from dcm's format to vgl's format.
  */

int convertDepthGdcmToVgl(int dcmDepth)
{
  if(dcmDepth <= 8)
     return IPL_DEPTH_8U;
  else if(dcmDepth <= 16)
     return IPL_DEPTH_16U;
  else if(dcmDepth <= 32)
     return IPL_DEPTH_32S;
}

/** \brief Convert depth from vlg's format to dcm's format.
  */

int convertDepthVglToDcm(int vglDepth)
{
  if(vglDepth == IPL_DEPTH_8U)
     return 8;
  else
     if(vglDepth <= IPL_DEPTH_16U)
       return 16;
     else
       if(vglDepth <= IPL_DEPTH_32S)
	 return 32;
}

/** \brief Convert ybr array with 3 bytes to rgb array with 3 bytes.
  */

inline void YBR2RGB(unsigned char rgb[3], unsigned char ybr[3])
{
    const double Y  = ybr[0];
    const double Cb = ybr[1] - 128.0;
    const double Cr = ybr[2] - 128.0;
    const double r = Y                 + 1.402    * Cr;
    const double g = Y - 0.344136 * Cb - 0.714136 * Cr;
    const double b = Y + 1.772    * Cb;
    double R = r < 0 ? 0 : r;
    R = R > 255 ? 255 : R;
    double G = g < 0 ? 0 : g;
    G = G > 255 ? 255 : G;
    double B = b < 0 ? 0 : b;
    B = B > 255 ? 255 : B;
    rgb[0] = R;
    rgb[1] = G;
    rgb[2] = B;
}

/** Function for create DICOM Header with GDCM library
  */

int vglCreateHeaderGdcm(VglImage* imagevgl, gdcm::Image* img)
{ 
  //img = reader.GetImage();
    
  img->SetNumberOfDimensions(3); 
  gdcm::PhotometricInterpretation pi;
  int samplesPerPixel = imagevgl->nChannels;
  int bits = convertDepthVglToDcm(imagevgl->depth);
  
  if(imagevgl->nChannels == 3)
    pi = gdcm::PhotometricInterpretation::RGB;
  else
    pi = gdcm::PhotometricInterpretation:: MONOCHROME2;
  img->SetPhotometricInterpretation(pi);

  /* Reference local in pixelFormat parameters
     SamplesPerPixel;     position 1
     BitsAllocated;       position 2
     BitsStored;          position 3
     HighBit;             position 4
     PixelRepresentation; position 5
  */
  gdcm::PixelFormat pixelFormat(samplesPerPixel, bits, bits, bits-1,0);
  img->SetPixelFormat(pixelFormat);
  
  int dim[3] = {};
  dim[0] = imagevgl->shape[VGL_WIDTH];
  dim[1] = imagevgl->shape[VGL_HEIGHT];
  dim[2] = imagevgl->shape[VGL_LENGTH];
  img->SetNumberOfDimensions(3);
  
  img->SetDimension(0, dim[0]); 
  img->SetDimension(1, dim[1]);
  img->SetDimension(2, dim[2]);
  
  int dcmDepth = convertDepthVglToDcm(imagevgl->depth);
  int pixelsPerFrame = imagevgl->shape[VGL_WIDTH]*imagevgl->shape[VGL_HEIGHT];
  int bytesPerFrame = pixelsPerFrame*imagevgl->nChannels;
  int totalBytes = bytesPerFrame*imagevgl->shape[VGL_LENGTH];
  if(imagevgl->depth == IPL_DEPTH_16U)
    totalBytes = totalBytes*2;

  return 0;

}

/** Function for loading DICOM images with GDCM library
  */

VglImage* vglGdcmLoadDicom(char* inFilename)
{
    gdcm::ImageReader reader;
    reader.SetFileName(inFilename);
    if(!reader.Read())
        std::cerr << "Could not read: " << inFilename << std::endl;
  
    // The output of gdcm::Reader is a gdcm::File
    //gdcm::File &file = reader.GetFile();

    // the dataset is the the set of element we are interested in:
    //gdcm::DataSet &ds = file.GetDataSet();

    gdcm::Image &image = reader.GetImage();
    //image.Print( std::cout );
  
    gdcm::PixelFormat pixelformat = image.GetPixelFormat();

    VglImage* imagevgl; 

    int width  = image.GetColumns();
    int height = image.GetRows();
    int layers = (image.GetDimensions())[2];
    int depth  = pixelformat.GetBitsAllocated();          // bits per pixel
    int iplDepth = convertDepthGdcmToVgl(depth);           // depth \in {IPL_DEPTH_8U, ...}
    char* filename = (char *) malloc(strlen(inFilename)+1);
    strcpy(filename, inFilename);
    
    int nChannels = pixelformat.GetSamplesPerPixel(); // number of channels

    imagevgl = vglCreate3dImage(cvSize(width,height), iplDepth, nChannels, layers);
    imagevgl->filename = filename;

    int ndarraySize = imagevgl->shape[VGL_WIDTH]*imagevgl->shape[VGL_HEIGHT]*imagevgl->shape[VGL_LENGTH]*imagevgl->nChannels;
    if(pixelformat.GetBitsAllocated() == 16)
        ndarraySize = ndarraySize*2;
    else if(pixelformat.GetBitsAllocated() == 32)
        ndarraySize = ndarraySize*4;
  
    char* buffer = (char*) malloc(ndarraySize);
    image.GetBuffer(buffer);
    imagevgl->ndarray = buffer; // pixels of image

    printf("%s:%s: getbitsallocated = %d\n", __FILE__, __FUNCTION__, pixelformat.GetBitsAllocated());
       
    /*printf("\n\nColumns: %d\nRows: %d\nFrames: %d\nDepth: %d\nChannels: %d\nndim: %d\n\n", imagevgl->shape[VGL_WIDTH], imagevgl->shape[VGL_HEIGHT], imagevgl->shape[VGL_LENGTH], imagevgl->depth, imagevgl->nChannels, imagevgl->ndim);*/

    gdcm::PhotometricInterpretation PI;
    PI = image.GetPhotometricInterpretation();

    if(imagevgl->nChannels == 3)
        if(PI == gdcm::PhotometricInterpretation::YBR_FULL_422) 
            for(int i = 0; i < ndarraySize/3; i++)
            {
	        unsigned char* rgb = (unsigned char*) malloc(3);
	        unsigned char* ybr = &((unsigned char*) (imagevgl->ndarray))[3*i];
	        YBR2RGB(rgb, ybr);
	        //printf("i = %d, imag.ndarray = %d , %d, %d ; rgb = %d , %d, %d\n\n", i, ybr[0], ybr[1], ybr[2], rgb[0], rgb[1], rgb[2]);
	        memcpy((unsigned char*)imagevgl->ndarray+3*i, rgb, 3);
	    }
       else
            if(!(PI == gdcm::PhotometricInterpretation::RGB))
	        printf("This format is not supported"); 
  
    vglSetContext(imagevgl, VGL_RAM_CONTEXT);
    return imagevgl;
}
  

/** Function for saving DICOM images with GDCM library
  */

int vglGdcmSaveDicom(VglImage* imagevgl, char* outFilename, int compress)
{
  if ( (imagevgl->nChannels != 1) && (imagevgl->nChannels != 3) )
  {
    fprintf(stderr, "%s: %s: Error: image has %d channels but only 1 or 3 channels supported. Use vglImage4to3Channels function before saving\n", __FILE__, __FUNCTION__);
    return 1;
  }

  gdcm::ImageReader reader;
  gdcm::Image* image = &reader.GetImage();
  if(!imagevgl->filename)
    int r = vglCreateHeaderGdcm(imagevgl, image);
  else
  {
    reader.SetFileName(imagevgl->filename);
    if(!reader.Read())
      std::cerr << "Could not read: " << imagevgl->filename << std::endl;
    image = &reader.GetImage();
  }
  

  int ndarraySize = imagevgl->shape[VGL_WIDTH]*imagevgl->shape[VGL_HEIGHT]*imagevgl->shape[VGL_LENGTH]*imagevgl->nChannels;

  if(imagevgl->depth == IPL_DEPTH_16U)
     ndarraySize = ndarraySize*2;
  
  gdcm::ImageChangeTransferSyntax change;
  if(imagevgl->filename)
  {
    change.SetTransferSyntax(gdcm::TransferSyntax::ExplicitVRLittleEndian);
    change.SetInput( *image );
    bool b = change.Change();
    if(!b)
      std::cerr << "Could not change the Transfer Syntax" << std::endl;
  }
  
  gdcm::DataElement pixeldata(gdcm::Tag(0x7fe0,0x0010));
  pixeldata.SetByteValue((char*)(imagevgl->ndarray), (uint32_t)ndarraySize);
  image->SetDataElement(pixeldata);

  if(!imagevgl->filename)
  {
    if(compress == 0)
      change.SetTransferSyntax(gdcm::TransferSyntax::ExplicitVRLittleEndian);
    else
      change.SetTransferSyntax(gdcm::TransferSyntax::JPEGBaselineProcess1/*ExplicitVRLittleEndian*/);
    change.SetInput( *image );
    bool b = change.Change();
    if(!b)
      std::cerr << "Could not change the Transfer Syntax" << std::endl;
   }
  
  gdcm::PhotometricInterpretation PI;
  PI = image->GetPhotometricInterpretation();
  if(imagevgl->nChannels == 3)
     if(PI == gdcm::PhotometricInterpretation::YBR_FULL)
        image->SetPhotometricInterpretation( gdcm::PhotometricInterpretation::RGB);
     else
        if(!(PI == gdcm::PhotometricInterpretation::RGB))
	   printf("This format is not supported");  

  if(imagevgl->filename)
    if(compress == 1)
    {
      if(imagevgl->depth == IPL_DEPTH_8U)
        change.SetTransferSyntax( gdcm::TransferSyntax::JPEGBaselineProcess1 );
      else
	if(imagevgl->depth == IPL_DEPTH_16U)
	  change.SetTransferSyntax( gdcm::TransferSyntax::JPEGLosslessProcess14_1 );
      
      change.SetInput( *image );
      bool b = change.Change();
      if( !b )
      {
	std::cerr << "Could not change the Transfer Syntax" << std::endl;
	return 1;
      }
    }
	  
  gdcm::ImageWriter writer;
  writer.SetImage( change.GetOutput() );
  writer.SetFile( reader.GetFile() );
  writer.SetFileName( outFilename );
  if( !writer.Write() )
  {
    std::cerr << "Could not write image" << std::endl;
  }
    
  return 0;
}


/** Function for saving uncompressed DICOM images with GDCM library
  */

int vglGdcmSaveDicomUncompressed(VglImage* imagevgl, char* outFilename)
{
  if ( (imagevgl->nChannels != 1) && (imagevgl->nChannels != 3) )
  {
    fprintf(stderr, "%s: %s: Error: image has %d channels but only 1 or 3 channels supported. Use vglImage4to3Channels function before saving\n", __FILE__, __FUNCTION__);
    return 1;
  }

  int compress = 0;
  int r = vglGdcmSaveDicom(imagevgl, outFilename, compress);
  return r;
}


/** Function for saving compressed DICOM images with GDCM library
  */


int vglGdcmSaveDicomCompressed(VglImage* imagevgl, char* outFilename)
{
  if ( (imagevgl->nChannels != 1) && (imagevgl->nChannels != 3) )
  {
    fprintf(stderr, "%s: %s: Error: image has %d channels but only 1 or 3 channels supported. Use vglImage4to3Channels function before saving\n", __FILE__, __FUNCTION__);
    return 1;
  }

  int compress = 1;
  int r = vglGdcmSaveDicom(imagevgl, outFilename, compress);
  return r;
}

#endif
