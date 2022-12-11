#include <iostream>
#include "../TGAProcessing/TGAProcessing.h"


int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cout << std::endl;
        std::cout << "Syntax error!" << std::endl << "Pease use: halfsize.exe original.tga half.tga" << std::endl;
        std::cout << std::endl;
    }
    else
    {
        TGAProcessing tgaImageProcessing;

        std::cout << "Reading \"" << argv[1] << "\"..." << std::endl;

        fileStatus result = tgaImageProcessing.LoadImage(std::string(argv[1]));

        if (FILE_OK == result) {
            std::cout << "Done" << std::endl;

            std::cout << "Original size: " << std::to_string(tgaImageProcessing.GetWidth()) << "x" << std::to_string(tgaImageProcessing.GetHeight()) << std::endl;
            std::cout << "Resizing to: " << std::to_string(tgaImageProcessing.GetWidth() / SCALING_FACTOR) << "x" << std::to_string(tgaImageProcessing.GetHeight() / SCALING_FACTOR) << std::endl;
        
            tgaImageProcessing.ResizeImage(SCALING_FACTOR, NEAREST_NEIGHBOR);

            std::cout << "Done." << std::endl;
            std::cout << "Saving " << argv[2] << "..." << std::endl;

            tgaImageProcessing.SaveImage(std::string(argv[2]));

            std::cout << "Done." << std::endl;
        
        }
        else {
            std::cout << "Image reading error." << std::endl;
        }

    }

    return 0;
}
