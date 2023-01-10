#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <memory.h>
#include "encoding.h"
#include "mini-printf.h"
#include "diskio.h"
#include "ff.h"
#include "bits.h"
#include "hid.h"
#include "eth.h"
#include "elfriscv.h"
#include "lowrisc_memory_map.h"
#include "fonctions_boot.h"
#include "RESIZE_model.h"
#include <math.h>

// max size of file image is 16M
#define MAX_FILE_SIZE 0x1000000

// 4K size read burst
#define SD_READ_SIZE 4096

//Dimensions de l'ecran en sortie
//IMPORTANT : multiple de 8 (pour l'affichage)
#define IMG_WIDTH 640
#define IMG_HEIGHT 480
#define IMG_SIZE IMG_HEIGHT *IMG_WIDTH

//Nombre d'images à lire
#define MAX_CNN_IMAGES 6

//Numero de l'image a lire (doit etre inférieur a MAX_CNN_IMAGES)
#define IMAGE_TO_READ 1

/* Les fonctions suivantes ont ete ajoutees dans ce fichier car nous n'avons pas reussi a les inclures depuis les fichiers du RISC-V */

// /* Fonction servant à ajouter une chaine de caractere a une autre */
// char * strcat(char * dest, const char * src)
// {
// 	char *tmp = dest;

// 	while (*dest)
// 		dest++;
// 	while ((*dest++ = *src++) != '\0')
// 		;

// 	return tmp;
// }

// /* Fonction servant a transformer un caractere en chiffre (dans les cas ou c'est possible) */
int my_atoi(char *chaine)
{
	int res = 0;
	int i;
	for (i = 0; chaine[i] != '\0'; i++)
	{
		res = res * 10 + chaine[i] - '0';
	}
	return res;
}

// /* Fonctions utilisees par strtok */
// char * strpbrk(const char * cs,const char * ct)
// {
//   const char *sc1,*sc2;

//   for( sc1 = cs; *sc1 != '\0'; ++sc1) {
//     for( sc2 = ct; *sc2 != '\0'; ++sc2) {
//       if (*sc1 == *sc2)
// 	return (char *) sc1;
//     }
//   }
//   return NULL;
// }

// size_t strspn(const char *s, const char *accept)
// {
//   const char *p;
//   const char *a;
//   size_t count = 0;

//   for (p = s; *p != '\0'; ++p) {
//     for (a = accept; *a != '\0'; ++a) {
//       if (*p == *a)
// 	break;
//     }
//     if (*a == '\0')
//       return count;
//     ++count;
//   }

//   return count;
// }

// /* Variable globale utilisee par strtok stockant les token suivants */
// char * ___strtok;

// /* Fonction permettant de séparer une chaine de caractere en differents token stockes dans __strtok
//    Utilisation : Token = strtok(chaine de caractere, separateur)
//                  Token suivant = strtok(NULL, separateur) */
// char * strtok(char * s,const char * ct)
// {

//   char *sbegin, *send;

//   sbegin  = s ? s : ___strtok;
//   if (!sbegin) {
//     return NULL;
//   }
//   sbegin +=strspn(sbegin,ct);
//   if (*sbegin == '\0') {
//     ___strtok = NULL;
//     return( NULL );
//   }
//   send = strpbrk( sbegin, ct);
//   if (send && *send != '\0')
//     *send++ = '\0';
//   ___strtok = send;
//   return (sbegin);
// }

/* Fonction utilisee pour resoudre un bug du while
   Elle soustrait la variable a la variable max
   On teste ensuite si la valeur est nulle */
int arret(int num, int max)
{
	printf("num : %d & max : %d\n", num, max); // print necessaire pour resoudre le bug en question
	return num - max;
}

extern volatile uint64_t *const hid_new_vga_ptr; // = (volatile uint64_t *)(new_vga_base_addr);
void init_csrs()
{
	write_csr(mie, 0);
	write_csr(sie, 0);
	write_csr(mip, 0);
	write_csr(sip, 0);
	write_csr(mideleg, 0);
	write_csr(medeleg, 0);
}

// char *addrrr;

// Work area (file system object) for logical drive
FATFS FatFs;

int main()
{

	init_csrs();
	printf("Starting Main !!\n");

	// addrrr = (char *)0x4003c000;
	// *addrrr = 0xFA;

	FIL fil;	// File object
	FRESULT fr; // FatFs return code

	TCHAR *plop;
	TCHAR chaine[512];
	char *strToken = calloc(100, sizeof(*strToken));
	char *text = calloc(10000, sizeof(*text));

	printf("After calloc \n");

	int fsize = 0; // file size count
	int br;		   // Read count
	int c1 = 0;
	int c2 = 0;
	int i = 0;
	int x = 0;
	int y = 0;
	int k = 0;
	int l = 0;
	int n_image = 1;
	int length = 0;
	int width = 0;
	int size = 0;
	char file_name[30] = {0};								 //Nom du fichier a ouvrir
	uint8_t global_tab[MAX_CNN_IMAGES * IMG_SIZE * 3] = {0}; //Tableau de pixel de toutes les images rangeais les uns apres les autres
	int tab_size[MAX_CNN_IMAGES] = {0};						 //Tableau de toutes les tailles d'images chargees
	int tab_width[MAX_CNN_IMAGES] = {0};					 //Tableau de toutes les largeur d'images chargees
	int tab_length[MAX_CNN_IMAGES] = {0};					 //Tableau de toutes les longueurs d'images chargees
	uint8_t pixels[IMG_SIZE * 3];							 //Tableau de pixel pour une image
	//uint8_t *boot_file_buf = (uint8_t *)(get_ddr_base()) + ((uint64_t)get_ddr_size()) - MAX_FILE_SIZE; // at the end of DDR space
	//uint8_t *memory_base = (uint8_t *)(get_ddr_base());

	// Register work area to the default drive

	if (f_mount(&FatFs, "", 1))
	{
		printf("Fail to mount SD driver!\n");
		return 0;
	}

	printf("Number of CNN images to read : %d\n", MAX_CNN_IMAGES);

	while (1)
	{
	}

	for (n_image = 1; n_image <= MAX_CNN_IMAGES; n_image++)
	{
		//   while(n_image <= MAX_CNN_IMAGES){ //Endroit ou le bug a ete repertorie. La boucle ne s'arretais pas meme lorsque n_image >= MAX_CNN_IMAGES+1
		length = 0;
		width = 0;
		size = 0;
		c1 = 0;
		c2 = 0;
		i = 0;

		//Generation du nom de fichier
		sprintf(file_name, "cnn/%d.ppm", n_image);
		//strcat(file_name,".ppm");
		// strcat(file_name,".ppm");

		// Open a file
		printf(">> Loading %s\n", file_name);

		continue; // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

		fr = f_open(&fil, file_name, FA_READ);
		if (fr)
		{
			printf("Failed to open %s!\n", file_name);
			return 0;
		}

		//Lecture de l'entete
		fr = f_read(&fil, &c1, 1, &br);
		fr = f_read(&fil, &c2, 1, &br);

		//Si l'entete vaut les caracteres 'P3' alors, on est dans le cas d'un fichier ppm
		if (c1 == 0x50 && c2 == 0x33)
		{
			printf("Le fichier %s est un fichier ppm P3.\n", file_name);
			plop = f_gets(text, 10000, &fil);
			plop = f_gets(text, 10000, &fil);
			if (text[0] == '#')
			{ // test ligne de commentaire de openCV
				plop = f_gets(text, 10000, &fil);
			}
			strToken = strtok(text, " ");
			length = my_atoi(strToken); //Lecture de la longueur de l'image
			strToken = strtok(NULL, "\n");
			width = my_atoi(strToken); //Lecture de la largeur de l'image
			size = length * width;
			tab_width[n_image - 1] = width;
			tab_length[n_image - 1] = length;
			tab_size[n_image - 1] = size;
			for (i = 0; i < size; i++)
			{
				pixels[i] = 0;
			}
			printf("File size: %d and image size : %d * %d = %d\n", fil.fsize, tab_length[n_image - 1], tab_width[n_image - 1], tab_size[n_image - 1]);
			plop = f_gets(text, 10000, &fil);
			i = 0;
			plop = calloc(3 * size, sizeof(*plop));
			//Pour toutes les lignes du fichier
			while (&fil != NULL && i < (3 * size))
			{
				plop = f_gets(text, 10000, &fil); //On lit une ligne
				strToken = strtok(text, " ");	 //On separe les differents chiffres
				//Pour tous les chiffres de la ligne
				while (strToken != NULL && i < (3 * size))
				{
					pixels[i] = my_atoi(strToken); //On remplit le tableau pixel par pixel
					i++;
					strToken = strtok(NULL, " "); //On selectionne le token suivant
					if (strToken[0] == '\n')
					{ // On enlève les caractère de saut de ligne '\n'
						strToken = NULL;
					}
				}
			}
		}
		//Si l'entete vaut 0xffe0 alors c'est un fichier jpg
		else if (c1 == 0xff && c2 == 0xe0)
		{
			printf("Le fichier est un fichier jpg.\n");
			//NON IMPLEMENTE
			//UTILISER LA LIBRAIRIE JPG
		}
		printf("n_image = %d\n", n_image);
		for (i = 0; i < size * 3; i++)
		{
			global_tab[(n_image - 1) * IMG_SIZE * 3 + i] = pixels[i]; //On remplit le tableau global pour pouvoir reutiliser le tableau pixel
		}
		n_image += 1;
		printf("Closing file %s\n", file_name);

		// Close the file
		if (f_close(&fil))
		{
			printf("fail to close file!");
			return 0;
		}
	}

	// STOP HERE <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	while (1)
		;

	free(text);
	free(strToken);
	free(plop);

	if (f_mount(NULL, "", 1))
	{ // unmount it
		printf("Fail to umount disk!");
		return 0;
	}

	printf("Start resize\n");

	int sizeNew = 24;
	float *resized = calloc(sizeNew * sizeNew * 3, sizeof(*resized));

	resizing(resized, global_tab, IMAGE_TO_READ, tab_width[IMAGE_TO_READ - 1], tab_length[IMAGE_TO_READ - 1], sizeNew);

	printf("Start normalize\n");
	float *normalized = calloc(sizeNew * sizeNew * 3, sizeof(*resized));

	normalized = normalizing(normalized, resized, sizeNew);

	free(resized);

	printf("Affichage image numero : %d   %d*%d=%d\n", IMAGE_TO_READ, tab_width[IMAGE_TO_READ - 1], tab_length[IMAGE_TO_READ - 1], tab_size[IMAGE_TO_READ - 1]);
	//GreyScale array
	uint8_t *TAB_GS = calloc(IMG_SIZE, sizeof(*TAB_GS));

	//Transformation Greyscale
	for (int i = 0; i < tab_size[IMAGE_TO_READ - 1] * 3; i += 3)
	{ //For each pixel on R, G et B
		TAB_GS[i / 3] = (0.3 * global_tab[(IMAGE_TO_READ - 1) * IMG_SIZE * 3 + i] + 0.59 * global_tab[(IMAGE_TO_READ - 1) * IMG_SIZE * 3 + (i + 1)] + 0.11 * global_tab[(IMAGE_TO_READ - 1) * IMG_SIZE * 3 + (i + 2)]);
	}

	//POINTER on GreyScale array
	volatile uint64_t *my_ptr = (uint64_t *)(TAB_GS);

	//For each pixel of the output screen
	for (y = 0; y < IMG_HEIGHT; ++y)
	{
		for (x = 0; x < IMG_WIDTH / 8; ++x)
		{ //Pointeur sur 64 bits donc on charge 8 pixels à la foix d'ou la division par 8
			if ((y < tab_width[IMAGE_TO_READ - 1]) && (x < tab_length[IMAGE_TO_READ - 1] / 8))
			{
				hid_new_vga_ptr[x + y * IMG_WIDTH / 8] = (*my_ptr);
				my_ptr++;
			}
			else
			{
				hid_new_vga_ptr[x + y * IMG_WIDTH / 8] = 0; //Si on sort de l'image a imprimer, on envoie du noir
			}
		}
	}

	printf("=========\n   END   \n=========");
	while (1)
		;

	return 1;
}
