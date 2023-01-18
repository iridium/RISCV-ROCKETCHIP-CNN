#ifndef _CNN_H_
#define _CNN_H_

#include "sizes.h"
#include "MaxPoolSE.h"
#include "conv2d.h"
#include "reshape.h"
#include "perceptron.h"

void top_cnn(float* tab_coeffs, int* cifar_class, float* normalized_tensor, float* cifar_probabilities);

#endif
