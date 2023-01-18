#include "cnn.h"

void top_cnn(float* tab_coeffs, int* cifar_class, float* normalized_tensor, float* cifar_probabilities){

    float output_fm_conv1[MAX_SIZE] = {0.};
    float output_fm_maxpool1[MAX_SIZE] = {0.};
    float output_fm_conv2[MAX_SIZE] = {0.};
    float output_fm_maxpool2[MAX_SIZE] = {0.};
    float output_fm_conv3[MAX_SIZE] = {0.};
    float output_fm_maxpool3[MAX_SIZE] = {0.};
    float output_fm_reshape[MAX_SIZE] = {0.};

    int stride[] = {2, 2};
 
    conv2d(normalized_tensor, output_fm_conv1, tab_coeffs, 24, 24, 3, 3, 3, 3, 64, 0);
    MaxPool(output_fm_conv1, output_fm_maxpool1, stride, 24, 24, 64);
    
    conv2d(output_fm_maxpool1, output_fm_conv2, tab_coeffs, 12, 12, 64, 3, 3, 64, 32, 1);
    MaxPool(output_fm_conv2, output_fm_maxpool2, stride, 12, 12, 32);
    
    conv2d(output_fm_maxpool2, output_fm_conv3, tab_coeffs, 12, 12, 32, 3, 3, 32, 20, 1);
    MaxPool(output_fm_conv3, output_fm_maxpool3, stride, 6, 6, 20);
    
    reshape(3, 3, 20, output_fm_maxpool3, output_fm_reshape);
    perceptron(output_fm_reshape, tab_coeffs, cifar_probabilities);


    cifar_class[0] = 0;
    int tmp = 0;
    // for loop to find max index and max value ? doesn't work
    for (int i = 0 ; i < 9 ; i++){
        for ( int j = 0 ; j < 9 - i ; j++){
            if (cifar_probabilities[cifar_class[j]] > cifar_probabilities[cifar_class[j+1]]){
                tmp = cifar_class[j+1];
                cifar_class[j+1] = cifar_class[j];
                cifar_class[j] = tmp;
            }
        }
    }
}
