
/*********************************************************************
***                                                                 ***
***  Source code generated by kernel2cu.pl                          ***
***                                                                 ***
***  Please do not edit                                             ***
***                                                                 ***
*********************************************************************/

#include "vglImage.h"
#include "vglLoadShader.h"
#include "vglContext.h"

#include <iostream>

//kernels
#include "kernel2cu_shaders.kernel"
/** vglCudaCopy

    Copy of image in cuda context.
*/
void vglCudaCopy(VglImage*  input, VglImage*  output){
    if (!input){
      printf("vglCudaCopy: Error: input parameter is null in file '%s' in line %i.\n",
              __FILE__, __LINE__);
      exit(1);
    }
    vglCheckContext(input, VGL_CUDA_CONTEXT);
    if (!input->cudaPtr){
      printf("vglCudaCopy: Error: input->cudaPtr is null in file '%s' in line %i.\n",
              __FILE__, __LINE__);
      exit(1);
    }

    if (!output){
      printf("vglCudaCopy: Error: output parameter is null in file '%s' in line %i.\n",
              __FILE__, __LINE__);
      exit(1);
    }
    vglCheckContext(output, VGL_CUDA_CONTEXT);
    if (!output->cudaPtr){
      printf("vglCudaCopy: Error: output->cudaPtr is null in file '%s' in line %i.\n",
              __FILE__, __LINE__);
      exit(1);
    }


    switch (input->depth){
      case (IPL_DEPTH_8U):
        global_Copy<<<input->height,384>>>((unsigned char* )input->cudaPtr, (unsigned char* )output->cudaPtr, input->width, input->height, input->nChannels);
        break;
      default:
        printf("vglCudaCopy: Error: unsupported img->depth = %d in file '%s' in line %i.\n",
                input->depth, __FILE__, __LINE__);
        exit(1);
    }

    vglSetContext(input, VGL_CUDA_CONTEXT);
    vglSetContext(output, VGL_CUDA_CONTEXT);
}

/** vglCudaInvert

    Inverts image stored in cuda context.
*/
void vglCudaInvert(VglImage*  input, VglImage*  output){
    if (!input){
      printf("vglCudaInvert: Error: input parameter is null in file '%s' in line %i.\n",
              __FILE__, __LINE__);
      exit(1);
    }
    vglCheckContext(input, VGL_CUDA_CONTEXT);
    if (!input->cudaPtr){
      printf("vglCudaInvert: Error: input->cudaPtr is null in file '%s' in line %i.\n",
              __FILE__, __LINE__);
      exit(1);
    }

    if (!output){
      printf("vglCudaInvert: Error: output parameter is null in file '%s' in line %i.\n",
              __FILE__, __LINE__);
      exit(1);
    }
    vglCheckContextForOutput(output, VGL_CUDA_CONTEXT);
    if (!output->cudaPtr){
      printf("vglCudaInvert: Error: output->cudaPtr is null in file '%s' in line %i.\n",
              __FILE__, __LINE__);
      exit(1);
    }


    switch (input->depth){
      case (IPL_DEPTH_8U):
        global_Invert<<<input->height,384>>>((unsigned char* )input->cudaPtr, (unsigned char* )output->cudaPtr, input->width, input->height, input->nChannels);
        break;
      default:
        printf("vglCudaInvert: Error: unsupported img->depth = %d in file '%s' in line %i.\n",
                input->depth, __FILE__, __LINE__);
        exit(1);
    }

    vglSetContext(output, VGL_CUDA_CONTEXT);
}

/** vglCudaInvertOnPlace

    Inverts image, stored in cuda context, on place.
*/
void vglCudaInvertOnPlace(VglImage*  input){
    if (!input){
      printf("vglCudaInvertOnPlace: Error: input parameter is null in file '%s' in line %i.\n",
              __FILE__, __LINE__);
      exit(1);
    }
    vglCheckContext(input, VGL_CUDA_CONTEXT);
    if (!input->cudaPtr){
      printf("vglCudaInvertOnPlace: Error: input->cudaPtr is null in file '%s' in line %i.\n",
              __FILE__, __LINE__);
      exit(1);
    }


    switch (input->depth){
      case (IPL_DEPTH_8U):
        global_InvertOnPlace<<<input->height,384>>>((unsigned char* )input->cudaPtr, input->width, input->height, input->nChannels);
        break;
      default:
        printf("vglCudaInvertOnPlace: Error: unsupported img->depth = %d in file '%s' in line %i.\n",
                input->depth, __FILE__, __LINE__);
        exit(1);
    }

    vglSetContext(input, VGL_CUDA_CONTEXT);
}

