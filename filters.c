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

int cpyheader(FILE* out, FILE* photo); // copies 54 bytes of meta data
int* get_dimentions(FILE* photo); // gets heights and width located in the header and returns an array [width, height]
int get_image(FILE* photo, int height, int width, px* image); // get the pixles matrix
int cpy_image(FILE* out, px* image, int height, int width); // after applying the filter we copy the image to out
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


    if (cpyheader(out, photo) == 1)
    {
        printf("copying header error.");
        fclose(photo);
        fclose(out);
        return 1;
    }

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

    px* image = malloc(height * width * sizeof(px));
    if (image == NULL)
    {
        printf("Image error\n");
        fclose(photo);
        fclose(out);
        return 1;
    }

    if(get_image(photo, height, width, image) == 1) // image is a 2d array of pixles
    {
        printf("getting image error\n");
        fclose(photo);
        fclose(out);
        free(image);
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
        int b = 5; // blur_strength, increasing b makes the blur stronger 
        for (int i = 0; i < b; i++)
        {
            blur(height, width, (px(*)[width]) image);
        }
        
    }
    else // this is redundant for now, I did it in case I wanted to add other filters in the future
    {
        printf("Invalid flag\nSelect one of [-r , -g, -b]");
        fclose(photo);
        fclose(out);
        free(image);
        return 1;
    }

    if (cpy_image(out, image, height, width) == 1)
    {
        printf("Image copying error.\n");
        fclose(photo);
        fclose(out);
        free(image);
        return 1;
    }
    
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


int cpyheader(FILE* out, FILE* photo)
{
    unsigned char buffer[54];
     if(fread(buffer, 54, 1, photo) != 1)
     {
        return 1;
     }

     if (fwrite(buffer, 54, 1, out) != 1)
     {
        return 1;
     }
     return 0;
}



int* get_dimentions(FILE* photo)
{
    // skipping the bitmap file headear and the first 4 bytes of the info header to get the width and height
    int* dim = malloc(2 * sizeof(int));
    if(dim == NULL)
    {
        return NULL;
    }

    fseek(photo, 18, SEEK_SET);
    for (int i = 0; i < 2; i++)
    {
        if(fread(dim + i, 4, 1, photo) != 1)
        {
            free(dim);
            return NULL;
        }
    }
    return dim;
}



int get_image(FILE* photo, int height, int width, px* image)
{
    int padding = 4 - (width * 3 - 4 * floor((width*3 / 4.0)));
    if (padding == 4)
    {
        padding = 0;
    }
    
    for (int i = 0; i < height; i++)
    {
        fseek(photo, 54 + i * (width * sizeof(px) + padding), SEEK_SET);

        if (fread(image + i * width, width * sizeof(px), 1, photo) != 1)
        {
            return 1;
        }
    }
    return 0;
}



int cpy_image(FILE* out, px* image, int height, int width)
{
    unsigned char buffer[] = {0, 0, 0};

    int padding = 4 - (width * 3 - 4 * floor((width*3 / 4.0)));
    if (padding == 4)
    {
        padding = 0;
    }

    for (int i = 0; i < height; i++)
    {
        unsigned char* p = buffer;

        fseek(out, 54 + i * (width * sizeof(px) + padding), SEEK_SET);

        if (fwrite(image + i * width, width * sizeof(px), 1, out) != 1)
        {
            return 1;
        }
        
        fwrite(p, 1, padding, out);
        
    }
    return 0;
}