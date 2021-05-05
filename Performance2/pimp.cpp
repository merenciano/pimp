#include "stdafx.h"

#include "pimp.h"
#include <thread>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace pimp;

void Image::ProcessImg(Image &img)
{
    img.loadJPG();

    img.greyBrightRotate();
    img.bilinearScale();

    int err = img.saveAsPNG();

    // I am the first one offended by the while loop I just wrote.
    // But in this particular case I know for sure that the memory will be avaiable
    // soon when other threads finish their task and release.
    // If the code is compiled in 64 bits this never happens but in 32 bits it does
    // when the cpu supports more than 8 threads and the images are huge (like the SampleImages).
    while (err == PIMP_ERROR)
    {
        printf("%s: Memory allocation failed while encoding to PNG, trying again in 0.5 seconds.\n", img.name_.c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        err = img.saveAsPNG();
    }
    img.reset();
#if PIMP_VERBOSE == 1
    printf("%s: Done!\n", img.name().c_str());
#endif
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
#if PIMP_VERBOSE == 1
    printf("%s: Loading JPG...\n", name_.c_str());
#endif
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
#if PIMP_VERBOSE == 1
    printf("%s: Exporting as PNG...\n", name_.c_str());
#endif
    std::string path = "../Output/" + name_ + ".PNG";
    return saveAsPNG(path);
}

int Image::saveAsPNG(std::string path)
{
    stbi_write_png_compression_level = 0;
    int err = stbi_write_png(path.c_str(), width_, height_, 1, pix_, width_);
    if (err == 0)
    {
        printf("%s: Exporting as PNG failed\n", name_.c_str());
        return PIMP_ERROR;
    }
    return PIMP_SUCCESS;
}

std::string Image::name() const
{
    return name_;
}

