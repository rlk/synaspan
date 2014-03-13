# Synthetic All-Sky Panorama

Copyright &copy; 2014 -- [Robert Kooima](http://kooima.net)

SYNASPAN is a short C program that synthesizes an all-sky panorama from Hipparcos and Tycho-2 stellar catalog data.

    Usage: synaspan [-g] [-H dat] [-T dat] [-o tif] [-w w] [-h h] [-s s] [-m m]
        -H NONE .......... Hipparcos catalog
        -T NONE .......... Tycho-2 catalog
        -o out.tif ....... output TIFF file name
        -w 4096 .......... output width
        -h 2048 .......... output height
        -s 1.0 ........... star shape standard deviation
        -m 6.0 ........... magnitude of 1-pixel star
        -g ............... output in galactic coordinates

Stars are rendered as 2D Gaussian functions. The `-s` option gives the standard deviation and thus determines the size of each star. The `-m` option determines the overall brightness of the rendering by giving the stellar magnitude of a star that would be rendered with a "volume" of 1. Specifically, a star with magnitude `m` will be rendered as a 2D Gaussian function with standard deviation `s` and integral 1. All other stars will be scaled relative to this definition. Increase `s` to make the stars bigger and increase `m` to make them brighter.

The output is presented in equirectangular projection, with stars near the poles  distorted for correct reconstruction during reprojection. The equatorial coordinate system is used by default, and the `-g` option requests the galactic coordinate system instead. Output format is 32-bit floating point RGB TIFF.

The raw Hipparcos and Tycho-2 catalogs may be downloaded from the [Strasbourg Astronomical Data Center](http://cdsweb.u-strasbg.fr). The Hipparcos catalog resides in the gzipped file `hip_main.dat` [here](ftp://cdsarc.u-strasbg.fr/pub/cats/I/239). The Tycho-2 catalog resides in the segmented gzipped file `tyc2.dat` [here](ftp://cdsarc.u-strasbg.fr/pub/cats/I/259).
