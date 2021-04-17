#ifndef __PIMP_H__
#define __PIMP_H__

#define PIMP_GPU 1

#include <string>

namespace pimp {

const int PIMP_ERROR = 0;
const int PIMP_SUCCESS = 1;

inline float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

inline unsigned char LerpHalf(unsigned char a, unsigned char b)
{
    return a + (b - a) / 2;
}

inline float Blerp(float c00, float c10, float c01, float c11, float tx, float ty)
{
    return Lerp(Lerp(c00, c10, tx), Lerp(c01, c11, tx), ty);
}

inline unsigned char BlerpHalf(unsigned char tl, unsigned char tr, unsigned char bl, unsigned char br)
{
    return LerpHalf(LerpHalf(tl, tr), LerpHalf(bl, br));
}

class Image
{
public:

    static void ProcessImg(Image& img);
    Image();
    Image(std::string name);
    ~Image();

    void reset();

    int loadJPG();
    int saveAsPNG();
    int loadJPG(std::string path);
    int saveAsPNG(std::string path);

    void saveAsJPG(); // For testing

    void grayscale(float r_weight, float g_weight, float b_weight);
    void brighten(float amount);
    void rotateClockwise();
    void bilinearInterpScalation(float amount);
    void doubleSizeBilinearInterp();

    void allEffectsOptimized();

    std::string name() const;

private:
    std::string name_;
    unsigned char *pix_;
    int width_;
    int height_;
};

} // namespace pimp
#endif // __PIMP_H__