#include <iostream>
#include <fstream>
#include <cstdio> // Para sprintf
#include <cstdlib> // Para new y delete

using namespace std;
// Tipos y constantes
typedef unsigned char Byte;
// Función auxiliar: Manejo de buffer dinámico.
// Duplica la capacidad del buffer si no hay espacio.
void expandirBuffer(char*& buffer, int& actualSize, int requerido) {
    int newSize = actualSize;
    while (newSize < requerido) {
        newSize *= 2;
    }
    char* newBuffer = new char[newSize];

    // Copiar datos del buffer viejo al nuevo
    for (int i = 0; i < actualSize; ++i) {
        newBuffer[i] = buffer[i];
    }

    delete[] buffer;
    buffer = newBuffer;
    actualSize = newSize;
}

// Descompresión RLE con buffer dinámico
char* descomprimirRLE(const Byte* compressed, int len, int& outLen) {
    int bufferSize = 1024; // Tamaño inicial
    char* output = new char[bufferSize];
    int pos = 0;
    outLen = 0;

    for (int i = 0; i < len; i += 2) {
        int count = compressed[i];
        char character = compressed[i + 1];

        if (pos + count >= bufferSize) {
            expandirBuffer(output, bufferSize, pos + count);
        }

        for (int j = 0; j < count; j++) {
            output[pos++] = character;
        }
        outLen += count;
    }

    // Redimensionar al tamaño exacto para la salida final
    char* finalOutput = new char[outLen + 1];
    for (int i = 0; i < outLen; i++) {
        finalOutput[i] = output[i];
    }
    finalOutput[outLen] = '\0';
    delete[] output;

    return finalOutput;
}

// Descompresión LZ78 con buffer dinámico
char* descomprimirLZ78(const Byte* compressed, int len, int& outLen) {
    char** diccionario = new char*[65536];
    int tamDic = 0;

    int bufferSize = 1024;
    char* output = new char[bufferSize];
    int pos = 0;
    outLen = 0;

    for (int i = 0; i < len; i += 3) {
        if (i + 2 >= len) break;

        unsigned short indice = (compressed[i] << 8) | compressed[i + 1];
        char caracter = compressed[i + 2];

        char* nuevaCadena;
        int longitudNueva;

        if (indice == 0) {
            nuevaCadena = new char[2];
            nuevaCadena[0] = caracter;
            nuevaCadena[1] = '\0';
            longitudNueva = 1;
        } else {
            if (indice - 1 >= tamDic) {
                // Índice inválido, error en la descompresión
                for (int j = 0; j < tamDic; j++) delete[] diccionario[j];
                delete[] diccionario;
                delete[] output;
                return nullptr;
            }
            char* prefijo = diccionario[indice - 1];
            int longPrefijo = 0;
            while (prefijo[longPrefijo] != '\0') longPrefijo++;

            longitudNueva = longPrefijo + 1;
            nuevaCadena = new char[longitudNueva + 1];

            for (int j = 0; j < longPrefijo; j++) {
                nuevaCadena[j] = prefijo[j];
            }
            nuevaCadena[longPrefijo] = caracter;
            nuevaCadena[longitudNueva] = '\0';
        }

        if (pos + longitudNueva >= bufferSize) {
            expandirBuffer(output, bufferSize, pos + longitudNueva);
        }

        for (int j = 0; j < longitudNueva; j++) {
            output[pos++] = nuevaCadena[j];
        }
        outLen += longitudNueva;

        if (tamDic < 65536) {
            diccionario[tamDic] = nuevaCadena;
            tamDic++;
        } else {
            delete[] nuevaCadena; // No se puede agregar al diccionario
        }
    }

    // Limpiar diccionario
    for (int i = 0; i < tamDic; i++) {
        delete[] diccionario[i];
    }
    delete[] diccionario;

    // Redimensionar al tamaño exacto para la salida final
    char* finalOutput = new char[outLen + 1];
    for (int i = 0; i < outLen; i++) {
        finalOutput[i] = output[i];
    }
    finalOutput[outLen] = '\0';
    delete[] output;

    return finalOutput;
}



// Rotación de bits a la derecha
Byte rotateRight(Byte b, int n) {
    return (b >> n) | (b << (8 - n));
}

// Desencriptación
Byte* desencriptar(const Byte* encrypted, int len, int n, Byte k) {
    Byte* decrypted = new Byte[len];
    for (int i = 0; i < len; i++) {
        decrypted[i] = rotateRight(encrypted[i] ^ k, n); //XOR
    }
    return decrypted;
}
