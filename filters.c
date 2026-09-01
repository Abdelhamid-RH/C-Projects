#include <stdio.h>
#include <math.h>
#include <stdlib.h>

typedef struct // pixel data structure
{
    unsigned char  Blue;
    unsigned char Green;
    unsigned char  Red;
}
px;

void cpyheader(FILE* out, FILE* photo); // copies 54 bytes of meta data
int* get_dimentions(FILE* photo); // gets heights and width located in the header
px* get_image(FILE* photo, int height, int width); // get the pixles matrix
void cpy_image(FILE* out, px* image, int height, int width); // after applying the filter we copy the image to out
void grayscale(int height, int width, px image[height][width]); // gray filter
void reflect(int height, int width, px image[height][width]); // reflect filter
void blur(int height, int width, px image[height][width]); // blur filter



int main(int argc, char* argv[])
{
    if (argc != 3) // check usage
    {
        printf("Invalid use\nCorrect use: ./filter -flag photo.bmp\n");
        return 1;
    }
    else if(argv[1][0] != '-' || (argv[1][1] != 'r' && argv[1][1] != 'g' && argv[1][1] != 'b'))
    {
        printf("Invalid flag\nSelect one of [-r , -g, -b]");
        return 1;
    }

    FILE* photo = fopen(argv[2], "rb"); // open the photo you want to modify
    if (photo == NULL)
    {
        printf("Unable to open %s\n", argv[2]);
        return 1;
    }

    FILE* out = fopen("out.bmp", "wb");  // output
    if (out == NULL)
    {
        printf("Unable to creat out.bmp\n");
        fclose(photo);
        return 1;
    }


    cpyheader(out, photo); // copy photo header to out

    int* dim = get_dimentions(photo);
    if (dim == NULL)
    {
        printf("Dimention error\n");
        fclose(photo);
        fclose(out);
        return 1;
    }

    int width = dim[0];
    int height = dim[1];
    
    free(dim);

    px* image = get_image(photo, height, width); // image is a 2d array of pixles
    if (image == NULL)
    {
        printf("Image error\n");
        fclose(photo);
        fclose(out);
        return 1;
    }

    if(argv[1][1] == 'r') // applying filters based on the flag
    {
        reflect(height, width, (px(*)[width]) image);
    }
    else if(argv[1][1] == 'g')
    {
        grayscale(height, width, (px(*)[width]) image);
    }
    else if(argv[1][1] == 'b')
    {
        blur(height, width, (px(*)[width]) image);
    }
    else // this is redundant for now, I did it in case I wanted to add other filters in the future
    {
        printf("Invalid flag\nSelect one of [-r , -g, -b]");
        fclose(photo);
        fclose(out);
        free(image);
        return 1;
    }

    cpy_image(out, image, height, width);
    
    free(image);
    
    fclose(photo);
    fclose(out);

}


void grayscale(int height, int width, px image[height][width])
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            float avg =
              round((image[i][j].Blue + image[i][j].Green + image[i][j].Red) / 3.0);
            image[i][j].Blue = avg;
            image[i][j].Green = avg;
            image[i][j].Red = avg;
        }
    }

    return;
}


void reflect(int height, int width, px image[height][width])
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width / 2; j++)
        {
            px tmp = image[i][j];
            image[i][j] = image[i][width - j - 1];
            image[i][width - j - 1] = tmp;
        }
    }

    return;
}



void blur(int height, int width, px image[height][width])
{
    px* tmp_1d = malloc(width * height * sizeof(px));
    if(tmp_1d == NULL)
    {
        printf("blur filter error\n");
        return;
    }

    px (*tmp)[width] = (px(*)[width]) tmp_1d;

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++) // for each pixel
        {
            int np_blue = 0, np_green = 0, np_red = 0;
            int c = 0; // counts number of adjacent pixles
            for(int k = -1; k <= 1; k++)
            {
                for (int l = -1; l <= 1; l++) // for each adjacent pixel p[i+1][j] p[i-1][j] p[i+1][j+1] ... 
                {
                    if(i + k >= 0 && i + k <= height - 1 && j + l >= 0 && j + l <= width - 1) // verify we are in bound so we don't get p[-1]
                    {
                        np_red += image[i + k][j + l].Red ; // calculattig new pixel colors
                        np_blue += image[i + k][j + l].Blue;
                        np_green += image[i + k][j + l].Green;
                        c++; // counter 
                    }
                }
            }
        tmp[i][j].Red = round(np_red / (float) c);
        tmp[i][j].Blue = round(np_blue / (float) c);
        tmp[i][j].Green = round(np_green / (float) c);
        }
    }

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            image[i][j] = tmp[i][j];
        }
    }

    free(tmp_1d);

    return;
}
