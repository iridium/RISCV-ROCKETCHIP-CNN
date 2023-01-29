#include "cnn.h"

int cnn(float *img_in){
    float output_fm_conv1[MAX_SIZE]      = {0};
    float output_fm_maxpool1[MAX_SIZE]   = {0};

    float output_fm_conv2[MAX_SIZE]      = {0};
    float output_fm_maxpool2[MAX_SIZE]   = {0};

    float output_fm_conv3[MAX_SIZE]      = {0};
    float output_fm_maxpool3[MAX_SIZE]   = {0};

    float output_fm_reshape[MAX_SIZE]    = {0};
    float output_fm_perceptron[MAX_SIZE] = {0};

    conv2d(img_in, output_fm_conv1, coeffs, 24, 24, 3, 3, 3, 3, 64, 0);
    maxpool(24, 24, 64, output_fm_conv1, output_fm_maxpool1);
    
    conv2d(output_fm_maxpool1, output_fm_conv2, coeffs, 12, 12, 64, 3, 3, 64, 32, 1);
    maxpool(12, 12, 32, output_fm_conv2, output_fm_maxpool2);
    
    conv2d(output_fm_maxpool2, output_fm_conv3, coeffs, 6, 6, 32, 3, 3, 32, 20, 2);
    maxpool(6, 6, 20, output_fm_conv3, output_fm_maxpool3);
    
    reshape(3, 3, 20, output_fm_maxpool3, output_fm_reshape);
    
    perceptron(output_fm_reshape, coeffs, output_fm_perceptron);

    int index = 0;
    float tmp = 0;
    for (int i = 0 ; i < 10 ; i++)
    {
        if (output_fm_perceptron[i]>tmp)
        {
            tmp = output_fm_perceptron[i];
            index = i;
        }
    }

    return index;
}
