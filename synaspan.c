/* Copyright (c) 2014 Robert Kooima                                           */
/*                                                                            */
/* Permission is hereby granted, free of charge, to any person obtaining a    */
/* copy of this software and associated documentation files (the "Software"), */
/* to deal in the Software without restriction, including without limitation  */
/* the rights to use, copy, modify, merge, publish, distribute, sublicense,   */
/* and/or sell copies of the Software, and to permit persons to whom the      */
/* Software is furnished to do so, subject to the following conditions:       */
/*                                                                            */
/* The above copyright notice and this permission notice shall be included in */
/* all copies or substantial portions of the Software.                        */
/*                                                                            */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    */
/* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    */
/* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        */
/* DEALINGS IN THE SOFTWARE.                                                  */

#include <tiffio.h>
#include <assert.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <math.h>

/*----------------------------------------------------------------------------*/

struct image
{
    float *p;
    int    w;
    int    h;
};

typedef struct image image;

void imgfree(image *I)
{
    if (I)
    {
        if (I->p) free(I->p);
        free(I);
    }
}

image *imgalloc(int w, int h)
{
    image *I;

    if ((I = (image *) malloc(sizeof (image))))
    {
        if ((I->p = (float *) calloc(3 * w * h, sizeof (float))))
        {
            I->w = w;
            I->h = h;
            return I;
        }
    }
    imgfree(I);
    return NULL;
}

void imgwrite(image *I, const char *filename)
{
    TIFF *T;
    int   y;

    if ((T = TIFFOpen(filename, "w")))
    {
        TIFFSetField(T, TIFFTAG_IMAGEWIDTH,      I->w);
        TIFFSetField(T, TIFFTAG_IMAGELENGTH,     I->h);
        TIFFSetField(T, TIFFTAG_BITSPERSAMPLE,   32);
        TIFFSetField(T, TIFFTAG_SAMPLESPERPIXEL, 3);
        TIFFSetField(T, TIFFTAG_PHOTOMETRIC,     PHOTOMETRIC_RGB);
        TIFFSetField(T, TIFFTAG_SAMPLEFORMAT,    SAMPLEFORMAT_IEEEFP);
        TIFFSetField(T, TIFFTAG_ORIENTATION,     ORIENTATION_TOPLEFT);
        TIFFSetField(T, TIFFTAG_PLANARCONFIG,    PLANARCONFIG_CONTIG);

        for (y = 0; y < I->h; y++)
            TIFFWriteScanline(T, I->p + I->w * y * 3, y, 0);

        TIFFClose(T);
    }
}

static int mod(int a, int b)
{
    return (a % b + b) % b;
}

void imgsum(image *I, int x, int y, float r, float g, float b)
{
    if (y <    0) { y =      - 1 - y; x += I->w / 2; }
    if (y > I->h) { y = I->h - 1 - y; x += I->w / 2; }

    x = mod(x, I->w);

    if (0 <= y && y < I->h)
    {
        I->p[3 * I->w * y + 3 * x + 0] += r;
        I->p[3 * I->w * y + 3 * x + 1] += g;
        I->p[3 * I->w * y + 3 * x + 2] += b;
    }
}

/*----------------------------------------------------------------------------*/

static const float halfpi = 1.5707963;
static const float     pi = 3.1415927;
static const float  twopi = 6.2831853;

static float toradians(float a)
{
    return pi * a / 180.0f;
}

/* Convert the coordinate with right ascension r and declination d from       */
/* equatorial coordinates to galactic coordinates.                            */

static void togalactic(float *r, float *d)
{
    const float c1 = toradians(282.25f) + toradians( 0.5f);
    const float c2 = toradians( 62.60f);
    const float c3 = toradians( 33.00f) - toradians(90.0f);

    float n1 =                       cosf(*d) * cosf(*r - c1);
    float n2 = sinf(*d) * sinf(c2) + cosf(*d) * sinf(*r - c1) * cosf(c2);
    float n3 = sinf(*d) * cosf(c2) - cosf(*d) * sinf(*r - c1) * sinf(c2);

    *r = c3 - atan2f(n1, n2);
    *d =       asinf(n3);

    if (*r > twopi) *r -= twopi;
    if (*r <     0) *r += twopi;
}

/* Linearly interpolate two RGB colors. */

static void lerp(float t, float *r, float *g, float *b,
                        const float *A, const float *B)
{
    *r = (1.0f - t) * A[0] + t * B[0];
    *g = (1.0f - t) * A[1] + t * B[1];
    *b = (1.0f - t) * A[2] + t * B[2];
}

/* Compute an RGB color for color index c. */

static void tocolor(float c, float *r, float *g, float *b)
{
    static const float index[7] = {
        -0.33f,
        -0.17f,
         0.15f,
         0.44f,
         0.68f,
         1.15f,
         1.64f
    };
    static const float color[7][3] = {
        { 0.608f, 0.690f, 1.000f },
        { 0.667f, 0.749f, 1.000f },
        { 0.792f, 0.843f, 1.000f },
        { 0.973f, 0.969f, 1.000f },
        { 1.000f, 0.957f, 0.918f },
        { 1.000f, 0.824f, 0.631f },
        { 1.000f, 0.800f, 0.435f }
    };

    if      (c < index[0]) lerp(c, r, g, b, color[0], color[0]);
    else if (c < index[1]) lerp(c, r, g, b, color[0], color[1]);
    else if (c < index[2]) lerp(c, r, g, b, color[1], color[2]);
    else if (c < index[3]) lerp(c, r, g, b, color[2], color[3]);
    else if (c < index[4]) lerp(c, r, g, b, color[3], color[4]);
    else if (c < index[5]) lerp(c, r, g, b, color[4], color[5]);
    else if (c < index[6]) lerp(c, r, g, b, color[5], color[6]);
    else                   lerp(c, r, g, b, color[6], color[6]);
}

/* Draw a star at right ascension r, declination d, with blue magnitude b     */
/* and visual magnitude v. The star is rendered as a Gaussian with standard   */
/* deviation s, scaled such that a star of magnitude m has a "volume" of      */
/* one pixel.                                                                 */

static void drawstar(image *I, float r, float d,
                               float b, float v,
                               float s, float m)
{
    int x = floorf(I->w * ( twopi - r) / twopi);
    int y = floorf(I->h * (halfpi - d) /    pi);

    float R;
    float G;
    float B;

    /* Calculate the magnitude and color. */

    float BV =     0.850 * (b - v);
    float V  = v - 0.090 * (b - v);

    tocolor(BV, &R, &G, &B);

    /* The intensity of a star is the "volume" of pixels it should fill. */

    float intensity = powf(10.0, (m - V) / 2.5);

    /* Compute the amplitude of that Gaussian. */

    float A = intensity / (twopi * s * s);

    /* Draw over an area of 4 sigma. */

    float c = cosf(d);
    float dx = 4.0f * s / c;
    float dy = 4.0f * s;

    int xx;
    int yy;

    for     (yy = -floorf(dy); yy < +ceil(dy); yy++)
        for (xx = -floorf(dx); xx < +ceil(dx); xx++)
        {
            float k = A * expf(-((xx * xx * c * c) / (s * s) / 2.0f +
                                 (yy * yy        ) / (s * s) / 2.0f));

            imgsum(I, x + xx, y + yy, R * k, G * k, B * k);
        }
}

/*----------------------------------------------------------------------------*/

static void tyc(image *I, const char *name, int g, float s, float m)
{
    FILE  *stream;

    if ((stream = fopen(name, "r")))
    {
        int   h;  /* Hipparcos number  */
        float r;  /* Right ascension  */
        float d;  /* Declination  */
        float b;  /* B magnitude  */
        float v;  /* V magnitude  */

        char buf[1024];

        while (fgets(buf, 1024, stream))
            if (sscanf(buf + 142, "%d", &h) == 0 &&
                sscanf(buf +  15, "%f", &r) == 1 &&
                sscanf(buf +  28, "%f", &d) == 1 &&
                sscanf(buf + 110, "%f", &b) == 1 &&
                sscanf(buf + 123, "%f", &v) == 1)
            {
                r = toradians(r);
                d = toradians(d);

                if (g) togalactic(&r, &d);

                drawstar(I, r, d, b, v, s, m);
            }

        fclose(stream);
    }
}

static void hip(image *I, const char *name, int g, float s, float m)
{
    FILE  *stream;

    if ((stream = fopen(name, "r")))
    {
        float r;  /* Right ascension  */
        float d;  /* Declination  */
        float b;  /* B magnitude  */
        float v;  /* V magnitude  */

        char buf[1024];

        while (fgets(buf, 1024, stream))
            if (sscanf(buf +  51, "%f", &r) == 1 &&
                sscanf(buf +  64, "%f", &d) == 1 &&
                sscanf(buf + 217, "%f", &b) == 1 &&
                sscanf(buf + 230, "%f", &v) == 1)
            {
                r = toradians(r);
                d = toradians(d);

                if (g) togalactic(&r, &d);

                drawstar(I, r, d, b, v, s, m);
            }

        fclose(stream);
    }
}

static int usage(const char *exe)
{
    fprintf(stderr, "Usage: %s [-g] [-H dat] [-T dat] [-o tif] [-w w] [-h h] [-s s] [-m m]\n"
                    "\t-H NONE .......... Hipparcos catalog\n"
                    "\t-T NONE .......... Tycho-2 catalog\n"
                    "\t-o out.tif ....... output TIFF file name\n"
                    "\t-w 4096 .......... output width\n"
                    "\t-h 2048 .......... output height\n"
                    "\t-s 1.0 ........... star shape standard deviation\n"
                    "\t-m 6.0 ........... magnitude of 1-pixel star\n"
                    "\t-g ............... output in galactic coordinates\n"
                    , exe);
    return -1;
}

int main(int argc, char **argv)
{
    char *T = 0;
    char *H = 0;
    char *o = "out.tif";
    int   w = 4096;
    int   h = 2048;
    int   g = 0;
    float s = 1.0f;
    float m = 6.0f;
    int   c;

    while ((c = getopt(argc, argv, "go:w:h:s:m:T:H:")) != -1)

        switch (c)
        {
            case 'T': T = optarg;                     break;
            case 'H': H = optarg;                     break;
            case 'o': o = optarg;                     break;
            case 'w': w = (int) strtol(optarg, 0, 0); break;
            case 'h': h = (int) strtol(optarg, 0, 0); break;
            case 's': s =       strtof(optarg, 0);    break;
            case 'm': m =       strtof(optarg, 0);    break;
            case 'g': g = 1;                          break;
            case '?': return usage(argv[0]);
        }

    image *I = imgalloc(w, h);

    if (T) tyc(I, T, g, s, m);
    if (H) hip(I, H, g, s, m);

    imgwrite(I, o);
    imgfree(I);

    return 0;
}
