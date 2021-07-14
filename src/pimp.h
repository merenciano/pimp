/*
 * Lucas Merenciano Martinez
 * 
 * CONFIGURATION
 * -----------------------------
 * Set PIMP_MULTITHREADING define to 1 to process the images using threads
 * Set PIMP_GPU define to 1 to use the GPU for image processing
 * Set PIMP_VERBOSE define to 1 to see the logs (useful to see how multithreading is behaving)
 * Set any of these defines to 0 in order to switch them off
 * 
 * WHY MALLOC INSTEAD OF NEW?
 * -----------------------------
 * This library uses malloc instead of new for the image data buffers
 * because the JPG loader creates the first buffer using malloc,
 * so using malloc myself allows me to release all buffers with free()
 * without caring about who allocated them.
 */

#ifndef __PIMP_H__
#define __PIMP_H__

#define PIMP_MULTITHREADING 1
#define PIMP_GPU 1
#define PIMP_VERBOSE 1

#include <string>

namespace pimp {

const int PIMP_ERROR = 0;
const int PIMP_SUCCESS = 1;

inline float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

inline float Blerp(float tl, float tr, float bl, float br, float tx, float ty)
{
    return Lerp(Lerp(tl, tr, tx), Lerp(bl, br, tx), ty);
}

inline unsigned char LerpHalf(unsigned char a, unsigned char b)
{
    return a + (b - a) / 2;
}

inline unsigned char BlerpHalf(unsigned char tl, unsigned char tr, unsigned char bl, unsigned char br)
{
    return LerpHalf(LerpHalf(tl, tr), LerpHalf(bl, br));
}

void Close();

class Image
{
public:

    static void ProcessImg(Image& img);
    Image();
    Image(std::string name);
    ~Image();

    // Frees the pixel buffer and sets W and H to 0
    void reset();

    // Without parameters it uses the name_ attrib as filename
    int loadJPG();
    int saveAsPNG();
    int loadJPG(std::string path);
    int saveAsPNG(std::string path);

    void doubleSizeBilinearInterp();

    // Exercise effects
    void greyBrightRotate();
    void bilinearScale();

    std::string name() const;

private:
    std::string name_;
    unsigned char *pix_;
    int width_;
    int height_;
};

} // namespace pimp
#endif // __PIMP_H__