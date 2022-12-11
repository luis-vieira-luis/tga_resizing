TGA Image Scaling - Coding test
============
___

## Image Processing task
The goal of the task was to be able to, given a certain image, scale its dimensions to half the size. A few considerations were to be taken:

1. The image file format is TGA or TARGA (True Vision Graphics)
2. Command line executable for scaling image size

Throughout the implementation of this program, I have tried to generalized methods and prevent the usage of "magic numbers", these can be found under the `utilities.h` in .\Common folder.

___
#### TGA image file

The TGA file format is composed by a header file of 18 bytes containing some useful metadata for processing, e.g. width, height, bit depth. The pixel information is organized in a different order than the standard RGBA. It is instead BGRA. Some curious facts I came across for this different alignment:
- BGR is the little-endian version of RGB.
- TGA format originated due to the Targa graphics cards which displayed 24bit colour back when most people were still running 16 colour (64 colour palette) EGA graphics cards.
- Targa cards did format the colours BGR rather than RGB.

#### Program "halfsize.exe"
The program takes in 2 arguments from a command line:

    halfsize.exe original.tga half.tga

This calls a class `TGAProcessing`. This class contains functions for:
- Loading an image: reading header and pixel data
    - `LoadImage()`
    - `ReadImage()`
- Resizing an image: scales the image bigger or smaller according to a scaling factor and can use either nearest neighbor or bilinear interpolation
    - `ResizeImage()`
- Saving an image: writes the new scaled image into a new file
    - `SaveImage()`
    - `WriteImage()`

This class uses methods defined in a separate file `utilities.h` that can be generalized processing methods for other types of image formats:

- `interleave_rgba_channels()`
- `deinterleave_rgba_channels()`
- `nn_interpolation()`
- `bilinear_interpolation()`

___
#### Scaling methods

The principle of image scaling is to have a reference image and using this image as the base to construct a new scaled image. The constructed image will be smaller, larger, or equal in size depending on the scaling ratio. When enlarging an image, we are actually introducing empty spaces in the original base picture. Shrinking, on the other hand, involves reduction of pixels and it means lost of irrecoverable information. In this case, scaling algorithm is to find the right pixels to throw away.

Two methods have been implemented that provide scaling either up or down: nearest neighbor and bilinear interpolation.

-  **Nearest Neighbor Scaling**

    In this method, the empty spaces when enlarging an image are filled by copying the nearest pixels. When shrinking, the opposite happens and the nearest pixel will be rejected.
- **Bilinear Interpolation Scaling**

    This method uses the same approach has the nearest neighbor with an extra step of interpolation. Instead of copying or deleting the neighboring pixels (which often results in jaggy image), interpolation technique based on surrounding pixels is used to produce a smoother scaling.

    By enlarging an image, some new pixels are constructed by means of interpolation.

    By shrinking, we are tempted to think the right pixels are selected to keep while the others are thrown away, but this is not the case. Unlike nearest neighbor shrinking where pixels are thrown, bilinear shrinking estimates a smaller resolution of the original image. Even though details are lost, almost all the new pixels in the shrunk image do not come directly from their original, but interpolated, indirectly keeping the properties of lost pixels.

    |    | x1  |  x   | x2  |
    |----|-----|------|-----|
    | y1 | q11 | `R1` | q21 |
    | y  |     |  P   |     |
    | y2 | q12 | `R2` | q22 |

    Steps:
    - Linear interpolation between Q11 - Q21 > `R1`
    - Linear interpolation between Q12 - Q22 > `R2`
    - Final interpolation between the new points R1 - R2

In the context of image processing a further consideration needs to be taken as for each pixel there is information for red, blue, green (24bit) and alpha channels (when 32bit pixel depth)

___
### Implementation Notes

The pixel colour data is fetched using a 1-D char pointer array of:

    image width x image length x bit depth x number of colour channels RGBA

In this array, the colour data is delivered interleaved in the order of BGRA per pixel. To be able to simplify the task of finding the correct indexes for either scaling method, I chose:
1. De-interleave the BGRA information into separate channel buffers, using `std::vector` for each colour
2. Apply the scaling method
3. Interleave the different colour buffers back to the BGRA format using a char pointer to a new sized array with the scaled image size.

#### Debugging Setup
To be able to understand if the pixel data is being processed correctly, it was important to provide a controlled setup. First, I have implemented the scaling methods on matlab processing only a random matrix of numbers on a range of [0:255].

When moving to the C++ on Visual Studio, I have first implemented debug vectors to hold the processed data at different points of the execution (reading original data, de-interleave data, scaling, interleave).

As input images I chose to progressively increase the complexity in terms of RGBA information and in terms of dimensions. This way I could run the project in a debug configuration and adding brakpoints along the code, confirm if the different debug vectors are holding the correct information. To finally preview the output image, I used this website to confirm if the scaling was providing the right BGRA colours (https://schmittl.github.io/tgajs/).

The following images were first converted to TGA format but for purposes of displaying the images are shown in PNG or JPG format.


<figure>
<img src="/Media/4_4_blue.png" style="width:20%;">
<figcaption align = "left"><b>Fig.1 - 4x4, BGRA = (255, 0, 0, 255) </b></figcaption>
</figure>

<figure>
<img src="/Media/4_4_alternative.png" style="width:20%;">
<figcaption align = "left"><b>Fig.2 - 4x4,  (255, 127, 127, 255)</b></figcaption>
</figure>

<figure>
<img src="/Media/24_24.png" style="width:20%;">
<figcaption align = "left"><b>Fig.3 - 24x24, 2 colours</b></figcaption>
</figure>

<figure>
<img src="/Media/24_24_alternative.png" style="width:20%;">
<figcaption align = "left"><b>Fig.4 - 24x24, 2 colours, irregular pattern</b></figcaption>
</figure>

<figure>
<img src="/Media/picture1.jpg" style="width:20%;">
<figcaption align = "left"><b>Fig.5 - Full colour picture</b></figcaption>
</figure>
<figure>
<img src="/Media/picture2.jpg" style="width:20%;">
<figcaption align = "left"><b>Fig.6 - Full colour picture</b></figcaption>
</figure>

\
I have added a DEBUG_FLAG macro that can be used to encapsulate prints to output console defining it as 1. Currently all the `std::cout` lines were removed but for future reference these could be save guarded by

    #if
    std::cout << "BGR colours = " << debugVecto[i] << std::endl
    #endif

___
### Personal Considerations
Coming from an audio DSP background, this task was both interesting and challenging. As an image processing task, it took me some time to be familiar with the metadata residing on the TGA header structure and how to collect this data. Furthermore, the initial reading on the pixel colour information was confusing as I did not know how the colours were stored (e,g, which order, how to retrieve the RBG value).

Coincidently, the abstraction level of RGBA values and the array manipulation (scaling) started to make quite some sense when I made the analogy with audio channels in an input buffer. From this point it was easier to understand what to do (interleave/de-interleave, nearest neighbor/bilinear interpolation). In the same way we have 3 or 4 channels of colours, we could have multiple channels of audio defining different audio configurations (mono, stereo or other multichannel setups).

It was also very helpful to export some of the test vectors as tables where I could see both color value and indexing (see figure 7)

<figure>
<img src="/Media/pixel_table.jpg" style="width:80%;">
<figcaption align = "left"><b>Fig.7 - Pixel table </b></figcaption>
</figure>
