#include "stdafx.h"
#include "pimp.h"

// Compile this file only if GPU option is setted
#if PIMP_GPU == 1
#include <amp.h>
#include <amp_graphics.h>

using namespace pimp;
using namespace concurrency;

void pimp::Close()
{
    // The application throws an exception when the
    // program exits if this function is not called
    amp_uninitialize();
}

void Image::bilinearScale()
{
    int new_width = width_ * 2;
    int new_height = height_ * 2;
    unsigned char* dst_buf = (unsigned char*)malloc(new_width * new_height);
#if PIMP_VERBOSE == 1
    printf("%s: (GPU) Scaling with bilinear interpolation...\n", name_.c_str());
#endif
    if (!dst_buf)
    {
        printf("Error %s: Could not allocate space for scale\n", name_.c_str());
        return;
    }

    extent<2> img_size(height_, width_);
    extent<2> dst_size(new_height, new_width);

    // Source Data
    graphics::texture<unsigned int, 2> data_src(img_size, pix_, img_size.size(), 8);
    graphics::texture_view<const unsigned int, 2> tex_src(data_src);

    // Result data
    graphics::texture<unsigned int, 2> data_dst(dst_size, 8);
    graphics::texture_view<unsigned int, 2> tex_dst(data_dst);

    // Loop float constants to fixed point
    unsigned int max_w_fx16 = (unsigned int)((width_  - 2.0f) * (float)(1 << 16));
    unsigned int max_h_fx16 = (unsigned int)((height_ - 2.0f) * (float)(1 << 16));
    unsigned int half_fx16 = 0.5f * (float)(1 << 16);

    // Precalculating the inverse of the size ratio so there will
    // be used a mul instead of a div inside of the inner loop
    float invwrat = (1.0f / new_width) * width_;
    float invhrat = (1.0f / new_height) * height_;
    unsigned int inv_w_ratio_fx16 = (unsigned int)(invwrat * (float)(1 << 16));
    unsigned int inv_h_ratio_fx16 = (unsigned int)(invhrat * (float)(1 << 16));

    parallel_for_each(dst_size, [=](index<2> idx) restrict(amp)
    {
        // gx and gy need at least 13 bits of integer part
        // so that is why I'm using fx16 for the algorithm
        // Getting pixel position at source scale
        unsigned int gx_fx16 = direct3d::imin(
            idx[1] * inv_w_ratio_fx16 - half_fx16, max_w_fx16);
        unsigned int gy_fx16 = direct3d::imin(
            idx[0] * inv_h_ratio_fx16 - half_fx16, max_h_fx16);
        
        // Closest top-Left pixel from source image is the integer part
        int gx = gx_fx16 >> 16;
        int gy = gy_fx16 >> 16;

        // Getting corner colors for interpolation
        index<2> isrc(gy, gx);
        unsigned int tl = tex_src(isrc);
        isrc[0] += 1;
        unsigned int bl = tex_src(isrc);
        isrc[1] += 1;
        unsigned int br = tex_src(isrc);
        isrc[0] -= 1;
        unsigned int tr = tex_src(isrc);

        // Bilinear interpolation
        // Using the fractional portion as T for lerp
        gx_fx16 &= (1 << 16);
        gy_fx16 &= (1 << 16);
        unsigned int t_fx16 = (tl << 16) + (tr - tl) * gx_fx16;
        unsigned int b_fx16 = (bl << 16) + (br - bl) * gx_fx16;
        // Strange shifts exist due to the fixed point multiplication
        // (fractional portion grows as a side effect)
        unsigned int result_fx24 = (t_fx16 << 8) + ((b_fx16 - b_fx16) >> 8) * gy_fx16;

        tex_dst.set(idx, result_fx24 >> 24);
    });
    free(pix_);
    graphics::copy(data_dst, dst_buf, dst_size.size());
    pix_ = dst_buf;
    width_ = new_width;
    height_ = new_height;
}

void Image::greyBrightRotate()
{
    unsigned char* dst = (unsigned char*)malloc(width_ * height_);
#if PIMP_VERBOSE == 1
    printf("%s: (GPU) Applying greyscale, brightness and rotating...\n", name_.c_str());
#endif
    if (!dst)
    {
        printf("Error %s: Could not allocate space for effects\n", name_.c_str());
        return;
    }

    extent<2> img_size(height_, width_);
    extent<2> dst_size(width_, height_);

    // Source
    graphics::texture<unsigned int, 2> data_src(img_size, pix_, img_size.size() * 4, 32);
    graphics::texture_view<const unsigned int, 2> tex_src(data_src);

    // Destination
    graphics::texture<unsigned int, 2> data_dst(dst_size, 8);
    graphics::texture_view<unsigned int, 2> tex_dst(data_dst);

    // Converting loop float constants to fixed point.
    // Using a fractional part of 23 bits since 9 bits are needed for the
    // color value since it can overflow
    unsigned int bright_scalar_fx7 = (unsigned int)(1.2f * (float)(1 << 7));
    // Color weights for greyscale
    unsigned int rw_fx16 = (unsigned int)(0.2627f * (float)(1 << 16));
    unsigned int gw_fx16 = (unsigned int)(0.6780f * (float)(1 << 16));
    unsigned int bw_fx16 = (unsigned int)(0.0593f * (float)(1 << 16));


    parallel_for_each(img_size, [=](index<2> idx) restrict(amp)
    {
        // Changing dst index for the rotation (matrix transpose)
        index<2> idst(idx[1], img_size[0] - 1 - idx[0]);

        // Extracting color channels, pixels are RGBX 8bit each channel.
        // The last component is unused but I had to load the JPG
        // with it because this lib can't paralelize pixels with 24 bpp
        unsigned int val = tex_src(idx);
        unsigned int r = (val >> 8)  & ((1 << 8) - 1);
        unsigned int g = (val >> 16) & ((1 << 8) - 1);
        unsigned int b = (val >> 24) & ((1 << 8) - 1);

        // Greyscale
        val = r * rw_fx16 + g * gw_fx16 + b * bw_fx16; // now val is fx16

        // +20% brightness
        // Note: bright scalar with 7 bits of decimal part because after being
        // multiplied by a fx16 the result will be fx23 
        // (9 bits are needed for the color because overflow can happen after brighten up)
        val = (val * bright_scalar_fx7) >> 23; // val goes back to fx0 (normal int)

        // Clamp in case of overflow (that's why 9 bits were needed)
        val = direct3d::imin(val, 255);

        tex_dst.set(idst, val);
    });

    free(pix_);
    graphics::copy(data_dst, dst, dst_size.size());
    pix_ = dst;
    int tmp = height_;
    height_ = width_;
    width_ = tmp;
}

#endif // PIMP_GPU