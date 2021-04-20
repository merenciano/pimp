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

    img.greyBrightRotate();
    img.bilinearScale();

    img.saveAsPNG();
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
    stbi_write_png_compression_level = 4;
    int err = stbi_write_png(path.c_str(), width_, height_, 1, pix_, width_);
    if (err == 0)
    {
        printf("%s: Exporting as PNG failed\n", name_.c_str());
        return PIMP_ERROR;
    }
    return PIMP_SUCCESS;
}
#if 0
int Image::saveAsPNGwithlibpng(std::string path)
{
    // open file
    // note: libpng tells us to make sure we open in binary mode
    FILE* fp = fopen(path.c_str(), "wb");
    if (fp == NULL)
    {
        printf("Error: %s file cannot be opened for writing", path.c_str());
        return PIMP_ERROR;
    }

    // create png structure
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL)
    {
        // cannot create png structure
        fprintf(stderr, "cannot create png structure\n");

        // close file
        fclose(fp);
        fp = NULL;
        return false;
    }

    // create png-info structure
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
        fprintf(stderr, "cannot create png info structure\n");

        // clear png resource
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

        // close file
        fclose(fp);
        fp = NULL;
        return false;
    }

  // call this once as all relevant operations all happen in just one routine
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        fprintf(stderr, "error png's set jmp for write\n");
        /* clear png resource */
        png_destroy_write_struct(&png_ptr, &info_ptr);
        /* close file */ 
        fclose(fp);     
        fp = NULL;      
        return false;    
    }

    // set up input code
    png_init_io(png_ptr, fp);

    // set important parts of png_info first
    png_set_IHDR(png_ptr,
        info_ptr,
        width_,
        height_,
        8,
        PNG_COLOR_TYPE_GRAY,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT);
    // <... add any png_set_*() function in any order if need here

    // ready to write info we've set before actual image data
    png_write_info(png_ptr, info_ptr);

    // now it's time to write actual image data
    png_write_image(png_ptr, (png_bytepp)pix_);

    // done writing image
    // pass NULL as we don't need to write any set data i.e. comment
    png_write_end(png_ptr, NULL);

    // clear png resource
    png_destroy_write_struct(&png_ptr, &info_ptr);

    // close file
    fclose(fp);
    fp = NULL;

    return true;
}
#endif

std::string Image::name() const
{
    return name_;
}

void Image::doubleSizeBilinearInterp()
{
    int new_width = width_ * 2;
    int new_height = height_ * 2;
    unsigned char* dst = (unsigned char*)malloc(new_width * new_height);
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
