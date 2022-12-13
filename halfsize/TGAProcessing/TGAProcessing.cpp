#include "TGAProcessing.h"


TGAProcessing::~TGAProcessing() {
    // Free memory allocation from char* 
    delete[] tga.data.originalImagePixelData;
    delete[] tga.data.resizedImagePixelData;

}


fileStatus TGAProcessing::LoadImage(const std::string& inputFileName)
{
    std::fstream imageFile;
    imageFile.open(inputFileName, std::ios::in | std::ios::binary);


    if (imageFile.is_open()) {
        fileStatus result = ReadImage(imageFile, tga.header, tga.data);
         
        if (FILE_OK == result) {
            imageFile.close();
            return result;
        }
        else {
            return result;
        }
    }
    else {
        return FILE_ERR_OPEN;
    }
}

fileStatus TGAProcessing::SaveImage(const std::string& outputFileName)
{
    std::fstream file;
    file.open(outputFileName, std::ios::out | std::ios::binary);

    if (file.is_open())
    {
        WriteImage(file, tga.header, tga.data);
        file.close();
        return FILE_OK;
    }
    else
    {
        return FILE_ERR_OPEN;
    }
}

fileStatus TGAProcessing::ReadImage(std::fstream& imageFile, t_tgaheader& tgaHeader, t_tgadata& tgaData)
{
    // Read Header
    imageFile.read(&tgaHeader.idLength, sizeof(tgaHeader.idLength));
    imageFile.read(&tgaHeader.colourMapType, sizeof(tgaHeader.colourMapType));
    imageFile.read(&tgaHeader.imageType, sizeof(tgaHeader.imageType));
    imageFile.read((char*)(&tgaHeader.colourMapOrigin), sizeof(tgaHeader.colourMapOrigin));
    imageFile.read((char*)(&tgaHeader.colourMapLength), sizeof(tgaHeader.colourMapLength));
    imageFile.read(&tgaHeader.colourMapDepth, sizeof(tgaHeader.colourMapDepth));
    imageFile.read((char*)(&tgaHeader.x_origin), sizeof(tgaHeader.x_origin));
    imageFile.read((char*)(&tgaHeader.y_origin), sizeof(tgaHeader.y_origin));
    imageFile.read((char*)(&tgaHeader.width), sizeof(tgaHeader.width));
    imageFile.read((char*)(&tgaHeader.height), sizeof(tgaHeader.height));
    imageFile.read(&tgaHeader.pixelDepth, sizeof(tgaHeader.pixelDepth));
    imageFile.read(&tgaHeader.imageDescriptor, sizeof(tgaHeader.pixelDepth));

    if (tgaHeader.colourMapType != 0)
        return FILE_ERR_UNSUPPORTED;
    if (tgaHeader.imageType != 2)
        return FILE_ERR_UNSUPPORTED;
    if ((tgaHeader.width < 1) || (tgaHeader.height < 1))
        return FILE_ERR_BAD_FORMAT;
    if (tgaHeader.pixelDepth > 32)
        return FILE_ERR_UNSUPPORTED;

    // ======================================================

    const size_t pixelArea = (const size_t)(tgaHeader.width) * (const size_t)(tgaHeader.height);
    const size_t bitDepth = (tgaHeader.pixelDepth / IMAGEBIT_SIZE);
    // pixel area * BGR values
    const size_t pixelAreaBitSize = (pixelArea * bitDepth);
    tgaData.originalImagePixelData = new char[pixelAreaBitSize];
    
    // Read BGR Data
    imageFile.read(tgaData.originalImagePixelData, pixelAreaBitSize);
    
    
    return FILE_OK;
}

void TGAProcessing::ResizeImage(float scaleFactor, resizeMethod interpolationMethod)
{
    const size_t bitDepth = (tga.header.pixelDepth / IMAGEBIT_SIZE);
    const int newHeight   = static_cast<const int>(static_cast<float>(tga.header.height) / scaleFactor);
    const int newWidth    = static_cast<const int>(static_cast<float>(tga.header.width) / scaleFactor);
    const int newArea     = newHeight * newWidth;

    // initialize colour buffers for the interpolation
    tga.data.blueChnResized.assign(newArea, sizeof(char));
    tga.data.greenChnResized.assign(newArea, sizeof(char));
    tga.data.redChnResized.assign(newArea, sizeof(char));
    tga.data.alphaChnResized.assign(newArea, sizeof(char));
    tga.data.resizedImagePixelData = new char[newArea * bitDepth];

    // ======================================================
    // De-interleave input stream to different colour channels
    deinterleave_rgba_channels(tga.data.originalImagePixelData, tga.header.width, tga.header.height, tga.header.pixelDepth,
        tga.data.blueChn, tga.data.greenChn, tga.data.redChn, tga.data.alphaChn, BGRA);

    // ======================================================
    // Interpolation of original pixel data to new image size
    if (NEAREST_NEIGHBOR == interpolationMethod) {

        nn_interpolation(scaleFactor, tga.header.width, tga.header.height, tga.header.pixelDepth,
            tga.data.blueChn, tga.data.greenChn, tga.data.redChn, tga.data.alphaChn,
            tga.data.blueChnResized, tga.data.greenChnResized, tga.data.redChnResized, tga.data.alphaChnResized);
    }
    else if (BILINEAR_INTERPOL == interpolationMethod) {

        bilinear_interpolation(scaleFactor, tga.header.width, tga.header.height, tga.header.pixelDepth,
            tga.data.blueChn, tga.data.greenChn, tga.data.redChn, tga.data.alphaChn,
            tga.data.blueChnResized, tga.data.greenChnResized, tga.data.redChnResized, tga.data.alphaChnResized);
    }

    // ======================================================
    // Interleave different colour channels to output stream [1, (width*height) pixels]
    interleave_rgba_channels(tga.data.resizedImagePixelData, tga.header.width, tga.header.height, tga.header.pixelDepth,
        tga.data.blueChnResized, tga.data.greenChnResized, tga.data.redChnResized, tga.data.alphaChnResized,
                            scaleFactor, BGRA);
}

void TGAProcessing::WriteImage(std::fstream& imageFile, t_tgaheader& tgaHeader, t_tgadata& tgaData)
{
    // pixel area * BGR values
    const int newHeight = tgaHeader.height / SCALING_FACTOR;
    const int newWidth = tgaHeader.width / SCALING_FACTOR;
    const int newArea = newHeight * newWidth;
    const size_t bitDepth = (tgaHeader.pixelDepth / IMAGEBIT_SIZE);
    const size_t pixelAreaBitSize = newArea * bitDepth ;

    // Write Header
    imageFile.write(&tgaHeader.idLength, sizeof(tgaHeader.idLength));
    imageFile.write(&tgaHeader.colourMapType, sizeof(tgaHeader.colourMapType));
    imageFile.write(&tgaHeader.imageType, sizeof(tgaHeader.imageType));
    imageFile.write((char*)(&tgaHeader.colourMapOrigin), sizeof(tgaHeader.colourMapOrigin));
    imageFile.write((char*)(&tgaHeader.colourMapLength), sizeof(tgaHeader.colourMapLength));
    imageFile.write(&tgaHeader.colourMapDepth, sizeof(tgaHeader.colourMapDepth));
    imageFile.write((char*)(&tgaHeader.x_origin), sizeof(tgaHeader.x_origin));
    imageFile.write((char*)(&tgaHeader.y_origin), sizeof(tgaHeader.y_origin));
    imageFile.write((char*)(&newWidth), sizeof(tgaHeader.width));
    imageFile.write((char*)(&newHeight), sizeof(tgaHeader.height));
    imageFile.write(&tgaHeader.pixelDepth, sizeof(tgaHeader.pixelDepth));
    imageFile.write(&tgaHeader.imageDescriptor, sizeof(tgaHeader.pixelDepth));

    // Write Pixel BGR data
    imageFile.write(tgaData.resizedImagePixelData, pixelAreaBitSize);    
}



size_t TGAProcessing::GetWidth()
{
    return tga.header.width;
}

size_t TGAProcessing::GetHeight()
{
    return tga.header.height;
}

size_t TGAProcessing::GetDepth()
{
    return tga.header.pixelDepth;
}