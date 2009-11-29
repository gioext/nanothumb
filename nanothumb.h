/*
 * Copyright (c) 2009 Kazuki UCHIDA
 * Licensed under the MIT License
 *   http://www.opensource.org/licenses/mit-license.php
 */
#ifndef NANOTHUMB_H
#define NANOTHUMB_H

#include <string>
#include <jpeglib.h>
#include <pthread.h>

namespace nanothumb {
  using namespace std;

  enum Color {
    RED = 0,
    GREEN,
    BLUE
  };

  class Image {
    JSAMPARRAY data_;
    int width_;
    int height_;

   public:
    inline Image(int width, int height) {
      Create(width, height);
    }

    inline Image(const string in) {
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
      Create(cinfo.output_width, cinfo.output_height);

      for (int i = 1; i < height_; i++) {
        data_[i] = data_[0] + 3 * width_ * i;
      }

      while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, data_ + cinfo.output_scanline, cinfo.output_height - cinfo.output_scanline);
      }

      jpeg_finish_decompress(&cinfo);
      jpeg_destroy_decompress(&cinfo);
      fclose(fin);
    }

    inline ~Image() {
      delete [] data_[0];
      delete data_;
    }

    inline void Create(int width, int height) {
      width_ = width;
      height_ = height;
      data_ = static_cast<JSAMPARRAY>(new JSAMPROW[height]);
      data_[0] = static_cast<JSAMPROW>(new JSAMPLE[3 * width * height]);

      for (int i = 1; i < height; i++) {
        data_[i] = data_[0] + 3 * width * i;
      }
    }

    inline int GetColor(Color c, int x, int y) {
      return data_[y][x * 3 + c];
    }

    inline void SetColor(Color c, int x, int y, int color) {
      data_[y][x * 3 + c] = color;
    }

    inline int Write(string filename, int quality)
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
      cinfo.image_width = width_;
      cinfo.image_height = height_;
      cinfo.input_components = 3;
      cinfo.in_color_space = JCS_RGB;
      jpeg_set_defaults(&cinfo);
      jpeg_set_quality(&cinfo, quality, TRUE);
      jpeg_start_compress(&cinfo, TRUE);
      jpeg_write_scanlines(&cinfo, data_, height_);

      jpeg_finish_compress(&cinfo);
      jpeg_destroy_compress(&cinfo);

      fclose(outfile);
      return 0;
    }

    inline JSAMPARRAY data() const {
      return data_;
    }

    inline int width() const {
      return width_;
    }

    inline int height() const {
      return height_;
    }
  };

  struct workp {
    int start;
    int end;
    int to_width;
    int to_height;
    Image *from_img;
    Image *to_img;
  };

  class Conv {
    Image *m_img;

   public:
    inline Conv(string in) {
      m_img = new Image(in);
    }

    inline ~Conv() {
      delete m_img;
    }

    inline void thumb(string out, double scale, int quality) {
      int to_width, to_height, from_width, from_height;

      from_width = m_img->width();
      from_height = m_img->height();
      to_width = m_img->width() * scale;
      to_height = m_img->height() * scale;
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
              r += m_img->GetColor(nanothumb::RED, x, y);
              g += m_img->GetColor(nanothumb::GREEN, x, y);
              b += m_img->GetColor(nanothumb::BLUE, x, y);
              count++;
            }
          }

          to_img.data()[h][w * 3 + 0] = r / count;
          to_img.data()[h][w * 3 + 1] = g / count;
          to_img.data()[h][w * 3 + 2] = b / count;
        }
      }

      to_img.Write(out, quality);
    }

    static void* work(void *p) {
      workp *param = (workp *)p;
      int from_width = param->from_img->width();
      int from_height = param->from_img->height();
      int to_width = param->to_width;
      int to_height = param->to_height;

      int x, y, w, h;
      int hw_t, hw_b, hh_t, hh_b;
      int count;
      int r, g, b;

      for (h = param->start; h < param->end; h++) {
        hh_t = (h + 0) * from_height / to_height;
        hh_b = (h + 1) * from_height / to_height;

        for (w = 0; w < to_width; w++) {
          hw_t = (w + 0) * from_width / to_width;
          hw_b = (w + 1) * from_width / to_width;

          count = 0, r = 0, g = 0, b = 0;
          for (y = hh_t; y < hh_b; y++) {
            for (x = hw_t; x < hw_b; x++) {
              r += param->from_img->GetColor(nanothumb::RED, x, y);
              g += param->from_img->GetColor(nanothumb::GREEN, x, y);
              b += param->from_img->GetColor(nanothumb::BLUE, x, y);
              count++;
            }
          }

          param->to_img->data()[h][w * 3 + 0] = r / count;
          param->to_img->data()[h][w * 3 + 1] = g / count;
          param->to_img->data()[h][w * 3 + 2] = b / count;
        }
      }
      return NULL;
    }

    inline void thumb2(string out, double scale, int quality) {
      int to_width = m_img->width() * scale;
      int to_height = m_img->height() * scale;
      int center = to_height * scale / 2;
      int end = to_height;
      Image to_img(to_width, to_height);

      workp p1;
      p1.start = 0;
      p1.end = center;
      p1.to_width = to_width;
      p1.to_height = to_height;
      p1.to_img = &to_img;
      p1.from_img = m_img;

      workp p2;
      p2.start = center;
      p2.end = end;
      p2.to_width = to_width;
      p2.to_height = to_height;
      p2.to_img = &to_img;
      p2.from_img = m_img;

      pthread_t th1;
      pthread_create(&th1, NULL, nanothumb::Conv::work, &p1); 
      work(&p2);

      pthread_join(th1, NULL);
      to_img.Write(out, quality);
    }
  };

  inline void thumb(string in, string out, double rate, int quality) {
    nanothumb::Conv c(in);
    c.thumb2(out, rate, quality);
  }
}

#endif
