#define STB_IMAGE_IMPLEMENTATION // Exactly one .CPP (this Image.cpp) should #define this before #including stb_image.h
#include "ThirdParty/stb/stb_image.h"
#include "Image.hpp"
#include "Engine/Core/Rgba8.hpp"

Image::Image() 
	: m_dimensions(0, 0) 
{

}

Image::~Image() 
{

}

Image::Image(const char* imageFilePath)
{
	m_imageFilePath = imageFilePath;

	int width, height, channels;
	unsigned char* imageData = stbi_load(imageFilePath, &width, &height, &channels, STBI_rgb_alpha);
	if (imageData)
	{
		m_dimensions = IntVec2(width, height);
		m_texelRgba8Data.resize(width * height);

		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				int srcIndex = (height - 1 - y) * width + x;
				int dstIndex = y * width + x;               
				m_texelRgba8Data[dstIndex] = Rgba8(
					imageData[srcIndex * 4 + 0],
					imageData[srcIndex * 4 + 1],
					imageData[srcIndex * 4 + 2],
					imageData[srcIndex * 4 + 3]
				);
			}
		}

		stbi_image_free(imageData);
	}
	else
	{
		m_dimensions = IntVec2(0, 0);
	}
}

Image::Image(IntVec2 size, Rgba8 color) 
	: m_dimensions(size)
{
	m_texelRgba8Data.resize(size.x * size.y, color); 
}

IntVec2 Image::GetDimensions() const
{
	return m_dimensions;
}

const std::string& Image::GetImageFilePath() const
{
	return m_imageFilePath;
}

const void* Image::GetRawData() const
{
	return m_texelRgba8Data.data();
}