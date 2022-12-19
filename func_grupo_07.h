/*
 * UNIVERSIDAD NACIONAL DE ASUNCION
 * Facultad Politécnica – Ingeniería en Informática
 * Lenguaje de Programación I – Año 2022
 *
 *Descripción: Este trabajo práctico realizado será evaluado como el examen final de la asignatura.
 *
 *@autor/es: Joaquin Manuel Uliambre Frutos (Cedula de identidad: 5.360.716) y
            Zinri Alice Bobadilla Peralta (Cedula de identidad: 5.360.716)
 *@id-grupo: 7
 *@versión: 2.7
 *@fecha y hora última actualización: lunes, 19 de diciembre de 2022 20:00
 */

// Para compilar el programa usar la siguiente línea: gcc bot_grupo_07.c func_grupo_07.c -o bot -lws2_32

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
void horaActual(char *hora);
void socketCliente(FILE *registro, int modoLocal);
int seFormoCuadrado(int A[10][10], int buscar);
int cantVacias(int A[10][10]);
DWORD WINAPI enviarMensaje(LPVOID lpParam, char mensaje[BUFLEN], FILE *registro, int modoLocal);
void leer_mensaje(FILE *registro, char mensaje[], char *respuesta, int modoLocal);
void clonarMatriz(char *origen, char *destino);
int marcaServidor(char *tiempo);
int inferiorDerecho(int A[10][10], int x, int y, int buscar);
int superiorDerecho(int A[10][10], int x, int y, int buscar);
int inferiorIzquierdo(int A[10][10], int x, int y, int buscar);
int superiorIzquierdo(int A[10][10], int x, int y, int buscar);
int conOrientacion(int A[10][10], int x, int y, int buscar);
int validarPosicion(int tablero[10][10], int posicionFila, int posicionCol, int busqueda);
int verificarMensaje(char mensaje[], char *incorrecto);