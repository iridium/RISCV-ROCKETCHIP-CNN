#include <stdio.h>
#include <stdint.h>
#include "lowrisc_memory_map.h"
#include "mini-printf.h"
#include "hid.h"
#include <math.h>

// Include the 2 different inputs images
#include "images.h"

// Include the files necessary for the CNN
#include "CNN_RISCV.h"
#include "convolution.h"
#include "softmax.h"
#include "maxpool.h"
#include "CNN_coefs_3x3.h"

// MACROS 
#define indexCalculationCONV(i, j, c, size_i, size_j, size_c) (i + j*size_i + c*size_i*size_j)

///////////////////////////////////////////////////////////////////////////////////
///////////////////// CONVOLUTION SIMPLE   ////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

// Informations of the read images
#define CONV_READ_WIDTH 640
#define CONV_READ_HEIGHT 480
#define CONV_READ_SIZE_PPM CONV_READ_WIDTH*CONV_READ_HEIGHT*3
#define CONV_READ_SIZE_PGM CONV_READ_WIDTH*CONV_READ_HEIGHT
#define CONV_READ_INT_FORMAT float

// Informations of the written images
#define CONV_WRITE_WIDTH 640
#define CONV_WRITE_HEIGHT 480
#define CONV_WRITE_SIZE_PPM CONV_WRITE_WIDTH*CONV_WRITE_HEIGHT*3
#define CONV_WRITE_SIZE_PGM CONV_WRITE_WIDTH*CONV_WRITE_HEIGHT
#define CONV_WRITE_INT_FORMAT float


/* CONVOLUTION */ 
#define CONV_CONV_SIZE_0 640
#define CONV_CONV_SIZE_1 480
#define CONV_CONV_SIZE_2 1
#define CONV_CONV_TOTAL_SIZE CONV_CONV_SIZE_0*CONV_CONV_SIZE_1
#define CONV_CONV_FIXED_FORMAT float
#define CONV_CONV_NORMALIZE 15

/* CONV KERNEL(S) */
#define KERNEL1_CONV_SIZE_L 1
#define KERNEL1_CONV_SIZE_M 3
#define KERNEL1_CONV_SIZE_N 3

#define KERNEL_CONV_FIXED_FORMAT float

#define EDGE_DETECTOR_NORMALIZE (float)0.00194
#define EDGE_DETECTOR_NORMALIZE2 (float)0.0623
#define EDGE_DETECTOR_THRESHOLD 15
/* CONV BIAISES */ 
#define BIAISES_CONV_FIXED_FORMAT float

/* LUMINANCE */ 
#define LUM_COEF_FIXED_FORMAT float 
#define LUM_RED_COEF (LUM_COEF_FIXED_FORMAT)0.2126
#define LUM_GREEN_COEF (LUM_COEF_FIXED_FORMAT)0.7152
#define LUM_BLUE_COEF (LUM_COEF_FIXED_FORMAT)0.0722
#define LUM_SUM_FIXED_FORMAT float 

///////////////////////////////////////////////////////////////////////////////////
///////////////////// CONVOLUTION SIMPLE FIN //////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

static KERNEL_CONV_FIXED_FORMAT kernel[] = { -0.125,  -0.125,  -0.125, 
									   		  -0.125,  	 	1,  -0.125, 
									   		  -0.125,  -0.125,  -0.125}; 

static BIAISES_CONV_FIXED_FORMAT biaises[] = {0}; 

void convolution_filter(uint8_t image[CONV_READ_SIZE_PGM], KERNEL_CONV_FIXED_FORMAT kernel[3*3*1], BIAISES_CONV_FIXED_FORMAT biaises[1], uint8_t output[CONV_CONV_TOTAL_SIZE])
{
	for(int j = 0; j < CONV_CONV_SIZE_1 ; j++)
	{
		for(int i = 0; i < CONV_CONV_SIZE_0 ; i++)
		{
			for(int c = 0; c < CONV_CONV_SIZE_2 ; c++)
			{
				CONV_CONV_FIXED_FORMAT sum = 0;
				for(int l = 0; l < KERNEL1_CONV_SIZE_L ; l++)
				{
					for(int m = 0; m < KERNEL1_CONV_SIZE_M ; m++)
					{
						bn:for(int n = 0; n < KERNEL1_CONV_SIZE_N ; n++)
						{
							if( ( (j+n) > (CONV_CONV_SIZE_1 - 1) ) && ( (i+m) < (CONV_CONV_SIZE_0-1) ) )
							{
								sum = sum + 0;
							}
							else if( ( (i+m) > (CONV_CONV_SIZE_0-1) ) && ( (j+n) < (CONV_CONV_SIZE_1-1) ) )
							{
								sum = sum + 0;
							}
							else if( ( (i+m) > (CONV_CONV_SIZE_0-1) ) && ( (j+n) > (CONV_CONV_SIZE_1-1) ) )
							{
								sum = sum + 0;
							}
							else if( ( (i+m) < (CONV_CONV_SIZE_0) ) && ( (j+n) < (CONV_CONV_SIZE_1) ) )
							{
								sum = sum + image[indexCalculationCONV((i+m),(j+n),l,(CONV_CONV_SIZE_0),(CONV_CONV_SIZE_1),(CONV_CONV_SIZE_2))] * kernel[m + n*KERNEL1_CONV_SIZE_M+ l*KERNEL1_CONV_SIZE_M*KERNEL1_CONV_SIZE_N + c*KERNEL1_CONV_SIZE_M*KERNEL1_CONV_SIZE_N*KERNEL1_CONV_SIZE_L  ];
							}
							
						}
					}
				}
				
				CONV_CONV_FIXED_FORMAT tmp = sum*CONV_CONV_NORMALIZE;

				if(tmp < 0)
				{
					tmp = tmp*(CONV_CONV_FIXED_FORMAT)(-1);
				}
				
				if(sum < EDGE_DETECTOR_THRESHOLD)
				{
					output[indexCalculationCONV(i,j,c,(CONV_CONV_SIZE_0),(CONV_CONV_SIZE_1),(CONV_CONV_SIZE_2))] = 0 ; 
				}
				else if(sum > 255 || tmp > 255) 
				{
					output[indexCalculationCONV(i,j,c,(CONV_CONV_SIZE_0),(CONV_CONV_SIZE_1),(CONV_CONV_SIZE_2))] = 255 ;
				}
				else
				{	
					output[indexCalculationCONV(i,j,c,(CONV_CONV_SIZE_0),(CONV_CONV_SIZE_1),(CONV_CONV_SIZE_2))] = (uint8_t)(tmp + biaises[0]) ;
				} 
			}
		}
	}
};

const char Cifar10Base[10][11] = {
	"Airplane",
	"Automobile",
	"Bird",
	"Cat",
	"Deer",
	"Dog",
	"Frog",
	"Horse",
	"Ship",
	"Truck"
};
	
// Used by the CNN
CNN_GLOBAL_TYPE CNNArray1[CNN_MAX_STEP_SIZE];
CNN_GLOBAL_TYPE CNNArray2[CNN_MAX_STEP_SIZE];	
float probabilities[CNN_PERCEPTRON_OUTPUT_SIZE];


// The New VGA Driver
extern volatile uint64_t *const hid_new_vga_ptr;

// Filtered image array 
uint8_t filtered_img[640*480]= {0};

// Filter type enum 
enum filter_type {BYPASS, EDGE_DETECTOR, CNN};  
typedef enum filter_type filter_type; 



void display(int img_in_number, filter_type filter_nb, uint8_t previous_imageSel, uint8_t previous_filterSel, uint8_t* edgeDetectorDone, uint8_t* CNNDone)
{
	volatile uint64_t * my_ptr = 0;
	uint8_t * my_ptr_im = 0;
	volatile uint64_t * my_ptr_filter = (uint64_t *)(filtered_img);
	float * my_ptr_cnn = 0;
	
	if(img_in_number == 0)
	{
		my_ptr = (uint64_t *)(imageDatas_0);
		my_ptr_im = imageDatas_0;
	}
	else if(img_in_number == 1)
	{
		my_ptr = (uint64_t *)(imageDatas_1);
		my_ptr_im = imageDatas_1;
	}

	int x,y;
	
	switch(filter_nb)
	{
		case BYPASS : 
			
			if(img_in_number == 0)
			{
				my_ptr = (uint64_t *)(imageDatas_0);
			}
			else if(img_in_number == 1)
			{
				my_ptr = (uint64_t *)(imageDatas_1);
			}
			for(y = 0; y < CONV_READ_HEIGHT; ++y)
			{
				for(x = 0; x < CONV_READ_WIDTH/8; ++x)
				{
					hid_new_vga_ptr[x + y*CONV_READ_WIDTH/8] = (*my_ptr);
					my_ptr++;
				}
			}
		break ;
		
		case EDGE_DETECTOR : 
			if((previous_imageSel ==  img_in_number && *edgeDetectorDone == 0) || (previous_imageSel !=  img_in_number))
			{
				convolution_filter(my_ptr_im, kernel, biaises, filtered_img);
				*edgeDetectorDone = 1;
			} 
			my_ptr_filter = (uint64_t *)(filtered_img);
			for(y = 0; y < CONV_READ_HEIGHT; ++y)
			{
				for(x = 0; x < CONV_READ_WIDTH/8; ++x)
				{
					hid_new_vga_ptr[x + y*CONV_READ_WIDTH/8] = (*my_ptr_filter);
					my_ptr_filter++;
				}
			}
		break ;
		
		case CNN :
			if(img_in_number == 0)
			{
				my_ptr_cnn = (float *)(imageDatas_resize_2);
			}
			else if(img_in_number == 1)
			{
				my_ptr_cnn = (float *)(imageDatas_resize_3);
			}
			
			if((previous_imageSel ==  img_in_number && *CNNDone == 0) || (previous_imageSel !=  img_in_number))
			{
				int returnValue;
				CNN_RISCV(my_ptr_cnn, CNNArray1, CNNArray2);
				if(returnValue == -1)
				{
					printf("[ERROR] : Error in the CNN for the image \n\n");
				}
				float sum = 0;
				float maxProb = -1;
				int maxProbIndex = -1;

				for (int j = 0 ;  j < CNN_PERCEPTRON_OUTPUT_SIZE ; j++)
				{
					probabilities[j] = 100*(CNNArray1[j]);
					sum = sum + probabilities[j];
					if(probabilities[j] > maxProb)
					{
						maxProb = probabilities[j];
						maxProbIndex = j;
					}
				}
				printf("Result of the CNN : \n");
				printf("airplane : %d \n",(int) probabilities[0]);
				printf("automobile : %d \n",(int) probabilities[1]);
				printf("bird : %d \n",(int) probabilities[2]);
				printf("cat : %d \n",(int) probabilities[3]);
				printf("deer : %d \n",(int) probabilities[4]);
				printf("dog : %d \n",(int) probabilities[5]);
				printf("frog : %d \n",(int) probabilities[6]);
				printf("horse : %d \n",(int) probabilities[7]);
				printf("ship : %d \n",(int) probabilities[8]);
				printf("truck : %d \n",(int) probabilities[9]);
				printf("(sum : %d \n",(int) sum);
				printf("--> The image type is %s with a probability of : %d \n\n", Cifar10Base[maxProbIndex], (int)
				 maxProb);
				*CNNDone = 1;
			}
			for(y = 0; y < CONV_READ_HEIGHT; ++y)
			{
				for(x = 0; x < CONV_READ_WIDTH/8; ++x)
				{
					hid_new_vga_ptr[x + y*CONV_READ_WIDTH/8] = (*my_ptr);
					my_ptr++;
				}
			}		 
		break ;
	}
};
