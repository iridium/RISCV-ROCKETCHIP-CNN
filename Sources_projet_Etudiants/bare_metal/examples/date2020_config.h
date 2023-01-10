#ifndef  DATE2020_CONFIG_H
#define DATE2020_CONFIG_H


// max size of file image is 16M
#define MAX_FILE_SIZE 0x1000000

// 4K size read burst
#define SD_READ_SIZE 4096

//Dimensions de l'ecran en sortie
#define DISPLAY_IMAGE_HEIGHT 480
#define DISPLAY_IMAGE_WIDTH 640
#define DISPLAY_IMAGE_SIZE (DISPLAY_IMAGE_HEIGHT * DISPLAY_IMAGE_WIDTH)

// Dimensions for the NN
#define NN_IN_HEIGHT 24
#define NN_IN_WIDTH 24
#define NN_IN_SIZE (NN_IN_HEIGHT * NN_IN_WIDTH)

// Dimensions for the Overlays
#define OVERLAY_WIDTH 176
#define OVERLAY_HEIGHT 80

//Nombre d'images à lire
#define MIN_IMAGES_TO_READ 1
#define MAX_IMAGES_TO_READ    3
//#define MAX_IMAGES_TO_READ 14
#define NB_IMAGES_TO_BE_READ (MAX_IMAGES_TO_READ - MIN_IMAGES_TO_READ + 1)

//Numero de l'image a lire (doit etre inférieur a MAX_IMAGES_TO_READ)
#define IMAGE_TO_BE_READ 9

#define MIN_FILTERS_TO_APPLY  0
#define MAX_FILTERS_TO_APPLY  2


#endif
