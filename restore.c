#include <stdio.h>
#include <stdlib.h>

void recover(char *file);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Invalid use.\n");
        return 1;
    }

    recover(argv[1]);
}

void recover(char *file_name)
{
    FILE *f1 = fopen(file_name, "rb");
    if (f1 == NULL)
    {
        printf("Unable to open %s\n", file_name);
        fclose(f1);
        return;
    }

    unsigned char buffer[512];

    int nf_number = 0; // new file number
    char nf_name[100]; // new file name
    FILE *nf = NULL;

    while (fread(buffer, 512, 1, f1) == 1)
    {
        if (buffer[0] == 0xff && buffer[1] == 0xd8 && buffer[2] == 0xff &&
            (buffer[3] & 0xf0) == 0xe0)
        {
            if (nf_number > 0)
            {
                fclose(nf);
            }

            snprintf(nf_name, sizeof(nf_name), "%03d.jpg", nf_number);
            nf_number++;

            nf = fopen(nf_name, "wb");

            if (nf == NULL)
            {
                printf("Unable to create %s\n", nf_name);
                return;
            }

            fwrite(buffer, 512, 1, nf);
        }
        else if (nf_number > 0)
        {
            fwrite(buffer, 512, 1, nf);
        }
    }

    if (nf != NULL)
    {
        fclose(nf);
    }

    fclose(f1);
}
