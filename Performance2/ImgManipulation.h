#ifndef __PIMP_IMG_MANIPULATION_H__
#define __PIMP_IMG_MANIPULATION_H__

namespace pimp {

struct Image
{
    unsigned char *pix;
    int width;
    int height;
};

void BilinearScale(Image in, Image *out)
{

}

void RotateClockwise(Image *img)
{
    unsigned char *dst = (unsigned char*)malloc(img->width * img->height * 3);

    for (int y = 0; y < img->height; ++y)
    {
        for (int x = 0; x < img->width; ++x)
        {
            int i = (y * img->width + x) * 3;
            int idst = (x * img->height + (img->height-1-y)) * 3;

            dst[idst + 0] = img->pix[i + 0];
            dst[idst + 1] = img->pix[i + 1];
            dst[idst + 2] = img->pix[i + 2];
        }
    }

    img->pix = dst;
    int tmp = img->height;
    img->height = img->width;
    img->width = tmp;
}

void GrayScale(Image *img)
{
    for (int y = 0; y < img->height; ++y)
    {
        for (int x = 0; x < img->width; ++x)
        {
            int i = (y * img->width + x) * 3;

            unsigned char value = (unsigned char)(img->pix[i + 0] * 0.2627f + img->pix[i + 1] * 0.6780f + img->pix[i + 2] * 0.0593f);

            img->pix[i + 0] = value;
            img->pix[i + 1] = value;
            img->pix[i + 2] = value;
        }
    }
}

void Brighten(Image *img)
{
    // Raises the overall brightness of the image by 20%
    for (int y = 0; y < img->height; ++y)
    {
        for (int x = 0; x < img->width; ++x)
        {
            int i = (y * img->width + x) * 3;

            img->pix[i + 0] = min((unsigned short)(img->pix[i + 0] * 1.2f), 255);
            img->pix[i + 1] = min((unsigned short)(img->pix[i + 1] * 1.2f), 255);
            img->pix[i + 2] = min((unsigned short)(img->pix[i + 2] * 1.2f), 255);
        }
    }
}

} // namespace pimp
#endif // __PIMP_IMG_MANIPULATION_H__