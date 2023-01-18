#include "cnn.h"
#include "img.h"
#include "coeffs.h"
#include <iostream>

using namespace std;

int main()
{
    int classes[10] = {0};
    float probs[10] = {0.};

    int max_i = 0;
    float max = 0;
    top_cnn(coeffs, classes, img, probs);
    for ( int i = 0 ; i < 10 ; i++){
        if ( probs[i] > max){
            max = probs[i];
            max_i = i;
        }
    }
    cout << "IT must be : " << max_i << endl;

    return 0;
}
