// =============================================================================
//  StbImage.cpp  -  compiles the stb_image library exactly once.
//
//  stb_image is a "single header" library: the header contains the code, and
//  ONE .cpp file must define STB_IMAGE_IMPLEMENTATION before including it.
//  Every other file just does  #include <stb_image.h>  normally.
//  Nothing else should ever go in this file.
// =============================================================================
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
