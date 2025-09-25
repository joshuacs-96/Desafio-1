#ifndef FUNTIONS_H
#define FUNTIONS_H
typedef unsigned char Byte;
void expandirBuffer(char*& buffer, int& actualSize, int requerido);
char* descomprimirRLE(const Byte* compressed, int len, int& outLen);
char* descomprimirLZ78(const Byte* compressed, int len, int& outLen);
Byte rotateRight(Byte b, int n);
Byte* desencriptar(const Byte* encrypted, int len, int n, Byte k);
Byte* leerArchivoBinario(const char* nombre, int& tamano);
char* leerArchivoTexto(const char* nombre, int& tamano);
bool contieneFragmento(const char* texto, int tamTexto, const char* fragmento, int tamFrag);
bool resolverCaso(const Byte* encriptado, int tamEnc, const char* pista, int tamPista,
                  int& metodo, int& rotacion, int& clave, char*& textoOriginal, int& tamOriginal);
#endif // FUNTIONS_H
