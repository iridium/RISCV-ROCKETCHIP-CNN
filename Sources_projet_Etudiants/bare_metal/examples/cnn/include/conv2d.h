#ifndef CONV2D_H_
#define CONV2D_H_

#include "sizes.h"

int conv2d(
    float input_fm[MAX_SIZE],
    float output_fm[MAX_SIZE],
    float kernel[ROM_SIZE],

    int size_fm_x,
    int size_fm_y,
    int size_fm_z,

    int size_k_x,
    int size_k_y,
    int size_k_z,
    int nbk,
    // int bias_nbr, = nbk 

    int conv_idx
);

#endif