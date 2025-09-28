#include <iostream>
#include "Funtions.h"
#include <fstream>
#include <cstdio> // Para sprintf
#include <cstdlib> // free
#include <cstring>

using namespace std;

// Función auxiliar: Manejo de buffer dinámico.
// Duplica la capacidad del buffer si no hay espacio.
void expandirBuffer(char*& buffer, int& actualSize, int requerido) {
    int newSize = actualSize;
    // Usamos while para asegurar que el nuevo tamaño sea suficiente
    while (newSize < requerido) {
        newSize *= 2;
    }
    // Usamos malloc/realloc/free para consistencia con Desafio1_Unificado.cpp
    // y para evitar delete[] si el buffer fue creado con malloc
    char* nuevo = (char*) malloc(newSize);
    if (!nuevo) return; // Manejo básico de error de malloc
    for (int i = 0; i < actualSize; ++i) nuevo[i] = buffer[i];
    free(buffer);
    buffer = nuevo;
    actualSize = newSize;
}

// Valida patrón RLE ASCII: uno+ dígitos ASCII seguidos de un símbolo alfanumérico
int pareceRLE_ASCII(const Byte* data, int len) {
    int i = 0;
    while (i < len) {
        if (!(data[i] >= '0' && data[i] <= '9')) return 0;
        int huboDig = 0;
        while (i < len && data[i] >= '0' && data[i] <= '9') { huboDig = 1; i++; }
        if (!huboDig) return 0;
        if (i >= len) return 0;
        Byte s = data[i++];
        int esAlnum = ((s>='A'&&s<='Z')||(s>='a'&&s<='z')||(s>='0'&&s<='9'));
        if (!esAlnum) return 0;
    }
    return 1;
}

// Descompresión RLE ASCII (ej: "11W1B...")
char* descomprimirRLE_ASCII(const Byte* data, int len, int& outLen) {
    int cap = 1024;
    char* out = (char*) malloc(cap);
    if (!out) return nullptr;
    int pos = 0; outLen = 0;
    int i = 0;
    while (i < len) {
        if (data[i] < '0' || data[i] > '9') { free(out); return NULL; }
        long count = 0;
        while (i < len && data[i] >= '0' && data[i] <= '9') {
            count = count * 10 + (data[i] - '0');
            i++;
        }
        if (i >= len) { free(out); return NULL; }
        Byte sym = data[i++];
        int ok = ((sym>='A'&&sym<='Z')||(sym>='a'&&sym<='z')||(sym>='0'&&sym<='9'));
        if (!ok) { free(out); return NULL; }

        if (pos + (int)count > cap) {
            // Usamos la función expandirBuffer para reallocar
            expandirBuffer(out, cap, pos + (int)count);
            if (!out) return NULL;
        }
        for (long k = 0; k < count; ++k) out[pos++] = (char)sym;
        outLen += (int)count;
    }
    char* fin = (char*) malloc(outLen + 1);
    if (!fin) { free(out); return NULL; }
    for (int k = 0; k < outLen; ++k) fin[k] = out[k];
    fin[outLen] = '\0';
    free(out);
    return fin;
}
// D======================= LZ78 =======================
int strlen_c(const char* s) { int L=0; while (s[L]!='\0') L++; return L; }

char* concatenarPrefijoChar(const char* pref, char c) {
    int L = 0; if (pref) L = strlen_c(pref);
    char* s = (char*) malloc(L + 2);
    if (!s) return NULL;
    for (int i = 0; i < L; ++i) s[i] = pref[i];
    s[L] = c; s[L+1] = '\0';
    return s;
}

// Descompresión LZ78 con índice big-endian y manejo de memoria dinámica
char* descomprimirLZ78(const Byte* data, int len, int& outLen) {
    if (len % 3 != 0) return NULL;
    int tokens = len / 3;
    int cap = (int)tokens * 256;
    if (cap < tokens) return NULL; // Desbordamiento o tamaño ridículo

    char* out = (char*) malloc((size_t)cap + 1);
    if (!out) return NULL;
    long pos = 0; outLen = 0;

    // diccionario [1..tokens] - Usamos calloc para inicializar a NULL
    char** dict = (char**) calloc((size_t)(tokens + 1), sizeof(char*));
    if (!dict) { free(out); return NULL; }
    int dictSize = 0;

    for (int t = 0; t < tokens; ++t) {
        Byte bh = data[t*3 + 0];
        Byte bl = data[t*3 + 1];
        Byte ch = data[t*3 + 2];
        int idx = ((int)bh << 8) | (int)bl; // Índice Big-Endian (0-65535)

        const char* pref = "";
        if (idx > 0) {
            if (idx <= dictSize && dict[idx] != NULL) pref = dict[idx];
            else { // Índice inválido: limpiar y salir
                for (int i = 1; i <= dictSize; ++i) if (dict[i]) free(dict[i]);
                free(dict); free(out); return NULL;
            }
        }
        char* S = concatenarPrefijoChar(pref, (char)ch);
        if (!S) {
            for (int i = 1; i <= dictSize; ++i) if (dict[i]) free(dict[i]);
            free(dict); free(out); return NULL;
        }
        int LS = strlen_c(S);

        if (pos + LS + 1 > cap) {
            // Usamos expandirBuffer para reallocar
            expandirBuffer(out, cap, (int)(pos + LS + 1));
            if (!out) {
                for (int i = 1; i <= dictSize; ++i) if (dict[i]) free(dict[i]);
                free(dict); free(S); return NULL;
            }
            // LÍNEA ELIMINADA: cap = (int)cap;
        }

        for (int j = 0; j < LS; ++j) out[pos++] = S[j];
        outLen += LS;

        // Agregar al diccionario (1-based)
        if (dictSize < tokens) {
            dictSize++; dict[dictSize] = S;
        }
        // LÍNEA ELIMINADA: } else { free(S); }
    }

    out[pos] = '\0';
    // Limpiar diccionario (solo los que se asignaron y no se liberaron)
    for (int i = 1; i <= dictSize; ++i) if (dict[i]) free(dict[i]);
    free(dict);

    return out;
}

// Rotación de bits a la derecha
Byte rotateRight(Byte b, int n) {
    n &= 7; if (n == 0) return b;
    return (Byte)((b >> n) | (b << (8 - n)));
}

// Quita saltos de línea al final del fragmento
void limpiarFinalCRLF(char* s, int& L) {
    while (L > 0 && (s[L-1]=='\n' || s[L-1]=='\r')) { s[L-1] = '\0'; L--; }
}


// Desencriptación
Byte* desencriptar(const Byte* encrypted, int len, int n, Byte k) {
    // Usamos malloc/free como en Desafio1_Unificado.cpp
    Byte* decrypted = (Byte*) malloc(len);
    if (!decrypted) return nullptr;
    for (int i = 0; i < len; i++) {
        Byte t = (Byte)(encrypted[i] ^ k);
        decrypted[i] = rotateRight(t, n);
    }
    return decrypted;
}

// Leer archivo binario
Byte* leerArchivoBinario(const char* nombre, int& tamano) {
    tamano = 0;
    ifstream archivo(nombre, ios::binary | ios::ate);
    if (!archivo) return nullptr;
    streampos sz = archivo.tellg();
    if (sz < 0) return nullptr;
        archivo.seekg(0, ios::beg);
        Byte* datos = (Byte*) malloc((size_t)sz);
    if (!datos) return nullptr;
        archivo.read(reinterpret_cast<char*>(datos), sz);
    if (!archivo) { free(datos); return nullptr; }
        tamano = (int)sz;
        return datos;
}
// Leer archivo de texto
char* leerArchivoTexto(const char* nombre, int& tamano) {
    tamano = 0;
    ifstream archivo(nombre, ios::binary | ios::ate); // binary para no transformar CRLF
    if (!archivo) return nullptr;
    streampos sz = archivo.tellg();
    if (sz < 0) return nullptr;
    archivo.seekg(0, ios::beg);
    char* datos = (char*) malloc((size_t)sz + 1);
    if (!datos) return nullptr;
    archivo.read(datos, sz);
    if (!archivo) { free(datos); return nullptr; }
    datos[(int)sz] = '\0';
    tamano = (int)sz;
    return datos;
}

// Verificar si el texto contiene el fragmento
bool contieneFragmento(const char* texto, int tamTexto, const char* fragmento, int tamFrag) {
    if (!texto || !fragmento) return false;
    if (tamFrag == 0) return true;
    if (tamTexto < tamFrag) return false;

    for (int i = 0; i <= tamTexto - tamFrag; i++) {
        int ok = 1;
        for (int j = 0; j < tamFrag; j++) {
            if (texto[i + j] != fragmento[j]) {
                ok = 0;
                break;
            }
        }
        if (ok) return true;
    }
    return false;
}
//Logica pricipal resolucion
bool resolverCaso(const Byte* encriptado, int tamEnc, const char* pista, int tamPista,
                  int& metodo, int& rotacion, int& clave, char*& textoOriginal, int& tamOriginal) {
    textoOriginal = nullptr; tamOriginal = 0;

    for (int rot = 1; rot <= 7; rot++) {
        for (int k = 0; k < 256; k++) {

            Byte* plain = desencriptar(encriptado, tamEnc, rot, (Byte)k);
            if (!plain) continue;

            // 1. Intentar RLE ASCII si el patrón coincide
            char* deRLE = nullptr; int Lr = 0;
            if (pareceRLE_ASCII(plain, tamEnc)) {
                deRLE = descomprimirRLE_ASCII(plain, tamEnc, Lr);
                if (deRLE && contieneFragmento(deRLE, Lr, pista, tamPista)) {
                    metodo = 0; rotacion = rot; clave = k;
                    textoOriginal = deRLE; tamOriginal = Lr;
                    free(plain);
                    return true;
                }
                if (deRLE) free(deRLE);
            }

            // 2. Intentar LZ78 si la longitud es múltiplo de 3
            if ((tamEnc % 3) == 0) {
                char* deLZ = nullptr; int Ll = 0;
                deLZ = descomprimirLZ78(plain, tamEnc, Ll);
                if (deLZ && contieneFragmento(deLZ, Ll, pista, tamPista)) {
                    metodo = 1; rotacion = rot; clave = k;
                    textoOriginal = deLZ; tamOriginal = Ll;
                    free(plain);
                    return true;
                }
                if (deLZ) free(deLZ);
            }

            free(plain);
        }
    }
    return false;
}
