/*
 * Copyright (c) 2009 Kacuki UCHIDA
 * Licensed under the MIT License
 *   http://www.opensource.org/licenses/mit-license.php
 */
#ifndef NANOTHUMB_H
#define NANOTHUMB_H

#include <cstdio>
#include <cstdlib>
#include <jpeglib.h>


namespace nanothumb {

class Gic {
public:
    JSAMPARRAY data;
    int width;
    int height;
};

Gic *convert(Gic *img, double scale);
Gic *gic_jpeg_open(char *filename);
Gic *gic_create_image(int width, int height);
int gic_write_image(Gic *img, char *filename, int quality);

Gic *convert(Gic *img, double scale)
{
    int to_width, to_height, from_width, from_height;

    from_width = img->width;
    from_height = img->height;
    to_width = img->width * scale;
    to_height = img->height * scale;
    Gic *to_img = gic_create_image(to_width, to_height);

    int x, y, w, h;
    int hw_t, hw_b, hh_t, hh_b;
    int count;
    int r, g, b;
    for (h = 0; h < to_height; h++) {
        hh_t = (h + 0) * from_height / to_height;
        hh_b = (h + 1) * from_height / to_height;

        for (w = 0; w < to_width; w++) {
            hw_t = (w + 0) * from_width / to_width;
            hw_b = (w + 1) * from_width / to_width;

            count = 0, r = 0, g = 0, b = 0;
            for (y = hh_t; y < hh_b; y++) {
                for (x = hw_t; x < hw_b; x++) {
                    r += img->data[y][x * 3 + 0];
                    g += img->data[y][x * 3 + 1];
                    b += img->data[y][x * 3 + 2];
                    count++;
                }
            }

            to_img->data[h][w * 3 + 0] = r / count;
            to_img->data[h][w * 3 + 1] = g / count;
            to_img->data[h][w * 3 + 2] = b / count;
        }
    }

    return to_img;
}


Gic *gic_jpeg_open(char *filename)
{
    int i;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *fin;
    Gic *img;

    if (!(fin = fopen(filename, "rb"))) {
        return NULL;
    }

    img = (Gic *)malloc(sizeof(Gic));

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fin);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);
    img->width = cinfo.output_width;
    img->height = cinfo.output_height;

    img->data = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * img->height);
    img->data[0] = (JSAMPROW)calloc(sizeof(JSAMPLE), 3 * img->width * img->height);
    for (i = 1; i < img->height; i++) {
        img->data[i] = img->data[0] + 3 * img->width * i;
    }

    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, img->data + cinfo.output_scanline, cinfo.output_height - cinfo.output_scanline);
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(fin);
    return img;
}

Gic *gic_create_image(int width, int height)
{
    int i;

    Gic *img = (Gic *)malloc(sizeof(Gic));
    img->width = width;
    img->height = height;
    img->data = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * height);
    img->data[0] = (JSAMPROW)calloc(sizeof(JSAMPLE), 3 * width * height);

    for (i = 1; i < height; i++) {
        img->data[i] = img->data[0] + 3 * width * i;
    }

    return img;
}

int gic_write_image(Gic *img, char *filename, int quality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *outfile;

    outfile = fopen(filename, "wb");
    if (!outfile) {
        return -1;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = img->width;
    cinfo.image_height = img->height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);
    jpeg_write_scanlines(&cinfo, img->data, img->height);

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    fclose(outfile);
    return 0;
}
}

#endif
