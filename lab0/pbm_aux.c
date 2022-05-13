#include "pbm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

PPMImage * new_ppmimage( unsigned int w, unsigned int h, unsigned int m )
{
    //allocate memory for struct for pointer
    PPMImage * ppm;
    ppm = malloc(8 * sizeof(int*));

    //initialize struct variables
    ppm->width = w;
    ppm->height = h;
    ppm->max = m;

    //initialize pixmap
    ppm->pixmap[0] = malloc(h*sizeof(int *));
    ppm->pixmap[1] = malloc(h*sizeof(int *));
    ppm->pixmap[2] = malloc(h*sizeof(int *));
    for(int i = 0; i < h; i++){
        ppm->pixmap[0][i] = malloc(w * sizeof(int));
        ppm->pixmap[1][i] = malloc(w * sizeof(int));
        ppm->pixmap[2][i] = malloc(w * sizeof(int));
    }
    //return pointer
    return ppm;
}

PBMImage * new_pbmimage( unsigned int w, unsigned int h )
{
    //allocate memory for struct for pointer
    PBMImage * pbm;
    pbm = malloc(8 * sizeof(int*));

    //initialize struct variables
    pbm->width = w;
    pbm->height = h;

    //initialize pixmap
    pbm->pixmap = malloc(h*sizeof(int *));
    for(int i = 0; i < h; i++){
        pbm->pixmap[i] = malloc(w * sizeof(int));
    }

    //return pointer
    return pbm;
}

PGMImage * new_pgmimage( unsigned int w, unsigned int h, unsigned int m )
{
    //allocate memory for struct for pointer
    PGMImage * pgm;
    pgm = malloc(8 * sizeof(int*));

    //initialize struct variables
    pgm->width = w;
    pgm->height = h;
    pgm->max = m;

    //initialize pixmap
    pgm->pixmap = malloc(h*sizeof(int *));
    for(int i = 0; i < h; i++){
        pgm->pixmap[i] = malloc(w * sizeof(int));
    }

    //return pointer
    return pgm;
}

void del_ppmimage( PPMImage * p )
{
    for(int i = p->height - 1; i >= 0; i--){
        free(p->pixmap[2][i]);
        free(p->pixmap[1][i]);
        free(p->pixmap[0][i]);
    }
    free(p->pixmap[2]);
    free(p->pixmap[1]);
    free(p->pixmap[0]);
    free(p);
}

void del_pgmimage( PGMImage * p )
{
    for(int i = p->height - 1; i >= 0; i--){
        free(p->pixmap[i]);
    }
    free(p->pixmap);
    free(p);
}

void del_pbmimage( PBMImage * p )
{
    for(int i = p->height - 1; i >= 0; i--){
        free(p->pixmap[i]);
    }
    free(p->pixmap);
    free(p);
}

