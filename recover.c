/**
 * Recovers deleted JPEGs images from memory cards
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./recover image\n");
        return 1;
    }

    // check whether the image can be opened
    char *image = argv[1];
    FILE *file = fopen(image, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", image);
        return 2;
    }

    // find how many blocks of 512 bytes the file has
    fseek(file, 0L, SEEK_END);
    int total = ftell(file);
    rewind(file);

    //initialize counter in order to make JPEG's file names
    int count = 0;

    // initialize pointer to outfile
    FILE* outfile = NULL;

    // set pseudo-boolean for later checking
    int running = -1;

    // iterate over blocks of 512 bytes from the forensic image
    for (int i = 0; i < total;)
    {
        // create buffer and read file into it in blocks of 512
        unsigned char buffer[512];
        int size = fread(buffer, 1, 512, file);

        // check if it is a start of a JPEG file
        if (buffer[0] == 0xff &&
            buffer[1] == 0xd8 &&
            buffer[2] == 0xff &&
            (buffer[3] & 0xf0) == 0xe0)
        {
            // if so, create a name for the outfile
            char filename[7];
            if (count < 10)
            {
                sprintf(filename, "00%d.jpg", count);
            }
            else if (count >= 10 && count < 100)
            {
                sprintf(filename, "0%d.jpg", count);
            }
            else if (count > 100)
            {
                sprintf(filename, "%d.jpg", count);
            }
            count++;

            // open the outfile
            outfile = fopen(filename, "w");

            // write the content of the buffer into the outfile
            fwrite(buffer, 1, size, outfile);

            // set the pseudo-boolean to inform that the progress began
            running = 1;
        }
        else
        {
            // if reach end of the file, write buffer and close outfile
            if (size < 512 && running == 1)
            {
                fwrite(buffer, 1, size, outfile);
                fclose(outfile);
            }
            // if it is not the beginning nor the end, just write
            else if (size == 512 && running == 1)
            {
                fwrite(buffer, 1, size, outfile);
            }
        }
        i += size;
    }
    // close image
    fclose(file);

    // success
    return 0;
}
