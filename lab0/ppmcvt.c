#include "pbm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//helper functions:
double average_rgb(PPMImage * ppm, unsigned int h, unsigned int w){
    int sum = ppm->pixmap[0][h][w] + ppm->pixmap[1][h][w] + ppm->pixmap[2][h][w];
    double average = (double)sum / 3;

    return average;
}

//transformations:
// -b: convert input file to a Portable Bitmap (PBM) file. (DEFAULT) 
void * convert_to_PBMImage (const char * infilename, const char * outfilename){
    PPMImage * ppm = read_ppmfile(infilename);
    unsigned int h = ppm->height;
    unsigned int w = ppm->width;
    unsigned int max = ppm->max;

    PBMImage * pbm = new_pbmimage(w, h);

    for(int i = 0; i < h; i++){
        for(int j = 0; j < w; j++){
            pbm->pixmap[i][j] = average_rgb(ppm, i, j) < ((double)max / 2);
        }
    }

    write_pbmfile(pbm, outfilename);
    del_ppmimage(ppm);
    del_pbmimage(pbm);
}

// -g: convert input file to a Portable Gray Map (PGM) file using the specified max grayscale pixel value [1-65535].
void * convert_to_PGMImage (const char * infilename, const char * outfilename, const int * pixel_value){
    PPMImage * ppm = read_ppmfile(infilename);
    unsigned int h = ppm->height;
    unsigned int w = ppm->width;
    unsigned int max = ppm->max;

    PGMImage * pgm = new_pgmimage(w, h, *pixel_value);

    for(int i = 0; i < h; i++){
        for(int j = 0; j < w; j++){
            pgm->pixmap[i][j] = (int)(average_rgb(ppm, i, j) / max * (*pixel_value));
        }
    }

    write_pgmfile(pgm, outfilename);
    del_ppmimage(ppm);
    del_pgmimage(pgm);
}

// -i: isolate the specified RGB channel. Valid channels are “red”, “green”, or “blue”.
void * isolate_RGB_channel (const char * infilename, const char * outfilename, const char * channel){
    PPMImage * in_ppm = read_ppmfile(infilename);
    unsigned int h = in_ppm->height;
    unsigned int w = in_ppm->width;
    unsigned int max = in_ppm->max;

    PPMImage * out_ppm = new_ppmimage(w, h, max);

    if(strcmp(channel, "red") == 0){
        for(int i = 0; i < h; i++){
            for(int j = 0; j < w; j++){
                out_ppm->pixmap[0][i][j] = in_ppm->pixmap[0][i][j];
                out_ppm->pixmap[1][i][j] = 0;
                out_ppm->pixmap[2][i][j] = 0;
            }
        }
    }
    else if(strcmp(channel, "green") == 0){
        for(int i = 0; i < h; i++){
            for(int j = 0; j < w; j++){
                out_ppm->pixmap[0][i][j] = 0;
                out_ppm->pixmap[1][i][j] = in_ppm->pixmap[1][i][j];
                out_ppm->pixmap[2][i][j] = 0;
            }
        }
    }
    else { //blue
        for(int i = 0; i < h; i++){
            for(int j = 0; j < w; j++){
                out_ppm->pixmap[0][i][j] = 0;
                out_ppm->pixmap[1][i][j] = 0;
                out_ppm->pixmap[2][i][j] = in_ppm->pixmap[2][i][j];
            }
        }
    }

    write_ppmfile(out_ppm, outfilename);
    del_ppmimage(in_ppm);
    del_ppmimage(out_ppm);
}

// -r: remove the specified RGB channel. Valid channels are “red”, “green”, or “blue”.
void * remove_RGB_channel (const char * infilename, const char * outfilename, const char * channel){
    PPMImage * in_ppm = read_ppmfile(infilename);
    unsigned int h = in_ppm->height;
    unsigned int w = in_ppm->width;
    unsigned int max = in_ppm->max;

    PPMImage * out_ppm = new_ppmimage(w, h, max);

    if(strcmp(channel, "red") == 0){
        for(int i = 0; i < h; i++){
            for(int j = 0; j < w; j++){
                out_ppm->pixmap[0][i][j] = 0;
                out_ppm->pixmap[1][i][j] = in_ppm->pixmap[1][i][j];
                out_ppm->pixmap[2][i][j] = in_ppm->pixmap[2][i][j];
            }
        }
    }
    else if(strcmp(channel, "green") == 0){
        for(int i = 0; i < h; i++){
            for(int j = 0; j < w; j++){
                out_ppm->pixmap[0][i][j] = in_ppm->pixmap[0][i][j];
                out_ppm->pixmap[1][i][j] = 0;
                out_ppm->pixmap[2][i][j] = in_ppm->pixmap[2][i][j];
            }
        }
    }
    else{ //blue
        for(int i = 0; i < h; i++){
            for(int j = 0; j < w; j++){
                out_ppm->pixmap[0][i][j] = in_ppm->pixmap[0][i][j];
                out_ppm->pixmap[1][i][j] = in_ppm->pixmap[1][i][j];
                out_ppm->pixmap[2][i][j] = 0;
            }
        }
    }

    write_ppmfile(out_ppm, outfilename);
    del_ppmimage(in_ppm);
    del_ppmimage(out_ppm);
}

// -s  apply a sepia transformation
void * sepia_transformation (const char * infilename, const char * outfilename){
    PPMImage * in_ppm = read_ppmfile(infilename);
    unsigned int h = in_ppm->height;
    unsigned int w = in_ppm->width;
    unsigned int max = in_ppm->max;
    double old_red, old_green, old_blue;
    double new_red, new_green, new_blue;
    
    PPMImage * out_ppm = new_ppmimage(w, h, max);

    for(int i = 0; i < h; i++){
        for(int j = 0; j < w; j++){
            old_red = (double)(in_ppm->pixmap[0][i][j]);
            old_green = (double)(in_ppm->pixmap[1][i][j]);
            old_blue = (double)(in_ppm->pixmap[2][i][j]);

            new_red = (.393 * old_red + .769 * old_green + .189 * old_blue);
            new_green = (.349 * old_red + .686 * old_green + .168 * old_blue);
            new_blue = (.272 * old_red + .534 * old_green + .131 * old_blue);

            if(new_red >= max){
                new_red = max;
            }
            if(new_green >= max){
                new_green = max;
            }
            if(new_blue >= max){
                new_blue = max;
            }

            out_ppm->pixmap[0][i][j] = (unsigned int)new_red;
            out_ppm->pixmap[1][i][j] = (unsigned int)new_green;
            out_ppm->pixmap[2][i][j] = (unsigned int)new_blue;
        }
    }

    write_ppmfile(out_ppm, outfilename);
    del_ppmimage(in_ppm);
    del_ppmimage(out_ppm);
}

// -m vertically mirror the first half of the image to the second half
void * mirror_image (const char * infilename, const char * outfilename){
    PPMImage * in_ppm = read_ppmfile(infilename);
    unsigned int h = in_ppm->height;
    unsigned int w = in_ppm->width;
    unsigned int max = in_ppm->max;

    PPMImage * out_ppm = new_ppmimage(w, h, max);

    for(int i = 0; i < h; i++){
        for(int j = 0; j < w/2; j++){
            out_ppm->pixmap[0][i][j] = in_ppm->pixmap[0][i][j];
            out_ppm->pixmap[0][i][w - (j + 1)] = in_ppm->pixmap[0][i][j];
            out_ppm->pixmap[1][i][j] = in_ppm->pixmap[1][i][j];
            out_ppm->pixmap[1][i][w - (j + 1)] = in_ppm->pixmap[1][i][j];
            out_ppm->pixmap[2][i][j] = in_ppm->pixmap[2][i][j];
            out_ppm->pixmap[2][i][w - (j + 1)] = in_ppm->pixmap[2][i][j];
        }
    }

    write_ppmfile(out_ppm, outfilename);
    del_ppmimage(in_ppm);
    del_ppmimage(out_ppm);
}

// -t: reduce the input image to a thumbnail based on the given scaling factor [1-8].
void * thumbnail_image (const char * infilename, const char * outfilename, const int * scale_factor){
    PPMImage * in_ppm = read_ppmfile(infilename);
    unsigned int h = in_ppm->height;
    unsigned int w = in_ppm->width;
    unsigned int max = in_ppm->max;
    
    unsigned int n = *scale_factor;

    unsigned int new_w = w/n;
    unsigned int new_h = h/n;

    if(w % n != 0){
        new_w++;
    }

    if(h % n != 0){
        new_h++;
    }

    PPMImage * out_ppm = new_ppmimage(new_w, new_h, max);

    for(int i = 0; i < h; i+=n){
        for(int j = 0; j < w; j+=n){
            out_ppm->pixmap[0][i/n][j/n] = in_ppm->pixmap[0][i][j];
            out_ppm->pixmap[1][i/n][j/n] = in_ppm->pixmap[1][i][j];
            out_ppm->pixmap[2][i/n][j/n] = in_ppm->pixmap[2][i][j];
        }
    }
    write_ppmfile(out_ppm, outfilename);
    del_ppmimage(in_ppm);
    del_ppmimage(out_ppm);
}

// -n: tile thumbnails of the input image based on the given scaling factor [1-8].
void * tile_thumbnail_image (const char * infilename, const char * outfilename, const int * scale_factor){
    PPMImage * in_ppm = read_ppmfile(infilename);
    unsigned int h = in_ppm->height;
    unsigned int w = in_ppm->width;
    unsigned int max = in_ppm->max;

    unsigned int n = *scale_factor;

    unsigned int new_w = w/n;
    unsigned int new_h = h/n;

    if(w % n != 0){
        new_w++;
    }

    if(h % n != 0){
        new_h++;
    }

    PPMImage * out_ppm = new_ppmimage(w, h, max);

    for(int j = 0; j < h; j++){
        for(int k = 0; k < w; k++){       
            out_ppm->pixmap[0][j][k] = in_ppm->pixmap[0][ (h % new_h) * n][ (w % new_h) * n];
            out_ppm->pixmap[1][j][k] = in_ppm->pixmap[1][ (h % new_h) * n][ (w % new_h) * n];
            out_ppm->pixmap[2][j][k] = in_ppm->pixmap[2][ (h % new_h) * n][ (w % new_h) * n];
        }
    }

    write_ppmfile(out_ppm, outfilename);
    del_ppmimage(in_ppm);
    del_ppmimage(out_ppm);
}

int main(int argc, char *argv[]){
    int Flag = 0;
    int bflag = 0;
    int gflag = 0;
    int iflag = 0;
    int rflag = 0;
    int sflag = 0;
    int mflag = 0;
    int tflag = 0;
    int nflag = 0;
    int c;

    int * int_arg = malloc(sizeof(int *));
    char * char_arg = malloc(sizeof(char *));

    char * infilename;
    char * outfilename;

    //process command line:
    while ((c = getopt(argc, argv, "bg:i:r:smt:n:o:")) != -1){
        switch (c){
        case 'b':
            bflag = 1;
            Flag++;
            break;
        case 'g':
            gflag = 1;
            Flag++;
            if(strtol(optarg, NULL, 10) > 65535){
                fprintf(stderr, "Error: Invalid max grayscale pixel value: %s; must be less than 65,536\n", optarg);
                exit(1);
            }
            * int_arg = strtol(optarg, NULL, 10);
            break;
        case 'i':
            iflag = 1;
            Flag++;
            if(strcmp(optarg, "red") != 0 && strcmp(optarg, "green") != 0 && strcmp(optarg, "blue") != 0){
                fprintf(stderr, "Error: Invalid channel specification: (%s); should be 'red', 'green' or 'blue'\n", optarg);
                exit(1);
            }
            char_arg = optarg;
            break;
        case 'r':
            rflag = 1;
            Flag++;
            if(strcmp(optarg, "red") != 0 && strcmp(optarg, "green") != 0 && strcmp(optarg, "blue") != 0){
                fprintf(stderr, "Error: Invalid channel specification: (%s); should be 'red', 'green' or 'blue'\n", optarg);
                exit(1);
            }
            char_arg = optarg;
            break;
        case 's':
            sflag = 1;
            Flag++;
            break;
        case 'm':
            mflag = 1;
            Flag++;
            break;   
        case 't':
            tflag = 1;
            Flag++;
            if(strtol(optarg, NULL, 10) > 8){
                fprintf(stderr, "Error: Invalid scale factor: %d; must be 1-8\n", (int)strtol(optarg, NULL, 10));
                exit(1);
            }
            * int_arg = strtol(optarg, NULL, 10);
            break;
        case 'n':
            nflag = 1;
            Flag++;
            if(strtol(optarg, NULL, 10) > 8){
                fprintf(stderr, "Error: Invalid scale factor: %d; must be 1-8\n", (int)strtol(optarg, NULL, 10));
                exit(1);
            }
            * int_arg = strtol(optarg, NULL, 10);
            break;
        case 'o':
            outfilename = optarg;
            break;
        case '?':
            fprintf(stderr, "Usage: ppmcvt [-bgirsmtno] [FILE]\n");
            exit(1);
            break;
        default:
            fprintf(stderr, "Usage: ppmcvt [-bgirsmtno] [FILE]\n");
            exit(1);
            break;
        }
    }

    //check final conditions:
    infilename = argv[argc - 1];
    if(Flag == 0){
        bflag = 1;
        Flag++;
    }
    if(Flag > 1){
        fprintf(stderr, "Error: Multiple transformations specified\n");
    }
    if(infilename == NULL){
        fprintf(stderr, "Error: No input file specified\n");
        exit(1);
    }
    if(outfilename == NULL){
        fprintf(stderr, "Error: No output file specified\n");
        exit(1);
    }

    //check flags:
    if(bflag == 1){
        convert_to_PBMImage(infilename, outfilename);
    }
    if(gflag == 1){
        convert_to_PGMImage(infilename, outfilename, int_arg);
    }
    if(iflag == 1){
        isolate_RGB_channel(infilename, outfilename, char_arg);
    }
    if(rflag == 1){
        remove_RGB_channel(infilename, outfilename, char_arg);
    }
    if(sflag == 1){
        sepia_transformation(infilename, outfilename);
    }
    if(mflag == 1){
        mirror_image(infilename, outfilename);
    }
    if(tflag == 1){
        thumbnail_image(infilename, outfilename, int_arg);
    }
    if(nflag == 1){
        tile_thumbnail_image(infilename, outfilename, int_arg);
    }

    free(int_arg);
    free(char_arg);
    return 0;
}