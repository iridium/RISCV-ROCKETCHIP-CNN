#include <stdio.h>


void MaxPool(float* Matrix_In, float* Matrix_Out, int* stride, int sizex, int sizey, int sizec)
{
	int aux1 = 0;

	for (int d = 0; d < sizec; d++)
	{
		int range1 = stride[0];
		int range2 = stride[1];
		for (int l = 0; l < sizex; l += stride[0])
		{
			for (int c = 0; c < sizey; c += stride[1])
			{
				int pixels[range1 * range2];
				int aux = 0;
				for (int s1 = 0; s1 < range1; s1++)
				{
					for (int s2 = 0; s2 < range2; s2++)
					{
						int a = l * sizey + s1 * sizey;
						int b = c + s2;

						pixels[aux] = Matrix_In[a + b + d * sizex * sizey];

						aux += 1;
					}
				}

				int max_value = pixels[0];
				for (int i = 1; i < range1 * range2; i++)
				{
					if (pixels[i] > max_value)
					{
						max_value = pixels[i];
					}
				}

				Matrix_Out[aux1] = max_value;
				aux1 += 1;
			}
		}
	}

}


/*

#define N 3
#define X 2
#define Y 8

int main()
{

	int Matrix_In[N * X * Y] =
	{
			1, 2, 9, 10, 7, 9, 1, 4,
			3, 4, 7, 8, 1, 5, 7, 6,

			9, 10, 13, 14, 8, 9, 1, 10,
			11, 12, 19, 20, 15, 2, 7, 9,

			17, 18, 19, 20, 1, 4, 6, 5,
			19, 20, 3, 14, 6, 7, 8, 10

	};
	int Matrix_Out[N * X / 2 * Y / 2];
	int stride[2] = { 2, 2 };
	MaxPool(Matrix_In, Matrix_Out, stride, X, Y, N);
	
	for (int i = 0; i < N * (S) * (S); i++)
	{
		printf("%d ", Matrix_In[i]);
	}


	for (int i = 0; i < N * (X/2) * (Y/2) ; i++)
	{

		printf("%d ", Matrix_Out[i]);

	}

	return 0;


}*/
