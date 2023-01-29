#ifndef _SIZES_H_
#define _SIZES_H_

// Definition of MAX SIZES
#define MAX_SIZE_X      24
#define MAX_SIZE_Y      24
#define MAX_SIZE_Z      64
#define MAX_SIZE        MAX_SIZE_X*MAX_SIZE_Y*MAX_SIZE_Z

#define MAX_K_SIZE_X    3
#define MAX_K_SIZE_Y    3
#define MAX_K_SIZE_Z    64
#define MAX_K_NBR       64
#define MAX_BIAS_NBR    64
#define CONV_NBR        3
#define END_MATRIX_S    180*10 + 10
#define MAX_KERNEL_SIZE MAX_K_SIZE_X*MAX_K_SIZE_Y*MAX_K_SIZE_Z
#define ROM_SIZE        CONV_NBR*(MAX_KERNEL_SIZE*MAX_K_NBR + MAX_BIAS_NBR) + END_MATRIX_S

#define fm(x,y,z)          x + y*MAX_SIZE_X + z*MAX_SIZE_Y*MAX_SIZE_X
#define ki(x,y,z,k_idx,conv_idx) (MAX_KERNEL_SIZE*MAX_K_NBR + MAX_BIAS_NBR)*conv_idx + x + y*MAX_K_SIZE_X + z*MAX_K_SIZE_X*MAX_K_SIZE_Y + k_idx*MAX_KERNEL_SIZE
#define b(bi,conv_idx)     MAX_KERNEL_SIZE*MAX_K_NBR*(conv_idx+1) + MAX_BIAS_NBR*conv_idx + bi

// Information about 1st MAXPOOL
#define MP1_IN_X 24
#define MP1_IN_Y 24
#define MP1_IN_Z 64

#define MP1_OUT_X 12
#define MP1_OUT_Y 12
#define MP1_OUT_Z 64

// Information about 2nd MAXPOOL
#define MP2_IN_X 12
#define MP2_IN_Y 12
#define MP2_IN_Z 32

#define MP2_OUT_X 6
#define MP2_OUT_Y 6
#define MP2_OUT_Z 32

// Information about 3rd MAXPOOL
#define MP3_IN_X 6
#define MP3_IN_Y 6
#define MP3_IN_Z 20

#define MP3_OUT_X 3
#define MP3_OUT_Y 3
#define MP3_OUT_Z 20

// Information about 1st conv2D
#define CONV1_IN_X 24
#define CONV1_IN_Y 24
#define CONV1_IN_Z 3

#define CONV1_NBK 64
#define CONV1_K_X 3
#define CONV1_K_Y 3
#define CONV1_K_Z 3

// Information about 2nd conv2D
#define CONV2_IN_X 12
#define CONV2_IN_Y 12
#define CONV2_IN_Z 64

#define CONV2_NBK 32
#define CONV2_K_X 3
#define CONV2_K_Y 3
#define CONV2_K_Z 64

// Information about 3rd conv2D
#define CONV3_IN_X 6
#define CONV3_IN_Y 6
#define CONV3_IN_Z 32

#define CONV3_NBK 20
#define CONV3_K_X 3
#define CONV3_K_Y 3
#define CONV3_K_Z 32

// Information about reshape
#define RESHAPE_OUT 180

#define PERCEP_IN 180
#define PERCEP_OUT 10

#endif
