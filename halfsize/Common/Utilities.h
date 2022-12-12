#pragma once
#include <string>
#include <vector>

#define SCALING_FACTOR      2
#define IMAGEBIT_SIZE       8

#define ORIGINAL_IMAGE_NAME 1
#define RESIZED_IMAGE_NAME  2


enum pixelBitDepth {
    PIXELDEPTH_24BIT = 3,
    PIXELDEPTH_32BIT = 4
};

enum fileStatus {
    FILE_ERR_OPEN = -1,
    FILE_OK = 0,
    FILE_ERR_BAD_FORMAT = 1,
    FILE_ERR_UNSUPPORTED,
};

enum resizeMethod
{
    NEAREST_NEIGHBOR = 0,
    BILINEAR_INTERPOL = 1
};

enum channelOrder {
    RGBA = 0,
    BGRA = 1
};


/**
* Interleave RGBA channels from seperate channel buffers
*
* @param resizedImagePixelData pointer to an array to hold the new interpolated pixel data
* @param width  - pixel width of original image
* @param height - pixel height of original image
* @param pixelDepth - defines the number of bits pers channel (32 bit or 24 bit)
* @param blueChn        - initialized vector for to hold interpolated blue pixels
* @param greenChn       - initialized vector for to hold interpolated green pixels
* @param redChn         - initialized vector for to hold interpolated red pixels
* @param alphaChn       - initialized vector for to hold interpolated alpha pixels
* @param scalingFactor  - resizing scale factor ( > 1 shrik, < 1 enlarge)
* @param colorOrder     - to fetch the correct order of color pixels
* @return Inited GIF Handler or NULL for error
*/
inline void interleave_rgba_channels(char* resizedImagePixelData, short& width, short& height, char& pixelDepth,
    std::vector<char>& blueChn, std::vector<char>& greenChn, std::vector<char>& redChn,
    std::vector<char>& alphaChn, float& scalingFactor, channelOrder colorOrder) 
{
    const size_t bitDepth = (pixelDepth / IMAGEBIT_SIZE);

    const int newHeight = height / SCALING_FACTOR;
    const int newWidth = width / SCALING_FACTOR;
    const int newArea = newHeight * newWidth;

    //resizedImagePixelData = new char[newArea * bitDepth];

    if (BGRA == colorOrder) {
        int sampleIdx = 0;
        for (int bgrIdx = 0; bgrIdx < newArea; bgrIdx++)
        {
            // order needs to be BRG
            resizedImagePixelData[sampleIdx] = blueChn[bgrIdx];
            sampleIdx++;
            resizedImagePixelData[sampleIdx] = greenChn[bgrIdx];
            sampleIdx++;
            resizedImagePixelData[sampleIdx] = redChn[bgrIdx];
            sampleIdx++;

            if (bitDepth == PIXELDEPTH_32BIT)
            {
                resizedImagePixelData[sampleIdx] = alphaChn[bgrIdx];
                sampleIdx++;
            }
        }
    }
}

/**
* Deinterleave RGBA channels from 1 x (width * height)  buffer
*
* @param originalImagePixelData - pointer to an array that holds the original pixel data
* @param width                  - pixel width of original image
* @param height                 - pixel height of original image
* @param pixelDepth             - defines the number of bits pers channel (32 bit or 24 bit)
* @param blueChn                - initialized vector for to hold original blue pixels
* @param greenChn               - initialized vector for to hold original green pixels
* @param redChn                 - initialized vector for to hold original red pixels
* @param alphaChn               - initialized vector for to hold original alpha pixels
* @param colorOrder             - to fetch the correct order of color pixels
*/
inline void deinterleave_rgba_channels(char* originalImagePixelData, short& width, short& height, char& pixelDepth,
    std::vector<char>& blueChn, std::vector<char>& greenChn, std::vector<char>& redChn, std::vector<char>& alphaChn,
    channelOrder colorOrder)
{
   // pixel area * BGR values
   const size_t pixelArea = (const size_t)(width) * (const size_t)(height);
   const size_t bitDepth = (pixelDepth / IMAGEBIT_SIZE);
   const size_t pixelAreaBitSize = (pixelArea * bitDepth);


   if (BGRA == colorOrder)
   {
       for (int sampleIdx = 0; sampleIdx < pixelAreaBitSize / bitDepth; sampleIdx++)
       {
           // BGR [0] [1] [2]
           blueChn.push_back(*originalImagePixelData);
           originalImagePixelData++;
           greenChn.push_back(*originalImagePixelData);
           originalImagePixelData++;
           redChn.push_back(*originalImagePixelData);
           originalImagePixelData++;

           if (bitDepth == PIXELDEPTH_32BIT) {
               alphaChn.push_back(*originalImagePixelData);
               originalImagePixelData++;
           }
       }
   }

}



/**
* Nearest Neighbor Interpolation 
*
* @param scaleFactor     - pointer to an array that holds the original pixel data
* @param width           - pixel width of original image
* @param height          - pixel height of original image
* @param pixelDepth      - defines the number of bits pers channel (32 bit or 24 bit)
* @param blueChn         - initialized vector for to hold original blue pixels
* @param greenChn        - initialized vector for to hold original green pixels
* @param redChn          - initialized vector for to hold original red pixels
* @param alphaChn        - initialized vector for to hold original alpha pixels
* @param blueChnResized  - initialized vector for to hold interpolated blue pixels
* @param greenChnResized - initialized vector for to hold interpolated green pixels
* @param redChnResized   - initialized vector for to hold interpolated red pixels
* @param alphaChnResized - initialized vector for to hold interpolated alpha pixels
*/
inline void nn_interpolation(float scaleFactor, short& width, short& height, char& pixelDepth,
    std::vector<char>& blueChn, std::vector<char>& greenChn, std::vector<char>& redChn, std::vector<char>& alphaChn,
    std::vector<char>& blueChnResized, std::vector<char>& greenChnResized, std::vector<char>& redChnResized, std::vector<char>& alphaChnResized) 
{
    const size_t bitDepth = (pixelDepth / IMAGEBIT_SIZE);
    //const int newHeight = height / scaleFactor;
    const int newHeight = static_cast<const int>(static_cast<float>(height) / scaleFactor);
    //const int newWidth = width / scaleFactor;
    const int newWidth = static_cast<const int>(static_cast<float>(width) / scaleFactor);
    const int newArea = newHeight * newWidth;


    float x_ratio = static_cast<float>(width )/ static_cast<float>(newWidth);
    float y_ratio = static_cast<float>(height) / static_cast<float>(newHeight);

    for (int i = 0; i < newHeight; i++) {
        for (int j = 0; j < newWidth; j++) {
            float px = floorf(static_cast<float>(j) * x_ratio);
            float py = floorf(static_cast<float>(i) * y_ratio);
            int output_index = i * newWidth + j;
            int input_index = static_cast<int>((py * static_cast<float>(width)) + px);

            blueChnResized[output_index]  = blueChn[input_index];
            greenChnResized[output_index] = greenChn[input_index];
            redChnResized[output_index]   = redChn[input_index];

            if (bitDepth == PIXELDEPTH_32BIT) {
                alphaChnResized[output_index] = alphaChn[input_index];
            }
        }
    }
}


/**
* Bilinear Interpolation
*
* @param scaleFactor     - pointer to an array that holds the original pixel data
* @param width           - pixel width of original image
* @param height          - pixel height of original image
* @param pixelDepth      - defines the number of bits pers channel (32 bit or 24 bit)
* @param blueChn         - initialized vector for to hold original blue pixels
* @param greenChn        - initialized vector for to hold original green pixels
* @param redChn          - initialized vector for to hold original red pixels
* @param alphaChn        - initialized vector for to hold original alpha pixels
* @param blueChnResized  - initialized vector for to hold interpolated blue pixels
* @param greenChnResized - initialized vector for to hold interpolated green pixels
* @param redChnResized   - initialized vector for to hold interpolated red pixels
* @param alphaChnResized - initialized vector for to hold interpolated alpha pixels
*/
inline void bilinear_interpolation(float scaleFactor, short& width, short& height, char& pixelDepth,
    std::vector<char>& blueChn, std::vector<char>& greenChn, std::vector<char>& redChn, std::vector<char>& alphaChn,
    std::vector<char>& blueChnResized, std::vector<char>& greenChnResized, std::vector<char>& redChnResized, std::vector<char>& alphaChnResized) 
{
    const size_t bitDepth = (pixelDepth / IMAGEBIT_SIZE);
    const int newHeight = static_cast<const int>(static_cast<float>(height) / scaleFactor);
    const int newWidth  = static_cast<const int>(static_cast<float>(width) / scaleFactor);
    const int newArea   = newHeight * newWidth;

    float x_ratio = static_cast<float>(width)  / static_cast<float>(newWidth);
    float y_ratio = static_cast<float>(height) / static_cast<float>(newHeight);

    int offset = 0;
    // border pixels for the new interpolated pixel
    int a, b, c, d;
    int pixel;
    for (int i = 0; i < newHeight; i++) {
        for (int j = 0; j < newWidth; j++) {
            int x = static_cast<int>(floorf(static_cast<float>(j) * x_ratio));
            int y = static_cast<int>(floorf(static_cast<float>(i) * y_ratio));
            float x_diff = (static_cast<float>(j) * x_ratio) - static_cast<float>(x);
            float y_diff = (static_cast<float>(i) * y_ratio) - static_cast<float>(y);
            int output_index = i * static_cast<int>(newWidth) + j;

            // calculating next idxs
            int input_index_a = y * static_cast<int>(width) + x;
            int input_index_b = input_index_a + 1;
            int input_index_c = input_index_a + static_cast<int>(width);
            int input_index_d = input_index_a + static_cast<int>(width) + 1;

            // blue channel
            a = blueChn[input_index_a];
            b = blueChn[input_index_b];
            c = blueChn[input_index_c];
            d = blueChn[input_index_d];

            pixel = static_cast<int>(a * (1 - x_diff) * 1 - y_diff + b * (x_diff) * (1 - y_diff) + c * (y_diff) * (1 - x_diff) + d * (x_diff * y_diff));
            blueChnResized[output_index] = blueChn[input_index_a];

            // green channel
            a = greenChn[input_index_a];
            b = greenChn[input_index_b];
            c = greenChn[input_index_c];
            d = greenChn[input_index_d];

            pixel = static_cast<int>(a * (1 - x_diff) * 1 - y_diff + b * (x_diff) * (1 - y_diff) + c * (y_diff) * (1 - x_diff) + d * (x_diff * y_diff));
            greenChnResized[output_index] = greenChn[input_index_a];

            // red channel
            a = redChn[input_index_a];
            b = redChn[input_index_b];
            c = redChn[input_index_c];
            d = redChn[input_index_d];

            pixel = static_cast<int>(a * (1 - x_diff) * 1 - y_diff + b * (x_diff) * (1 - y_diff) + c * (y_diff) * (1 - x_diff) + d * (x_diff * y_diff));
            redChnResized[output_index] = redChn[input_index_a];

            if (bitDepth == PIXELDEPTH_32BIT) {
                // red channel
                a = alphaChn[input_index_a];
                b = alphaChn[input_index_b];
                c = alphaChn[input_index_c];
                d = alphaChn[input_index_d];

                pixel = static_cast<int>(a * (1 - x_diff) * 1 - y_diff + b * (x_diff) * (1 - y_diff) + c * (y_diff) * (1 - x_diff) + d * (x_diff * y_diff));
                alphaChnResized[output_index] = alphaChn[input_index_a];
            }
        }
    }

}