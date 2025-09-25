#include <iostream>
#include <fstream>
#include <cstdio> // Para sprintf
#include <cstdlib> // Para new y delete
#include "Funtions.h"
using namespace std;

int main() {
    int numArchivos = 4;
    cout << "Evaluando " << numArchivos << " archivos..." << endl;

    for (int caso = 1; caso <= numArchivos; caso++) {
        char nombreEnc[50];
        char nombrePista[50];
        sprintf(nombreEnc, "Encriptado%d.txt", caso);
        sprintf(nombrePista, "pista%d.txt", caso);

        int tamEnc, tamPista;
        Byte* encriptado = leerArchivoBinario(nombreEnc, tamEnc);
        char* pista = leerArchivoTexto(nombrePista, tamPista);

        if (!encriptado || !pista) {
            cout << "Error leyendo archivos para caso " << caso << endl;
            if (encriptado) delete[] encriptado;
            if (pista) delete[] pista;
            continue;
        }

        int metodo, rotacion, clave, tamOriginal;
        char* textoOriginal = nullptr;

        bool resuelto = resolverCaso(encriptado, tamEnc, pista, tamPista,
                                     metodo, rotacion, clave, textoOriginal, tamOriginal);

        cout << "\n=== CASO " << caso << " ===" << endl;
        if (resuelto) {
            cout << "Metodo de compresion: " << (metodo == 0 ? "RLE" : "LZ78") << endl;
            cout << "Rotacion: " << rotacion << " bits" << endl;
            cout << "Clave XOR: 0x" << hex << clave << dec << endl;
            cout << "\nTexto original reconstruido:" << endl;
            // Para no imprimir un texto potencialmente enorme, solo se imprimen los primeros 100 caracteres.
            for (int i = 0; i < 100 && i < tamOriginal; i++) {
                cout << textoOriginal[i];
            }
            cout << "..." << endl;

            delete[] textoOriginal;
        } else {
            cout << "No se pudo resolver el caso" << endl;
        }

        delete[] encriptado;
        delete[] pista;
    }

    return 0;
}
