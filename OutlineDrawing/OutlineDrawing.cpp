#include <stdio.h>
#include <stdlib.h>
#pragma pack(push, 1)

typedef unsigned int DWORD;
typedef unsigned char BYTE;
typedef unsigned short int WORD;

struct BitMapHeader
{
	WORD bfType;
	DWORD bfSize;
	WORD bfReserved1;
	WORD bfReserved2;
	DWORD bfOffbits;
	DWORD biSize;
	DWORD biWidth;
	DWORD biHeight;
	WORD biPlanes;
	WORD biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	DWORD biXPelPerMeter;
	DWORD biYPelPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
};

struct tagRGBQUAD
{
	BYTE rgbBlue;
	BYTE rgbGreen;
	BYTE rgbRed;
	BYTE rgbReserved;
};

void ReadAndWriteHeader(FILE *file_read, FILE *file_write, BitMapHeader *header)
{
	fseek(file_read, 0, SEEK_SET);
	fread(header, sizeof(BitMapHeader), 1, file_read);
	fwrite(header, sizeof(BitMapHeader), 1, file_write);
}

void ReadAndWriteQuad(FILE *file_read, FILE *file_write, int size, tagRGBQUAD *RGB)
{
	fseek(file_read, size, SEEK_SET);
	fread(RGB, 1024, 1, file_read);
	fwrite(RGB, 1024, 1, file_write);
}

int main()
{
	// Объявление структур для заголовка и палитры изображения
	BitMapHeader header;
	tagRGBQUAD RGBQuad[256];

	// Имена файлов для чтения и записи
	char filename_read[] = "original.bmp";
	char filename_write[] = "patched.bmp";

	// Открытие файла для чтения
	FILE *file_read;
	fopen_s(&file_read, filename_read, "rb");

	// Проверка, открылся ли файл для чтения
	if (file_read == NULL)
	{
		printf("Failed to open the file for reading!\n");
		return 1;
	}

	// Открытие файла для записи
	FILE *file_write;
	fopen_s(&file_write, filename_write, "wb");

	// Проверка, открылся ли файл для записи
	if (file_write == NULL)
	{
		printf("Failed to open the file for writing!\n");
		return 1;
	}

	// Чтение и запись заголовка и палитры изображения
	ReadAndWriteHeader(file_read, file_write, &header);
	ReadAndWriteQuad(file_read, file_write, header.biSize + 14, RGBQuad);

	// Вывод информации о исходном файле
	printf("Image dimensions: %u x %u\n", header.biWidth, header.biHeight);
	printf("Original file size: %u bytes\n", header.bfSize);

	// Поиск зеленого цвета в палитре
	int green = NULL;
	for (int i = 0; i < 256; i++)
	{
		if (RGBQuad[i].rgbRed == 0 && RGBQuad[i].rgbGreen == 255 && RGBQuad[i].rgbBlue == 0)
		{
			green = i;
			break;
		}
	}

	// Проверка, найден ли зеленый цвет в палитре
	if (green == NULL)
	{
		printf("Failed to find the green color in the palette!\n");
		return 1;
	}

	// Перемещение указателя файла к началу данных изображения
	fseek(file_read, header.bfOffbits, SEEK_SET);
	fseek(file_write, header.bfOffbits, SEEK_SET);

	// Обход каждого пикселя изображения
	for (DWORD y = 0; y < header.biHeight; y++)
	{
		for (DWORD x = 0; x < header.biWidth; x++)
		{
			// Чтение пикселя из файла
			BYTE temp;
			fread(&temp, 1, 1, file_read);

			// Проверка, находится ли пиксель на границе изображения
			if (x < 1 || x >= header.biWidth - 1 || y < 1 || y >= header.biHeight - 1)
			{
				// Если да, запись зеленого цвета в файл
				fwrite(&green, 1, 1, file_write);
			}
			else
			{
				// Если нет, запись исходного цвета пикселя в файл
				fwrite(&temp, 1, 1, file_write);
			}
		}
	}

	// Закрытие файлов
	fclose(file_read);
	fclose(file_write);

	// Вывод сообщения об успешном патчинге файла
	printf("The file was patched successfully!\n");
}