// See LICENSE for license details.

//#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <memory.h>
// #include <math.h>
#include "encoding.h"
#include "mini-printf.h"
#include "diskio.h"
#include "ff.h"
#include "bits.h"
#include "hid.h"
#include "eth.h"
#include "elfriscv.h"
#include "lowrisc_memory_map.h"

// Including our header
#include "application.h"

// Including our CNN
#include "cnn.h"

// Including paramter (sizes, images to read, number of filters ...)
#include "date2020_config.h"

unsigned char OVERLAYS_LIST[];
//-----------------------------

#define DEBUG 0
#define DEBUG_PRINTF(...)  \
  do                       \
  {                        \
    if (DEBUG)             \
      printf(__VA_ARGS__); \
  } while (0)

//-----------------------------

FATFS FatFs; // Work area (file system object) for logical drive


// Informations of the read images
#define CONV_READ_WIDTH 640
#define CONV_READ_HEIGHT 480
#define CONV_READ_SIZE_PPM CONV_READ_WIDTH *CONV_READ_HEIGHT * 3
#define CONV_READ_SIZE_PGM CONV_READ_WIDTH *CONV_READ_HEIGHT
#define CONV_READ_INT_FORMAT float




//////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  Fonctions Utiles  /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////




/* Les fonctions suivantes ont ete ajoutees dans ce fichier car nous n'avons pas reussi a les inclures depuis les fichiers du RISC-V */
/* Fonction servant à ajouter une chaine de caractere a une autre */
char *My_strcat(char *dest, const char *src)
{
	char *tmp = dest;

	while (*dest)
		dest++;
	while ((*dest++ = *src++) != '\0')
		;

	return tmp;
}

/* Fonction servant a transformer un caractere en chiffre (dans les cas ou c'est possible) */
int My_atoi(char *chaine)
{
	int res = 0;
	int i;
	for (i = 0; chaine[i] != '\0'; i++)
	{
		res = res * 10 + chaine[i] - '0';
	}
	return res;
}

/* Fonctions utilisees par My_strtok */
char *My_strpbrk(const char *cs, const char *ct)
{
	const char *sc1, *sc2;

	for (sc1 = cs; *sc1 != '\0'; ++sc1)
	{
		for (sc2 = ct; *sc2 != '\0'; ++sc2)
		{
			if (*sc1 == *sc2)
				return (char *)sc1;
		}
	}
	return NULL;
}

size_t My_strspn(const char *s, const char *accept)
{
	const char *p;
	const char *a;
	size_t count = 0;

	for (p = s; *p != '\0'; ++p)
	{
		for (a = accept; *a != '\0'; ++a)
		{
			if (*p == *a)
				break;
		}
		if (*a == '\0')
			return count;
		++count;
	}

	return count;
}

/* Variable globale utilisee par My_strtok stockant les token suivants */
char *___mystrtok;

/* Fonction permettant de séparer une chaine de caractere en differents token stockes dans __strtok
   Utilisation : Token = strtok(chaine de caractere, separateur)
                 Token suivant = strtok(NULL, separateur) */
char *My_strtok(char *s, const char *ct)
{

	char *sbegin, *send;

	sbegin = s ? s : ___mystrtok;
	if (!sbegin)
	{
		return NULL;
	}
	sbegin += My_strspn(sbegin, ct);
	if (*sbegin == '\0')
	{
		___mystrtok = NULL;
		return (NULL);
	}
	send = My_strpbrk(sbegin, ct);
	if (send && *send != '\0')
		*send++ = '\0';
	___mystrtok = send;
	return (sbegin);
}

extern volatile uint64_t *const hid_new_vga_ptr; // = (volatile uint64_t *)(new_vga_base_addr);

uint8_t TAB_GS[NB_IMAGES_TO_BE_READ][DISPLAY_IMAGE_SIZE] = {0};          //Tableau de pixel de toutes les images rangeais les uns apres les autres
uint8_t TAB_GS_FILTERED[NB_IMAGES_TO_BE_READ][DISPLAY_IMAGE_SIZE] = {0}; //Tableau de pixel de toutes les images rangeais les uns apres les autres

// CNN Stuff --------------------------------------------------------------------
// Tableau de pixel de toutes les images rangés les uns apres les autres
uint8_t global_tab[NB_IMAGES_TO_BE_READ * DISPLAY_IMAGE_SIZE * 3] = {0};

uint8_t resized_img[NN_IN_SIZE * 3] = {0};
float resized_tensor[NN_IN_SIZE * 3] = {0};
float normalized_tensor[NN_IN_SIZE * 3] = {0};
// -------------------------------------------------------------------------------------



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////  Partie Lecture des images  //////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void read_pic(int n_image, int *tab_size, int *tab_width, int *tab_length, uint8_t *global_tab)
{

	FIL fil;    // File object
	FRESULT fr; // FatFs return code

	TCHAR *plop;
	TCHAR chaine[512];
	char *strToken = calloc(100, sizeof(*strToken));
	char *text = calloc(10000, sizeof(*text));

	int fsize = 0; // file size count
	int br;        // Read count
	int c1 = 0;
	int c2 = 0;
	int i = 0;
	int j = 0; //added to do OBS2

	int length = 0;
	int width = 0;
	int size = 0;
	char file_name[30] = {'\0'};                //Nom du fichier a ouvrir
	uint8_t pixels[DISPLAY_IMAGE_SIZE * 3];     //Tableau de pixel pour une image


	//Generation du nom de fichier
	//sprintf( ..., "%d.ppm", ... );
	sprintf( file_name, "%d.ppm", n_image );

	// Open a file
	//printf("Loading %s\n", ... );
	//fr = f_open( ... , ... , FA_READ);
	printf("Loading %s\n", file_name );
	fr = f_open( &fil , file_name , FA_READ);
	if (fr)
	{
		printf("Failed to open %s!\n",  file_name );
		//exit(1);
	}

	//Lecture de l'entete
	//fr = f_read( &fil , &c1, 1, ... );
	//fr = f_read( &fil , &c2, 1, ... );
	fr = f_read( &fil , &c1, 1, &br );
	fr = f_read( &fil , &c2, 1, &br );

	//Si l'entete vaut les caracteres 'P3' alors, on est dans le cas d'un fichier ppm
	if (c1 == 0x50 && c2 == 0x33)
	{
		//printf("Le fichier %s est un fichier ppm P3.\n", ... );
		//plop = f_gets(text, 10000, ... );
		//plop = f_gets(text, 10000, ... );
		printf("Le fichier %s est un fichier ppm P3.\n", file_name );
		plop = f_gets(text, 10000, &fil );
		plop = f_gets(text, 10000, &fil );
		if (text[0] == '#')
		{
			// test ligne de commentaire de openCV
			plop = f_gets(text, 10000, &fil);
		}
		//strToken = ...(text, " ");					//Utilisation des fonctions sur les chaînes de caractères décrites plus haut
		//length = ...(strToken); //Lecture de la longueur de l'image
		//strToken = ...(NULL, "\n");
		//width = ...(strToken); //Lecture de la largeur de l'image
		//size = length * width;
		//tab_width[...] = width;						//Remplissage des tableaux des valeus de longueur, largeur et taille des images lues
		//tab_length[...] = length;
		//tab_size[...] = size;

		strToken = My_strtok(text, " ");					//Utilisation des fonctions sur les chaînes de caractères décrites plus haut
		length = My_atoi(strToken); //Lecture de la longueur de l'image
		strToken = My_strtok(NULL, "\n");
		width = My_atoi(strToken); //Lecture de la largeur de l'image
		size = length * width;
		tab_width[n_image] = width;						//Remplissage des tableaux des valeus de longueur, largeur et taille des images lues
		tab_length[n_image] = length;
		tab_size[n_image] = size;
		for (i = 0; i < size; i++)					//initialisation du tableau pixel
		{
			pixels[i] = 0;
		}

		//plop = f_gets(text, 10000, ... );
		printf("File size: %d and image size : %d * %d = %d\n", tab_size[n_image] ,
			   tab_length[ n_image ],
			   tab_width[ n_image ],
			   tab_size[ n_image ]); //OBS 1: File size = image size??

		plop = f_gets(text, 10000, &fil );
		i = 0;
		plop = calloc(3 * size, sizeof(*plop));
		//Pour toutes les lignes du fichier
		while (&fil != NULL && i < (3 * size))
		{
			plop = f_gets(text, 10000, &fil ); //On lit une ligne

			strToken = My_strtok(text, " ");  //On separe les differents chiffres
			//Pour tous les chiffres de la ligne
			while (strToken != NULL && i < (3 * size))
			{
				pixels[i] = My_atoi(strToken); //On remplit le tableau pixel par pixel
				i++;
				strToken = My_strtok(NULL, " "); //On selectionne le token suivant
				if (strToken[0] == '\n')
				{
					// On enlève les caractère de saut de ligne '\n'
					strToken = NULL;
				}
			}
		}
	}

	printf("n_image = %d\n", n_image);

	//OBS2: This for was modified to save the files in n_image*width*length*bit
	for (j = 0; j < size * 3; j += 3)
	{
		for (i = 0; i <  3; i++)
		{
			global_tab[size * i + (n_image - 1)*size * 3] = pixels[i + j]; //On remplit le tableau global pour pouvoir reutiliser le tableau pixel
		}
	}
	printf("Closing file %s\n", file_name);

	// Close the file
	if (f_close(&fil))
	{
		printf("Fail to close file!");
		//exit(1);
	}

	//free(...);
	//free(...);
	//free(...);

	free(plop);
	free(strToken);
	free(text);

}




void convert_to_greyscale(int n_image, int *tab_size, int *tab_width, int *tab_length, uint8_t *global_tab, uint8_t image[CONV_READ_SIZE_PGM])
{
	printf("Affichage image numero : %d   %d*%d=%d\n", n_image, tab_width[n_image - 1], tab_length[n_image - 1], tab_size[n_image - 1]);
  int pass = (n_image - 1)*CONV_READ_SIZE_PPM;
  for (int i = 0; i < tab_size[n_image - 1] * 3; i ++)
	{
		//For each pixel on R, G et B
    //On remplit pixel par pixel le tableau image en utilisant 0.3 de la valeur de R, 0.57 de la valeur de G et et 0.11 de la valeur de B par pixels du tableau global_tab
    image[i] =  0.3*global_tab[i + pass] + 0.57*global_tab[CONV_READ_SIZE_PGM  + i + pass] + 0.11*global_tab[CONV_READ_SIZE_PGM*2  + i + pass] ;
	}               //R                       //G                                    //B
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////  Partie Interruptions  ////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// FOR INTERRUTPS  ------------------------------------------

void init_csrs()
{
  write_csr(mie,     0);    // init mie
  write_csr(sie,     0);    // init sie
  write_csr(mip,     0);    // init mip
  write_csr(sip,     0);    // init sip
  write_csr(mideleg, 0);    // init mideleg
  write_csr(medeleg, 0);    // init medeleg
}

#define PLIC_BASE_ADDRESS 0x0C000000
#define PLIC_MAX_PRIORITY 7

#define ID_BTNW 1
#define ID_BTNE 2
#define ID_BTNS 3
#define ID_BTNN 4

#define PLIC_PRIORITY_BTNW (PLIC_BASE_ADDRESS + 4 * ID_BTNW)
#define PLIC_PRIORITY_BTNE (PLIC_BASE_ADDRESS + 4 * ID_BTNE)
#define PLIC_PRIORITY_BTNS (PLIC_BASE_ADDRESS + 4 * ID_BTNS)
#define PLIC_PRIORITY_BTNN (PLIC_BASE_ADDRESS + 4 * ID_BTNN)

#define PLIC_INT_PENDING_BASEADDR 0x0C001000
#define PLIC_INT_ENABLE_BASEADDR 0x0C002000

#define PLIC_HART0_PRIO_THRESH_ADDR 0x0C200000
#define PLIC_HART0_CLAIM_COMPLETE_ADDR 0x0C200004

// Masks definition
// Refers to chip_top.sv to know the connections of the buttons
#define PLIC_PENDING_BTNW (1 << 1)
#define PLIC_ENABLE_BTNW (1 << 1)

#define PLIC_PENDING_BTNE (1 << 2)
#define PLIC_ENABLE_BTNE (1 << 2)

#define PLIC_PENDING_BTNS (1 << 3)
#define PLIC_ENABLE_BTNS (1 << 3)

#define PLIC_PENDING_BTNN (1 << 4)
#define PLIC_ENABLE_BTNN (1 << 4)





void enable_plic_interrupts()
{

  // Setting the Priority of the interrupt with ID 1,2,3 and 4 to value 1, so that the interrupts can be fired
  // Recall that an interrupt is fired when its priority is > than the threshold
  *(volatile unsigned int *) PLIC_PRIORITY_BTNW = 1;
  *(volatile unsigned int *) PLIC_PRIORITY_BTNE = 1;
  *(volatile unsigned int *) PLIC_PRIORITY_BTNS = 1;
  *(volatile unsigned int *) PLIC_PRIORITY_BTNN = 1;

  // Setting the priority threshold to Zero
  *(volatile unsigned int *) PLIC_HART0_PRIO_THRESH_ADDR = 0;

  // clear interrupt pending
  *(volatile unsigned int *) PLIC_INT_PENDING_BASEADDR = 0;

  // PLIC ENABLE interrupts of ID 1,2,3 and 4
  // (ID 1 and ID 2 are connected to zero)
  *(volatile unsigned int *) PLIC_INT_ENABLE_BASEADDR = (PLIC_ENABLE_BTNW | PLIC_ENABLE_BTNE | PLIC_ENABLE_BTNN | PLIC_ENABLE_BTNS);

  // Enable MEIP (Machine External Interrupt Pending) bit in MIE register
  set_csr(mie, MIP_MEIP);

  // Enable MIE (Machine Interrupt Enable) bit of MSTATUS
  set_csr(mstatus, MSTATUS_MIE);
}




volatile int imageSel;
volatile int filterSel;
volatile int isBouncing;


void external_interrupt(void)
{
	int claim = 0;
#ifdef VERBOSE
  //printf("Hello external interrupdet! "__TIMESTAMP__"\n");
#endif

  // Read the ID (the highest priority pending interrupt)
  // If the value we read is zero then no pending interrupt is coming from PLIC
  claim = plic[ 0x80001 ]; 									//consulter le fichier syscall.c
  clear_csr(mie, MIP_MEIP);
  if(isBouncing == 0)
  {
    // printf("Interrupt executed !\n");
  	// If BTNW :									//Si pression du bouton Ouest, décrémentation de la variable de sélection de l'image
  	if (claim == 1)									//Mise à sa valeur max si elle atteint sa valeur min
  	{
  	  imageSel--;
  		if( imageSel < 0) imageSel =  2;
    }
  	// If BTNE :									//Si pression du bouton Est, incrémentation de la variable de sélection de l'image
  	else if (claim == 2)								//Mise à sa valeur min si elle atteint sa valeur max
  	{
      imageSel++;
      if( imageSel > 2 ) imageSel = 0;
  	}
  	// If BTNS :									//Si pression du bouton Sud, décrémentation de la variable de sélection du filtre
  	else if (claim == 3)								//Mise à sa valeur max si elle atteint sa valeur min
  	{
  		filterSel--;
  		if( filterSel < 0 ) filterSel = 2 ;
  	}
  	// If BTNN :									//Si pression du bouton Nord, incrémentation de la variable de sélection du filtre
  	else if (claim == 4)								//Mise à sa valeur min si elle atteint sa valeur max
  	{
      filterSel++;
      if(filterSel > 2) filterSel = 0;
  	}
  	isBouncing = 1;
  }

  // Write the ID of the interrupt source to the claim/complete register to complete the interrupt
  // The PLIC will clear the pending bit of the corresponding ID
  // /!\ If the ID don't match the pending ID, the completion is silently ignored
  plic[ 0x80001 ] = claim;
  set_csr(mie, MIP_MEIP);
}









//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





// MACROS
#define indexCalculationCONV(i, j, c, size_i, size_j, size_c) (i + j * size_i + c * size_i * size_j)

// Filter type enum
enum filter_type
{
	BYPASS,
	EDGE_DETECTOR,
	CNN_CLASSIFIER
};
typedef enum filter_type filter_type;

/* CONVOLUTION */
#define CONV_CONV_SIZE_0 640
#define CONV_CONV_SIZE_1 480
#define CONV_CONV_SIZE_2 1
#define CONV_CONV_TOTAL_SIZE CONV_CONV_SIZE_0 *CONV_CONV_SIZE_1
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

///////////////////////////////////////////////////////////////////////////////////
///////////////////// CONVOLUTION SIMPLE FIN //////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

static KERNEL_CONV_FIXED_FORMAT kernel[] = { -0.125, -0.125, -0.125,
		-0.125, 1, -0.125,
		-0.125, -0.125, -0.125
										   };

static BIAISES_CONV_FIXED_FORMAT biaises[] = {0};

// filter_nb = soit 0 soit 1
void convolution_filter(uint8_t image[CONV_READ_SIZE_PGM], KERNEL_CONV_FIXED_FORMAT kernel[3 * 3 * 1], BIAISES_CONV_FIXED_FORMAT biaises[1], uint8_t output[CONV_CONV_TOTAL_SIZE])
{
	for (int j = 0; j < CONV_CONV_SIZE_1; j++)
	{
		for (int i = 0; i < CONV_CONV_SIZE_0; i++)
		{
			for (int c = 0; c < CONV_CONV_SIZE_2; c++)
			{
				CONV_CONV_FIXED_FORMAT sum = 0;
				for (int l = 0; l < KERNEL1_CONV_SIZE_L; l++)
				{
					for (int m = 0; m < KERNEL1_CONV_SIZE_M; m++)
					{
bn:
						for (int n = 0; n < KERNEL1_CONV_SIZE_N; n++)
						{
							if (((j + n) > (CONV_CONV_SIZE_1 - 1)) && ((i + m) < (CONV_CONV_SIZE_0 - 1)))
							{
								sum = sum + 0;
							}
							else if (((i + m) > (CONV_CONV_SIZE_0 - 1)) && ((j + n) < (CONV_CONV_SIZE_1 - 1)))
							{
								sum = sum + 0;
							}
							else if (((i + m) > (CONV_CONV_SIZE_0 - 1)) && ((j + n) > (CONV_CONV_SIZE_1 - 1)))
							{
								sum = sum + 0;
							}
							else if (((i + m) < (CONV_CONV_SIZE_0)) && ((j + n) < (CONV_CONV_SIZE_1)))
							{
								sum = sum + image[indexCalculationCONV((i + m), (j + n), l, (CONV_CONV_SIZE_0), (CONV_CONV_SIZE_1), (CONV_CONV_SIZE_2))] * kernel[m + n * KERNEL1_CONV_SIZE_M + l * KERNEL1_CONV_SIZE_M * KERNEL1_CONV_SIZE_N + c * KERNEL1_CONV_SIZE_M * KERNEL1_CONV_SIZE_N * KERNEL1_CONV_SIZE_L];
							}
						}
					}
				}

				CONV_CONV_FIXED_FORMAT tmp = sum * CONV_CONV_NORMALIZE;

				if (tmp < 0)
				{
					tmp = tmp * (CONV_CONV_FIXED_FORMAT)(-1);
				}

				if (sum < EDGE_DETECTOR_THRESHOLD)
				{
					output[indexCalculationCONV(i, j, c, (CONV_CONV_SIZE_0), (CONV_CONV_SIZE_1), (CONV_CONV_SIZE_2))] = 0;
				}
				else if (sum > 255 || tmp > 255)
				{
					output[indexCalculationCONV(i, j, c, (CONV_CONV_SIZE_0), (CONV_CONV_SIZE_1), (CONV_CONV_SIZE_2))] = 255;
				}
				else
				{
					output[indexCalculationCONV(i, j, c, (CONV_CONV_SIZE_0), (CONV_CONV_SIZE_1), (CONV_CONV_SIZE_2))] = (uint8_t)(tmp + biaises[0]);
				}
			}
		}
	}
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////  Partie CNN  ///////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// //square root function
float __ieee754_sqrtf(float x)
{
  asm("fsqrt.s %0, %1"
      : "=f"(x)
      : "f"(x));
  return x;
}

double __ieee754_sqrt(double x)
{
  asm("fsqrt.d %0, %1"
      : "=f"(x)
      : "f"(x));
  return x;
}


// This use the AREA based resizing method, just like the one used in OpenCV
void my_resizing(uint8_t* target_img, uint8_t* source_img, int source_size, int source_sizeX, int source_sizeY, int target_size, int target_sizeX, int target_sizeY)
{
  double x_ratio = (double) source_sizeX / target_sizeX;
  double y_ratio = (double) source_sizeY / target_sizeY;
  for (int y = 0; y < target_sizeY; y++) {
    for (int x = 0; x < target_sizeX; x++) {
      for (int k = 0; k < 3; k++) {
        int tot = 0;

        for (int j = 0; j < y_ratio; j++){
          for (int i = 0; i < x_ratio; i++){
            int x2 = x_ratio*x + i;
            int y2 = y_ratio*y + j;
            tot += *(source_img + x2 + source_sizeX*y2 + source_sizeX*source_sizeY*k);
          }
        }
        uint8_t moy = tot/(y_ratio*x_ratio);
        *(target_img + x + target_sizeX*y + target_sizeX*target_sizeY*k) = moy;
      }
    }
  }
}



//
// Normalizing the image 24x24 to be feed to the CNN
//
void normalizing(float* normalized_img, uint8_t* resized_img, int size){ // height * width * 3 
// J'AI CHANGé L'ENTETE, RESIZEDIMG EST UN UINT_8* AU LIEU DE FLOAT*    
    float mu = 0;
    float ecartype = 0;

    // Calcul de la moyenne
    for (int i = 0; i < size; i++) {mu += *(resized_img+i);}
    mu /= size;

    // Calcul de l'écart-type
    for (int i = 0; i < size; i++) {ecartype += (*(resized_img+i) - mu) * (*(resized_img+i) - mu);}
    ecartype = ecartype/size;//__ieee754_sqrt(ecartype / size);

    // Calcul de la valeur maximale
    float maximum = 0;
    //if (*1/__ieee754_sqrt(size/3) > ecartype){maximum = 1/__ieee754_sqrt(size/3);}
    if (1/(size/3) > ecartype){maximum = 1/(size/3);}
    else {maximum = ecartype;}

    // Normalisation des valeurs de la matrice
    for (int i = 0; i < size; i++){
      *(normalized_img+i) = (*(resized_img+i) - mu) / maximum; //(mat_int.values[i][j][k] - mu) / maximum;
    }
}

int perform_cnn(int img_in_number)
{
  // Source = the 640*480 image
  int source_size = DISPLAY_IMAGE_SIZE; // SOURCE IMG (640*480)
  int source_sizeY = DISPLAY_IMAGE_HEIGHT;
  int source_sizeX = DISPLAY_IMAGE_WIDTH;
  // target is the resized/normalized outputs
  int target_size = NN_IN_SIZE; // RESIZED size
  int target_sizeY = NN_IN_HEIGHT;
  int target_sizeX = NN_IN_WIDTH;

  // Allocate memory for intermediate images/tensors
  uint8_t *source_img = (uint8_t *)calloc(DISPLAY_IMAGE_SIZE * 3, sizeof(*source_img));
  uint8_t *resized_img 		  = (uint8_t *)calloc(NN_IN_SIZE * 3, sizeof(*resized_img));
  float *normalized_tensor 	= (float *)calloc(NN_IN_SIZE * 3, sizeof(*normalized_tensor));

  // Load the 640*480 PPM image
  source_img = &global_tab[(img_in_number - MIN_IMAGES_TO_READ) * DISPLAY_IMAGE_SIZE * 3];

  // Resize to a 24*24 RGB img.
  DEBUG_PRINTF("Starting resizing");
  my_resizing(resized_img, source_img, source_size, source_sizeX, source_sizeY,
              target_size, target_sizeX, target_sizeY);

  // Already a tensor
  
  // Normalization
  DEBUG_PRINTF("Starting normalization \n");
  normalizing(normalized_tensor, resized_img, target_size);

  // Getting the class detected by the CNN in index
  int index = cnn(normalized_tensor);

  free(source_img);
  free(resized_img);
  free(normalized_tensor);	
  return index;	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////  Partie Display  /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void on_screen(int mode, int class, uint8_t *img)
{
	//, uint8_t* proc_img){
	//printf("Welcome to on_screen\n");

	int y, x;
	int y_offset, x_offset;
	volatile uint64_t *ptr_image = (uint64_t *)(img);
	volatile uint64_t *ptr_labels_overlay = (uint64_t *)(OVERLAYS_LIST);

  if (mode == BYPASS)										//Sélection de l'étiquette en fonction du filtre choisi
  {
    printf("\nPainting BYPASS overlay.\n");
    //L'image à l'indice 10 correspond à l'overlay du bypass
    ptr_labels_overlay = ptr_labels_overlay + 10*OVERLAY_WIDTH*OVERLAY_HEIGHT/8; // on decale pour sauter les etiquettes des classes du CNN
    y_offset = 0;
    x_offset = 0;
  }
  else if (mode == CNN_CLASSIFIER)
  {
    printf("\nPainting CNN CLASS overlay\n");
    //L'image aux indices 0 à 9 correspondent aux overlays des différentes classes du CNN
    ptr_labels_overlay = ptr_labels_overlay + class*OVERLAY_WIDTH*OVERLAY_HEIGHT/8 ;
    y_offset = 0;
    x_offset = 0;
  }
  else if (mode == EDGE_DETECTOR)
  {
    printf("\nPainting the FILTER overlay\n");
    //L'image à l'indice 11 correspond à l'overlay du edge detector
    ptr_labels_overlay = ptr_labels_overlay + 11*OVERLAY_WIDTH*OVERLAY_HEIGHT/8 ; //apres les etiquettes des classes
    y_offset = 0;
    x_offset = 0;
  }

  for (y = 0; y < 480; y++)						//Affichage de l'image
  {
    for (x = 0; x < 640 / 8; x++)
    {
      if ( (x<=OVERLAY_WIDTH/8)&&(y<=OVERLAY_HEIGHT))
      { //on verifie si on est dans la zone de l'etiquette
        hid_new_vga_ptr[x + y * 640 / 8] = (*ptr_labels_overlay);
        ptr_labels_overlay++;
      }
      else
      {
        hid_new_vga_ptr[x + y * 640 / 8] = (*ptr_image);
      }
      ptr_image++;
    }
  }
}



void display(int img_in_number, filter_type filter_nb, uint8_t previous_imageSel, uint8_t previous_filterSel) //, uint8_t *edgeDetectorDone, uint8_t *CNNDone)
{
	volatile uint64_t *display_ptr;
	volatile uint64_t *diplay_ptr_filtered;
	volatile uint8_t *ptr_selected_img;
	volatile uint8_t *ptr_selected_img_filtered;

	// Get the image to print
	display_ptr = (uint64_t *)(TAB_GS[img_in_number - 1]);
	ptr_selected_img = (uint8_t *)(TAB_GS[img_in_number - 1]);
	diplay_ptr_filtered = (uint64_t *)(TAB_GS_FILTERED[img_in_number - 1]);
	ptr_selected_img_filtered = (uint8_t *)(TAB_GS[img_in_number - 1]);

	int x, y;

  switch (filter_nb)				//Disjonction de cas en fonction du filtre sélectionné
  {

  case BYPASS:
    on_screen( BYPASS, 10, ptr_selected_img );
    break;

  case EDGE_DETECTOR:
    on_screen(EDGE_DETECTOR, 11,ptr_selected_img_filtered );
    break;

  case CNN_CLASSIFIER:
    // In this case we visualize the image, while computing ...
    display_ptr = (uint64_t *)(TAB_GS[img_in_number - 1]);
    
    for (y = 0; y < 480; ++y)
    {
      for (x = 0; x < 640 / 8; ++x)
      {
        hid_new_vga_ptr[x + y * 640 / 8] = (*display_ptr);
	      display_ptr++;
      }
    }
    // Launch the CNN
    int result = perform_cnn(img_in_number) ;
    // When finished, show the LABEL as an overlay.
    on_screen( CNN_CLASSIFIER, result, ptr_selected_img );
    break;
  }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////  Fonction Main //////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(void)
{
	// Start by initializing everything, just in case.
	init_csrs();

	// SHow about message
	about();

	int x = 0;
	int l = 0;
	int n_image = MIN_IMAGES_TO_READ;
	int tab_size[NB_IMAGES_TO_BE_READ] = {0};   //Tableau de toutes les tailles d'images chargees
	int tab_width[NB_IMAGES_TO_BE_READ] = {0};  //Tableau de toutes les largeur d'images chargees
	int tab_length[NB_IMAGES_TO_BE_READ] = {0}; //Tableau de toutes les longueurs d'images chargees

	// Register work area to the default drive
	if (f_mount(&FatFs, "", 1))
	{
		printf("Fail to mount SD driver!\n");
		return 0;
	}

	printf("Number of images to read : %d,    MIN = %d    MAX = %d\n", NB_IMAGES_TO_BE_READ, MIN_IMAGES_TO_READ, MAX_IMAGES_TO_READ);

	// MIN and MAX are included
	int r = 0;
	for (r = n_image; r <= NB_IMAGES_TO_BE_READ; r++)				// Lire chaque image et les stocker dans global_tab
	{

		read_pic(r, tab_size,tab_width,tab_length, global_tab);

	}


	if (f_mount(NULL, "", 1))
	{
		// unmount it
		printf("fail to umount disk!");
		return 0;
	}

	// All images loaded, grayscale conversion now.

  // Start the application: {filtering | no filtering} + on_screen
  //for ( ... )				//Pour chaque image de global_tab, appliquer le greyscale et stocker le résultat dans TAB_GS
  //{
  //	... ;
  //}
  for (r = n_image; r <= NB_IMAGES_TO_BE_READ; r++)					//Pour chaque image de global_tab, appliquer le greyscale et stocker le résultat dans TAB_GS
  {
    convert_to_greyscale(r, tab_size,tab_width,tab_length, global_tab, TAB_GS[r]) ;
  }



	// FILTERING STUFF
	printf("Starting filtering!\n");
	for (int fsobel = 0; fsobel < NB_IMAGES_TO_BE_READ; fsobel++)                           //Pour chaque image de TAB_GS, appliquer la convolution et les stocker dans TAB_GS_FILTERED
	{
		convolution_filter(TAB_GS[fsobel], kernel, biaises, TAB_GS_FILTERED[fsobel]) ;
	}
	printf("Filtering done !\n");



	// Activate the Button inputs
	init_csrs();
	enable_plic_interrupts();

	extern volatile int imageSel;
	extern volatile int filterSel;
	extern volatile int isBouncing;

	imageSel = 0;
	filterSel = 0;
	uint8_t previous_imageSel = -1;
	uint8_t previous_filterSel = -1;

	uint8_t edgeDetectorDone = 0;
	uint8_t CNNDone = 0;

	volatile unsigned int ii;

  while (1)
  {
    if ( (imageSel!= previous_imageSel)||(filterSel!=previous_filterSel))    //Comparaison des valeurs courantes et précédentes des variables de sélection de l'image et du filtre
    {
      if ( imageSel==BYPASS )
      {
        edgeDetectorDone = 0;
        CNNDone = 0;
      }
      printf("imageSel = %d     filterSel = %d \n", imageSel, filterSel);
      display( imageSel, filterSel, previous_imageSel, previous_filterSel );		//Si différence, maise à jour de l'affichage

      previous_imageSel = imageSel ;						//Mise à jour de des valeurs de previous_imageSel et previous_filterSel en fonction des valeurs courantes
      previous_filterSel = filterSel ;
    }

		ii = 10000;
		while (ii--)
		{
			isBouncing = 0;
		}
	}
}











///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void about()
{
	printf("----- DEMO DATE 2020 - University Booth --------\n");
	printf(" by Noureddine Ait Said, and the PHELMA students\n");
	printf("          Supervised by Mounir Benabdenbi       \n");
	printf("            AMfoRS Team, TIMA Laboratory        \n");
	printf("------------------------------------------------\n");
	printf("      Version 1.0  Built @" __TIMESTAMP__ "     \n");
	printf("------------------------------------------------\n");
	printf("\n");
	printf("(#################################################\n");
	printf("(#################################################\n");
	printf("(###########,,/((//*,*/.(###((######(#############\n");
	printf("(##########*/((((((((((((((((((*,*/((((,(#########\n");
	printf("(##########,((((((((((((((((((((((((((((.#########\n");
	printf("(#########,/((((((((((((((((((((((((((((*.,,,,,*##\n");
	printf("(#####(**,/(((((((((((((((((((((((((((((((((((((/,\n");
	printf("(###(./(((((((((((((((((((((((((((((((((((((((((((\n");
	printf("(,/((((((((((((((((((((((((((((((((((((((/,       \n");
	printf("/*((((((((((/////((((((((((((((((///(((.  ./(,    \n");
	printf(" /(((((((////////((((((((((((/.      ////,   ,    \n");
	printf("/(/*,     ,//////(((///.      .*,    *.   *(*     \n");
	printf("          ,/(//*/*     .,    ,((*    ,     ./((((/\n");
	printf("   .*((/*./*    /*    /((.   ,((*    /(((((((((((/\n");
	printf("/(((/     /*    /*    /((.   ,(((((((((((((((((((/\n");
	printf("/(((/     /*    /*    /((((((((((((((((((((((((((/\n");
	printf("/(((/     /*    ///((((((((((((((((((((((((((((((/\n");
	printf("/(((/     /((((((((((((((((((((((((((((((((((((((/\n");
	printf("/((((/(((((((((((((((((((((((((((((((((((((((((((/\n\n\n");
}





int lowrisc_init(unsigned long addr, int ch, unsigned long quirks);
void tohost_exit(long code);

unsigned long get_tbclk(void)
{
	unsigned long long tmp = 1000000;
	return tmp;
}

char *env_get(const char *name)
{
	return (char *)0;
}

int init_mmc_standalone(int sd_base_addr);

DSTATUS disk_initialize(uint8_t pdrv)
{
	printf("\nu-boot based first stage boot loader\n");
	init_mmc_standalone(sd_base_addr);
	return 0;
}

int ctrlc(void)
{
	return 0;
}

void *find_cmd_tbl(const char *cmd, void *table, int table_len)
{
	return (void *)0;
}

unsigned long timer_read_counter(void)
{
	return read_csr(0xb00) / 10;
}

void __assert_fail(const char *__assertion, const char *__file,
				   unsigned int __line, const char *__function)
{
	printf("assertion %s failed, file %s, line %d, function %s\n", __assertion, __file, __line, __function);
	tohost_exit(1);
}

void *memalign(size_t alignment, size_t size)
{
	char *ptr = malloc(size + alignment);
	return (void *)((-alignment) & (size_t)(ptr + alignment));
}

int do_load(void *cmdtp, int flag, int argc, char *const argv[], int fstype)
{
	return 1;
}

int do_ls(void *cmdtp, int flag, int argc, char *const argv[], int fstype)
{
	return 1;
}

int do_size(void *cmdtp, int flag, int argc, char *const argv[], int fstype)
{
	return 1;
}

DRESULT disk_read(uint8_t pdrv, uint8_t *buff, uint32_t sector, uint32_t count)
{
	while (count--)
	{
		read_block(buff, sector++);
		buff += 512;
	}
	return FR_OK;
}

DRESULT disk_write(uint8_t pdrv, const uint8_t *buff, uint32_t sector, uint32_t count)
{
	return FR_INT_ERR;
}

DRESULT disk_ioctl(uint8_t pdrv, uint8_t cmd, void *buff)
{
	return FR_INT_ERR;
}

DSTATUS disk_status(uint8_t pdrv)
{
	return FR_INT_ERR;
}

void part_init(void *bdesc)
{
}

void part_print(void *desc)
{
}

void dev_print(void *bdesc)
{
}

unsigned long mmc_berase(void *dev, int start, int blkcnt)
{
	return 0;
}

unsigned long mmc_bwrite(void *dev, int start, int blkcnt, const void *src)
{
	return 0;
}

const char version_string[] = "LowRISC minimised u-boot for SD-Card";






