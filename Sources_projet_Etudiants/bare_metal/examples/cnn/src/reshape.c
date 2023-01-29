#include "reshape.h"

int reshape(int x_size, int y_size, int z_size, float img_in[MAX_SIZE], float img_out[MAX_SIZE]){
    int i = 0;
    RSHP_Y: for (int y = 0; y < y_size ; y++){
        RSHP_X: for (int x = 0 ; x < y_size ; x++){
            RSHP_Z: for (int z = 0 ; z < z_size ; z++){            
                img_out[i] = img_in[y+x*MAX_SIZE_X+z*MAX_SIZE_X*MAX_SIZE_Y];
                i++;
            }
        }
    }
    return 1;
}
