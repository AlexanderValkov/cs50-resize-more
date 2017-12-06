// Copies a BMP file

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "bmp.h"

int main(int argc, char *argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s multiplier infile outfile\n", argv[0]);
        return 1;
    }

    // get mulitpler
    float n = atof(argv[1]);

    // remember filenames
    char *infile = argv[2];
    char *outfile = argv[3];

    // open input file
    FILE *inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE *outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }


    // changing parameters to n times
    BITMAPFILEHEADER nbf = bf;
    BITMAPINFOHEADER nbi = bi;
    nbi.biWidth          = round((float)bi.biWidth * n);
    nbi.biHeight         = round((float)bi.biHeight * n);

    // determine padding for scanlines
    int npadding = (4 - (nbi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    nbi.biSizeImage       = (nbi.biWidth * sizeof(RGBTRIPLE) + npadding) * abs(nbi.biHeight);
    nbf.bfSize = nbi.biSizeImage + nbf.bfOffBits;

    /*
    printf("biSize is %i\n", bi.biSize);
    printf("biWidth is %i\n", bi.biWidth);
    printf("biHeight is %i\n", bi.biHeight);
    printf("biPlanes is %i\n", bi.biPlanes);
    printf("biBitCount is %i\n", bi.biBitCount);
    printf("biSizeImage is %i\n", bi.biSizeImage);
    printf("biXPelsPerMeter is %i\n", bi.biXPelsPerMeter);
    printf("biYPelsPerMeter is %i\n", bi.biYPelsPerMeter);
    printf("biClrUsed is %i\n", bi.biClrUsed);
    printf("biClrImportant is %i\n", bi.biClrImportant);
    */

    // write outfile's BITMAPFILEHEADER
    fwrite(&nbf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&nbi, sizeof(BITMAPINFOHEADER), 1, outptr);

    // determine padding for scanlines
    int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {

        RGBTRIPLE line[nbi.biWidth];
        int lineIterator = 0;
        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {
            // temporary storage
            RGBTRIPLE triple;

            float t = n;
            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
            while ( t < 1 && j < bi.biWidth)
            {
                t += t;
                j++;
                fseek(inptr, sizeof(RGBTRIPLE), SEEK_CUR);
            }

            // write RGB triple to outfile
            for (int k = 0; k < n; k++, lineIterator++ )
                line[lineIterator] = triple;
        }

        // skip over padding, if any
        fseek(inptr, padding, SEEK_CUR);

        int l = 0;
        //for (int l = 0; l < n; l++)
        do 
        {
            l++;
            for (int m = 0; m < nbi.biWidth; m++)
                fwrite(&line[m], sizeof(RGBTRIPLE), 1, outptr);

            // then add it back (to demonstrate how)
            for (int k = 0; k < npadding; k++)
            {
                fputc(0x00, outptr);
            }
        }
        while ( l < n );

        float t = n;
        while ( t < 1 )
        {
            t += t;
            fseek(inptr, bi.biWidth * sizeof(RGBTRIPLE) + padding, SEEK_CUR);
        }
    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // success
    return 0;
}
