// bmp_editor.c
// Lee el header de un archivo BMP, extrae la informacion y la muestra.
// Compilar en Visual Studio como proyecto de consola (C).

#define _CRT_SECURE_NO_WARNINGS   // Permite usar fopen() normal en Visual Studio
#include <stdio.h>
#include <stdint.h>               // Para tipos de tamano exacto: uint16_t, uint32_t

#pragma pack(push, 1)  // IMPORTANTE: evita que el compilador meta bytes de relleno

// --- BITMAPFILEHEADER: 14 bytes ---
typedef struct {
    uint16_t bfType;        // Debe ser 0x4D42 ("BM")
    uint32_t bfSize;        // Tamano total del archivo en bytes
    uint16_t bfReserved1;   // Sin uso
    uint16_t bfReserved2;   // Sin uso
    uint32_t bfOffBits;     // Offset donde empiezan los datos de pixeles
} BMPFileHeader;

// --- BITMAPINFOHEADER: 40 bytes ---
typedef struct {
    uint32_t biSize;            // Tamano de este header (deberia ser 40)
    int32_t  biWidth;           // Ancho de la imagen en pixeles
    int32_t  biHeight;          // Alto de la imagen en pixeles
    uint16_t biPlanes;          // Siempre 1
    uint16_t biBitCount;        // Bits por pixel (24 = color normal)
    uint32_t biCompression;     // 0 = sin compresion
    uint32_t biSizeImage;       // Tamano de los datos de pixeles
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} BMPInfoHeader;

#pragma pack(pop)

int main(void) {
    char nombreArchivo[260];
    printf("Nombre del archivo BMP a editar: ");
    scanf("%259s", nombreArchivo);

    // Abrir en modo "r+b": lectura Y escritura, sin borrar el contenido existente
    FILE* archivo = fopen(nombreArchivo, "r+b");
    if (archivo == NULL) {
        printf("Error: no se pudo abrir el archivo '%s'.\n", nombreArchivo);
        return 1;
    }

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    // Leer los 14 bytes del file header
    fread(&fileHeader, sizeof(BMPFileHeader), 1, archivo);

    // Verificar que sea un BMP valido ('B' = 0x42, 'M' = 0x4D -> 0x4D42 en little-endian)
    if (fileHeader.bfType != 0x4D42) {
        printf("Error: este archivo no es un BMP valido.\n");
        fclose(archivo);
        return 1;
    }

    // Leer los 40 bytes del info header
    fread(&infoHeader, sizeof(BMPInfoHeader), 1, archivo);

    // Mostrar la informacion extraida
    printf("\n--- Informacion del archivo BMP ---\n");
    printf("Tamano del archivo:      %u bytes\n", fileHeader.bfSize);
    printf("Offset a los pixeles:    %u\n", fileHeader.bfOffBits);
    printf("Ancho:                   %d px\n", infoHeader.biWidth);
    printf("Alto:                    %d px\n", infoHeader.biHeight);
    printf("Bits por pixel:          %u\n", infoHeader.biBitCount);
    printf("Compresion:              %u\n", infoHeader.biCompression);
    printf("Tamano de datos imagen:  %u bytes\n", infoHeader.biSizeImage);

    // Este programa solo maneja BMP de 24 bits (3 bytes por pixel, sin paleta)
    if (infoHeader.biBitCount != 24) {
        printf("\nEste programa solo soporta BMP de 24 bits por pixel.\n");
        fclose(archivo);
        return 1;
    }

    int ancho = infoHeader.biWidth;
    int alto = infoHeader.biHeight;

    // Pedir que pixel modificar
    int x, y;
    int r, g, b;
    printf("\n--- Editar un pixel ---\n");
    printf("Coordenada X (0 a %d): ", ancho - 1);
    scanf("%d", &x);
    printf("Coordenada Y (0 a %d): ", alto - 1);
    scanf("%d", &y);

    if (x < 0 || x >= ancho || y < 0 || y >= alto) {
        printf("Error: coordenadas fuera del rango de la imagen.\n");
        fclose(archivo);
        return 1;
    }

    printf("Nuevo valor Rojo   (0-255): ");
    scanf("%d", &r);
    printf("Nuevo valor Verde  (0-255): ");
    scanf("%d", &g);
    printf("Nuevo valor Azul   (0-255): ");
    scanf("%d", &b);

    // --- Calcular la posicion exacta del pixel dentro del archivo ---

    // Cada fila debe ocupar un multiplo de 4 bytes (padding de BMP)
    int bytesPorFila = ancho * 3;
    int padding = (4 - (bytesPorFila % 4)) % 4;
    int filaConPadding = bytesPorFila + padding;

    // Las filas se guardan de ABAJO hacia ARRIBA, por eso invertimos "y"
    int filaInvertida = (alto - 1) - y;

    // Offset del pixel = inicio de datos + (fila * tamano de fila) + (columna * 3 bytes)
    long offsetPixel = fileHeader.bfOffBits
        + (long)filaInvertida * filaConPadding
        + (long)x * 3;

    // Posicionarnos ahi con fseek
    fseek(archivo, offsetPixel, SEEK_SET);

    // BMP guarda los colores en orden B, G, R (al reves de lo normal)
    unsigned char colorBGR[3];
    colorBGR[0] = (unsigned char)b;
    colorBGR[1] = (unsigned char)g;
    colorBGR[2] = (unsigned char)r;

    fwrite(colorBGR, sizeof(unsigned char), 3, archivo);

    printf("\nPixel (%d, %d) actualizado a RGB(%d, %d, %d).\n", x, y, r, g, b);

    fclose(archivo);
    return 0;
}
