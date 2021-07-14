#include "stdafx.h"
#include "pimp.h"

// Compile this file only if GPU option is not setted
#if PIMP_GPU == 0

using namespace pimp;

void pimp::Close()
{
    // Nothing to do in while using cpu
}

void Image::bilinearScale()
{
    int new_width = width_ * 2;
    int new_height = height_ * 2;
    unsigned char* dst = (unsigned char*)malloc(new_width * new_height);
#if PIMP_VERBOSE == 1
    printf("%s: (CPU) Scaling with bilinear interpolation...\n", name_.c_str());
#endif
    if (!dst)
    {
        printf("Error %s: Could not allocate space for scale\n", name_.c_str());
        return;
    }
    for (int y = 0; y < new_height; ++y)
    {
        for (int x = 0; x < new_width; ++x)
        {
            // Destination pixel index
            int i = (y * new_width + x);

            // Source relative position with offset (decimals)
            float gx = min(x / (float)(new_width) * (width_)-0.5f, width_ - 2);
            float gy = min(y / (float)(new_height) * (height_)-0.5f, height_ - 2);
            // Top-Left pixel from source image
            int gxi = (int)gx;
            int gyi = (int)gy;

            // Getting corner colors for interpolation
            unsigned char tl = pix_[gyi * width_ + gxi];
            unsigned char tr = pix_[gyi * width_ + gxi + 1];
            unsigned char bl = pix_[(gyi + 1) * width_ + gxi];
            unsigned char br = pix_[(gyi + 1) * width_ + gxi + 1];

            dst[i] = Blerp(tl, tr, bl, br, gx - gxi, gy - gyi);
        }
    }
    free(pix_);
    pix_ = dst;
    width_ = new_width;
    height_ = new_height;
}

void Image::greyBrightRotate()
{
    unsigned char* dst = (unsigned char*)malloc(width_ * height_);
#if PIMP_VERBOSE == 1
    printf("%s: (CPU) Applying greyscale, brightness and rotating...\n", name_.c_str());
#endif
    if (!dst)
    {
        printf("Error %s: Could not allocate space for effects\n", name_.c_str());
        return;
    }

    for (int y = 0; y < height_; ++y)
    {
        for (int x = 0; x < width_; ++x)
        {
            int i = (y * width_ + x) * 3;
            int idst = x * height_ + (height_ - 1 - y);

            // Greyscale
            unsigned char value = (unsigned char)(
                pix_[i + 0] * 0.2627f +
                pix_[i + 1] * 0.6780f +
                pix_[i + 2] * 0.0593f);

            // Brightness
            dst[idst] = min((unsigned short)(value * 1.2f), 255);
        }
    }

    free(pix_);
    pix_ = dst;
    int tmp = height_;
    height_ = width_;
    width_ = tmp;
}

#endif // PIMP_GPU
