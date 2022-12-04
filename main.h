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
int cantVacias(int A[10][10]);
void leer_mensaje(FILE *registro, char mensaje[], char *respuesta, int modoLocal);
DWORD WINAPI enviarMensaje(LPVOID lpParam, char mensaje[BUFLEN], FILE *registro);
int marcaServidor(char *tiempo);
int inferiorDerecho(int A[10][10], int x, int y, int buscar);
int superiorDerecho(int A[10][10], int x, int y, int buscar);
int inferiorIzquierdo(int A[10][10], int x, int y, int buscar);
int superiorIzquierdo(int A[10][10], int x, int y, int buscar);
int conOrientacion(int A[10][10], int x, int y, int buscar);
int validarPosicion(int tablero[10][10], int posicionFila, int posicionCol, int busqueda);
int validarJugada(int original[10][10], int nuevo[10][10], int x, int y, int jugador);