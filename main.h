#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <process.h>
#include <stdatomic.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#pragma comment(lib, "Ws2_32.lib")

#define BUFLEN 1000
#define PORT 9999
#define ADDRESS "127.0.0.1" // aka "localhost"
#define MAX_CLIENTS 1

void socketServidor(FILE *registro, int modoLocal);
void socketCliente(FILE *registro, int modoLocal);
void leer_mensaje(FILE *registro, char mensaje[], char *respuesta, int modoLocal);
DWORD WINAPI enviarMensaje(LPVOID lpParam, char mensaje[BUFLEN]);
int marcaServidor(char *tiempo);