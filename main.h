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

#pragma comment(lib, "Ws2_32.lib")

#define BUFLEN 512
#define PORT 9999
#define ADDRESS "127.0.0.1" // aka "localhost"
#define MAX_CLIENTS 1

void socketServidor();
DWORD WINAPI sendThreadFunc(LPVOID lpParam);
void socketCliente();
void leer_mensaje(FILE *, char mensaje[], char *respuesta);
DWORD WINAPI enviarMensaje(LPVOID lpParam, char mensaje[BUFLEN]);