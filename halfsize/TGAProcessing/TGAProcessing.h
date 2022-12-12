#ifndef TGAPROCESSING_H
#define TGAPROCESSING_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "../Common/Utilities.h"

#define DEBUG_FLAG          1



/**
TGA Header
More info: http://paulbourke.net/dataformats/tga/
*/
typedef struct 
{
    char  idLength;             // Image ID length of the image information field (unit byte)
    char  colourMapType;        // 0: No color table 1: With color table
    char  imageType;            // 2  -  Uncompressed, RGB images.

    short int colourMapOrigin;  // The position of the first colormap entry
    short int colourMapLength;  // Number of color table entries 
    char  colourMapDepth;       // The size of the color table entry, which can be 15, 16, 24, 32 (bit) 
     
    short int x_origin;         // Horizontal coordinates of the lower left corner of the image, low in the front and high in the back
    short int y_origin;         // Vertical coordinate of the lower left corner of the image, low in front and high in back
    short width;                // Image width (2 bytes): width in pixels
    short height;               // Image height (2 bytes): height in pixels
    char pixelDepth;            // Image color depth 8, 16, 24, 32 (bit)
    char imageDescriptor;       // Image descriptor (1 byte): bits 3-0 give the alpha channel depth, bits 5-4 give pixel ordering
} t_tgaheader;

/**
TGA Data
Channel order is BGRA
Each channel stores 8 bytes 
*/
typedef struct
{
    char* originalImagePixelData;
    char* resizedImagePixelData;   

    // De-interleaved RBG data
    std::vector<char> redChn;
    std::vector<char> greenChn;
    std::vector<char> blueChn;
    std::vector<char> alphaChn;
    
    std::vector<char> redChnResized;
    std::vector<char> greenChnResized;
    std::vector<char> blueChnResized;
    std::vector<char> alphaChnResized;

} t_tgadata;

 
typedef struct {
    t_tgaheader header;
    t_tgadata   data;

} t_tga;


class TGAProcessing
{
public:

    ~TGAProcessing();

    fileStatus LoadImage(const std::string& inputFileName);
    fileStatus SaveImage(const std::string& outputFileName);

    void ResizeImage(float scaleFactor, resizeMethod interpolationMethod);

    size_t GetWidth();
    size_t GetHeight();
    size_t GetDepth();

private:
    t_tga           tga;

    fileStatus ReadImage(std::fstream& imageFile, t_tgaheader& tgaHeader, t_tgadata& tgaData);
    void WriteImage(std::fstream& imageFile, t_tgaheader& tgaHeader, t_tgadata& tgaData);
};


#endif	//TGAPROCESSING_H