// c1.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdint.h>
#pragma pack(push,1)

typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} BMPFileHeader;

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

// Lee el color (R,G,B) de un pixel especifico, sin modificar nada.
void leerColorPixel(FILE* archivo, BMPFileHeader fileHeader, BMPInfoHeader infoHeader,
    int x, int y, unsigned char* r, unsigned char* g, unsigned char* b) {
    int bytesPorFila = infoHeader.biWidth * 3;
    int padding = (4 - (bytesPorFila % 4)) % 4;
    int filaConPadding = bytesPorFila + padding;
    int filaInvertida = (infoHeader.biHeight - 1) - y;

    long offsetPixel = fileHeader.bfOffBits
        + (long)filaInvertida * filaConPadding
        + (long)x * 3;

    fseek(archivo, offsetPixel, SEEK_SET);

    unsigned char colorBGR[3];
    fread(colorBGR, sizeof(unsigned char), 3, archivo);

    // BMP guarda en orden B,G,R -> lo regresamos ya en orden R,G,B
    *b = colorBGR[0];
    *g = colorBGR[1];
    *r = colorBGR[2];
}

// Escribe el color (R,G,B) en un pixel especifico. Misma formula de offset de siempre.
void escribirColorPixel(FILE* archivo, BMPFileHeader fileHeader, BMPInfoHeader infoHeader,
    int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    int bytesPorFila = infoHeader.biWidth * 3;
    int padding = (4 - (bytesPorFila % 4)) % 4;
    int filaConPadding = bytesPorFila + padding;
    int filaInvertida = (infoHeader.biHeight - 1) - y;

    long offsetPixel = fileHeader.bfOffBits
        + (long)filaInvertida * filaConPadding
        + (long)x * 3;

    fseek(archivo, offsetPixel, SEEK_SET);

    unsigned char colorBGR[3];
    colorBGR[0] = b;
    colorBGR[1] = g;
    colorBGR[2] = r;

    fwrite(colorBGR, sizeof(unsigned char), 3, archivo);
}

int main(void)
{
    char nombreArchivo[260];
    printf("Nombre del archivo BMP a leer: ");
    scanf_s("%259s", nombreArchivo, (unsigned)sizeof(nombreArchivo));

    FILE* archivo = fopen(nombreArchivo, "r+b");
    if (archivo == NULL) {
        printf("Error: no se pudo abrir el archivo '%s'.\n", nombreArchivo);
        return 1;
    }

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    // Leer los 14 bytes del file header
    fread(&fileHeader, sizeof(BMPFileHeader), 1, archivo);

    if (fileHeader.bfType != 0x4D42) {
        printf("Error: este archivo no es un BMP valido.\n");
        fclose(archivo);
        return 1;
    }

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

    if (infoHeader.biBitCount != 24) {
        printf("\nEste programa solo soporta BMP de 24 bits por pixel.\n");
        fclose(archivo);
        return 1;
    }

    int ancho = infoHeader.biWidth;
    int alto = infoHeader.biHeight;

    // --- Menu de opciones ---
    printf("\n--- Que deseas hacer? ---\n");
    printf("1. Editar un pixel individual\n");
    printf("2. Pintar un patron de franjas horizontales con varios colores\n");
    printf("Opcion: ");
    int opcion;
    scanf_s("%d", &opcion);

    if (opcion == 1) {
        // ===== Editar un pixel individual =====
        int x, y;
        int r, g, b;
        printf("\n--- Editar un pixel ---\n");
        printf("Coordenada X (0 a %d): ", ancho - 1);
        scanf_s("%d", &x);
        printf("Coordenada Y (0 a %d): ", alto - 1);
        scanf_s("%d", &y);

        if (x < 0 || x >= ancho || y < 0 || y >= alto) {
            printf("Error: coordenadas fuera del rango de la imagen.\n");
            fclose(archivo);
            return 1;
        }

        // Mostrar el color actual de ESE pixel antes de cambiarlo
        unsigned char rActual, gActual, bActual;
        leerColorPixel(archivo, fileHeader, infoHeader, x, y, &rActual, &gActual, &bActual);
        printf("\nColor actual del pixel (%d, %d): R=%d  G=%d  B=%d\n", x, y, rActual, gActual, bActual);

        printf("Nuevo valor Rojo   (0-255): ");
        scanf_s("%d", &r);
        printf("Nuevo valor Verde  (0-255): ");
        scanf_s("%d", &g);
        printf("Nuevo valor Azul   (0-255): ");
        scanf_s("%d", &b);

        escribirColorPixel(archivo, fileHeader, infoHeader, x, y, (unsigned char)r, (unsigned char)g, (unsigned char)b);

        printf("\nPixel (%d, %d) actualizado a RGB(%d, %d, %d).\n", x, y, r, g, b);

    }
    else if (opcion == 2) {
        // ===== Pintar un patron de franjas horizontales =====
        int numColores;
        printf("\n--- Patron de franjas ---\n");
        printf("Cuantos colores distintos quieres usar (maximo 10)? ");
        scanf_s("%d", &numColores);

        if (numColores < 1) numColores = 1;
        if (numColores > 10) numColores = 10;

        unsigned char coloresR[10], coloresG[10], coloresB[10];
        for (int i = 0; i < numColores; i++) {
            int r, g, b;
            printf("\nColor #%d\n", i + 1);
            printf("  Rojo  (0-255): ");
            scanf_s("%d", &r);
            printf("  Verde (0-255): ");
            scanf_s("%d", &g);
            printf("  Azul  (0-255): ");
            scanf_s("%d", &b);
            coloresR[i] = (unsigned char)r;
            coloresG[i] = (unsigned char)g;
            coloresB[i] = (unsigned char)b;
        }

        int sugerido = alto / 10;
        if (sugerido < 1) sugerido = 1;
        printf("\nTu imagen mide %d px de alto. Sugerencia de grosor: %d\n", alto, sugerido);
        printf("Grosor de cada franja, en pixeles: ");
        int grosor;
        scanf_s("%d", &grosor);
        if (grosor < 1) grosor = 1;

        // Recorrer cada fila y decidir que color le toca segun el grosor
        for (int fy = 0; fy < alto; fy++) {
            int indiceColor = (fy / grosor) % numColores;
            for (int fx = 0; fx < ancho; fx++) {
                escribirColorPixel(archivo, fileHeader, infoHeader, fx, fy,
                    coloresR[indiceColor], coloresG[indiceColor], coloresB[indiceColor]);
            }
        }

        printf("\nPatron aplicado: %d colores, franjas de %d px de grosor.\n", numColores, grosor);

    }
    else {
        printf("Opcion no valida.\n");
        fclose(archivo);
        return 1;
    }

    fclose(archivo);
    return 0;
}