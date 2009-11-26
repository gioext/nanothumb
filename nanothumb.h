/*
 * Copyright (c) 2009 Kacuki UCHIDA
 * Licensed under the MIT License
 *   http://www.opensource.org/licenses/mit-license.php
 */
#ifndef NANOTHUMB_H
#define NANOTHUMB_H

#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <jpeglib.h>

namespace nanothumb {
    using namespace std;

    class Image {
    public:
        Image(int width, int height)
        {
            m_width = width;
            m_height = height;
            m_data = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * height);
            m_data[0] = (JSAMPROW)calloc(sizeof(JSAMPLE), 3 * width * height);

            for (int i = 1; i < height; i++) {
                m_data[i] = m_data[0] + 3 * width * i;
            }
        }

        JSAMPARRAY m_data;
        int m_width;
        int m_height;
    };

    class Conv {
    private:
        Image *m_img;

    public:
        Conv() {
        }

        ~Conv() {
            // free m_img
        }

        inline void thumb(string out, double scale, int quality) {
            int to_width, to_height, from_width, from_height;

            from_width = m_img->m_width;
            from_height = m_img->m_height;
            to_width = m_img->m_width * scale;
            to_height = m_img->m_height * scale;
            Image to_img(to_width, to_height);

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
                            r += m_img->m_data[y][x * 3 + 0];
                            g += m_img->m_data[y][x * 3 + 1];
                            b += m_img->m_data[y][x * 3 + 2];
                            count++;
                        }
                    }

                    to_img.m_data[h][w * 3 + 0] = r / count;
                    to_img.m_data[h][w * 3 + 1] = g / count;
                    to_img.m_data[h][w * 3 + 2] = b / count;
                }
            }

            write(to_img, out, quality);
        }

        inline void open(string in) {
            struct jpeg_decompress_struct cinfo;
            struct jpeg_error_mgr jerr;
            FILE *fin;

            if (!(fin = fopen(in.c_str(), "rb"))) {
                return;
            }

            cinfo.err = jpeg_std_error(&jerr);
            jpeg_create_decompress(&cinfo);
            jpeg_stdio_src(&cinfo, fin);
            jpeg_read_header(&cinfo, TRUE);
            jpeg_start_decompress(&cinfo);
            m_img = new Image(cinfo.output_width, cinfo.output_height);

            m_img->m_data = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * m_img->m_height);
            m_img->m_data[0] = (JSAMPROW)calloc(sizeof(JSAMPLE), 3 * m_img->m_width * m_img->m_height);
            for (int i = 1; i < m_img->m_height; i++) {
                m_img->m_data[i] = m_img->m_data[0] + 3 * m_img->m_width * i;
            }

            while (cinfo.output_scanline < cinfo.output_height) {
                jpeg_read_scanlines(&cinfo, m_img->m_data + cinfo.output_scanline, cinfo.output_height - cinfo.output_scanline);
            }

            jpeg_finish_decompress(&cinfo);
            jpeg_destroy_decompress(&cinfo);
            fclose(fin);
        }

        int write(Image &img, string filename, int quality)
        {
            struct jpeg_compress_struct cinfo;
            struct jpeg_error_mgr jerr;
            FILE *outfile;

            outfile = fopen(filename.c_str(), "wb");
            if (!outfile) {
                return -1;
            }

            cinfo.err = jpeg_std_error(&jerr);
            jpeg_create_compress(&cinfo);
            jpeg_stdio_dest(&cinfo, outfile);
            cinfo.image_width = img.m_width;
            cinfo.image_height = img.m_height;
            cinfo.input_components = 3;
            cinfo.in_color_space = JCS_RGB;
            jpeg_set_defaults(&cinfo);
            jpeg_set_quality(&cinfo, quality, TRUE);
            jpeg_start_compress(&cinfo, TRUE);
            jpeg_write_scanlines(&cinfo, img.m_data, img.m_height);

            jpeg_finish_compress(&cinfo);
            jpeg_destroy_compress(&cinfo);

            fclose(outfile);
            return 0;
        }
    };

    inline void thumb(string in, string out, double rate, int quality) {
        nanothumb::Conv c;
        c.open(in);
        c.thumb(out, rate, quality);
    }
}

#endif
