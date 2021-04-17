#include "stdafx.h"

#include "pimp.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace pimp;

void Image::ProcessImg(Image &img)
{
    img.loadJPG();

    img.allEffectsOptimized();
    //img.doubleSizeBilinearInterp();
    img.bilinearInterpScalation(2.0f);

    img.saveAsPNG();
    img.reset();
}

Image::Image()
{
    name_ = "";
    pix_ = nullptr;
    width_ = 0;
    height_ = 0;
}

Image::Image(std::string name) : name_(name)
{
    pix_ = nullptr;
    width_ = 0;
    height_ = 0;
}

Image::~Image()
{
    if (pix_) free(pix_);
}

void Image::reset()
{
    free(pix_);
    pix_ = nullptr;
    width_ = 0;
    height_ = 0;
}

int Image::loadJPG()
{
    std::string path = "../SampleImages/" + name_ + ".JPG";
    return loadJPG(path);
}

int Image::loadJPG(std::string path)
{
    int channels;
    pix_ = stbi_load(path.c_str(), &width_, &height_, &channels, 4);
    if (pix_ == nullptr)
    {
        printf("Error while loading the image\n");
        return PIMP_ERROR;
    }

    return PIMP_SUCCESS;
}

int Image::saveAsPNG()
{
    printf("Exporting %s as PNG...\n", name_.c_str());
    std::string path = "../Output/" + name_ + ".PNG";
    return saveAsPNG(path);
}

int Image::saveAsPNG(std::string path)
{
    stbi_write_png_compression_level = 4;
    int err = stbi_write_png(path.c_str(), width_, height_, 1, pix_, width_);
    if (err == 0)
    {
        printf("Save as PNG failed for image %s\n", name_.c_str());
        return PIMP_ERROR;
    }
    printf("Finished with %s\n", name_.c_str());
    return PIMP_SUCCESS;
}

void Image::saveAsJPG()
{
    std::string path = "../Output/" + name_ + ".JPG";
    stbi_write_jpg(path.c_str(), width_, height_, 3, pix_, 100);
}

std::string Image::name() const
{
    return name_;
}

#if PIMP_GPU == 0

void Image::bilinearInterpScalation(float amount)
{
    int new_width = width_ * amount;
    int new_height = height_ * amount;
    unsigned char* dst = (unsigned char*)malloc(new_width * new_height);
    printf("Scaling %s...\n", name_.c_str());
    if (!dst)
    {
        printf("Could not allocate space for scale %s\n", name_.c_str());
        return;
    }
    for (int y = 0; y < new_height; ++y)
    {
        for (int x = 0; x < new_width; ++x)
        {
            // Destination pixel index
            int i = (y * new_width + x);

            // Source relative position with offset (decimals)
            float gx = min(x / (float)(new_width) * (width_) - 0.5f, width_ - 2);
            float gy = min(y / (float)(new_height) * (height_) - 0.5f, height_ - 2);
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

void Image::doubleSizeBilinearInterp()
{
    int new_width = width_ * 2;
    int new_height = height_ * 2;
    unsigned char* dst = (unsigned char*)malloc(new_width * new_height);
    printf("Scaling %s...\n", name_.c_str());
    while (!dst)
    {
        dst = (unsigned char*)malloc(new_width * new_height);
    }
    for (int y = 0; y < height_ - 1; ++y)
    {
        for (int x = 0; x < width_; ++x)
        {
            int isrc = y * width_ + x;
            int idst = (y * new_width + x) * 2; // = (y*2*new_width+x*2) * 3

            // Getting corner colors for interpolation
            unsigned char tl = pix_[isrc];
            unsigned char tr = pix_[isrc + 1];
            unsigned char bl = pix_[isrc + width_];
            unsigned char br = pix_[isrc + width_ + 1];

            // Pix TL
            unsigned char pix_tl = pix_[isrc];
            unsigned char pix_tr = LerpHalf(tl, tr);
            unsigned char pix_bl = LerpHalf(tl, bl);
            unsigned char pix_br = BlerpHalf(tl, tr, bl, br);

            dst[idst] = pix_tl;
            dst[idst + 1] = pix_tr;
            dst[idst + new_width] = pix_bl;
            dst[idst + new_width + 1] = pix_br;
        }
    }
    free(pix_);
    pix_ = dst;
    width_ = new_width;
    height_ = new_height;
}

void Image::allEffectsOptimized()
{
    unsigned char* dst = (unsigned char*)malloc(width_ * height_);
    printf("Transforming %s...\n", name_.c_str());
    if (!dst)
    {
        printf("Could not allocate space for rotate on %s\n", name_.c_str());
        return;
    }

    for (int y = 0; y < height_; ++y)
    {
        for (int x = 0; x < width_; ++x)
        {
            int i = (y * width_ + x) * 3;
            int idst = x * height_ + (height_ - 1 - y);

            // Grayscale
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
