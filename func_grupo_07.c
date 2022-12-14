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

#include "func_grupo_07.h"

// variables globales
int running = 0;
int jugadasContrario = 0;
int jugadasMio = 0;
int duracionTotal = 0;
int duracionMio = 0;
int duracionContrario = 0;
char inicioJuego[20];
char finJuego[20];
int matrizEnviada[10][10];
int finalizadoLleno = 0;

//  motivos de fallido
int tardoMucho = 0;
int mensajeValido = 1;
int jugadaTrampa = 0;
char msjInvalido[300];

// variables del cuadrado formado
int coordCuadrado[8];

void socketServidor(FILE *registro, int modoLocal)
{
    int res, sendRes;

    // INICIALIZACION ==============================
    WSADATA wsaData; // configuration data
    res = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (res)
    {
        printf("Inicio fallo: %d\n", res);
    }
    //==============================================

    // CONFIGURAR SERVER =================================

    // constructor socket
    SOCKET listener;
    listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listener == INVALID_SOCKET)
    {
        printf("Error con constructor: %d\n", WSAGetLastError());
        WSACleanup();
    }

    // unir a la direccion
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ADDRESS);
    address.sin_port = htons(PORT);
    res = bind(listener, (struct sockaddr *)&address, sizeof(address));

    if (res == SOCKET_ERROR)
    {
        printf("Error al unir: %d\n", WSAGetLastError());
        closesocket(listener);
        WSACleanup();
    }

    // definir para escuchar
    res = listen(listener, SOMAXCONN);
    if (res == SOCKET_ERROR)
    {
        printf("Error al escuchar: %d\n", WSAGetLastError());
        closesocket(listener);
        WSACleanup();
    }
    //==============================================

    printf("Aceptando en %s:%d\n", ADDRESS, PORT);

    // HANDLE A CLIENT =============================

    // aceptar socket cliente
    SOCKET client;
    struct sockaddr_in clientAddr;
    int clientAddrlen;
    client = accept(listener, NULL, NULL);
    if (client == INVALID_SOCKET)
    {
        printf("No se pudo aceptar: %d\n", WSAGetLastError());
        closesocket(listener);
        WSACleanup();
    }
    {
        running = 1;
    }

    // get client information
    getpeername(client, (struct sockaddr *)&clientAddr, (socklen_t *)&clientAddr);
    printf("Cliente conectado en %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

    // receive messages
    char recvbuf[BUFLEN];

    do
    {
        res = recv(client, recvbuf, BUFLEN, 0);
        if (res > 0)
        {
            recvbuf[res] = '\0';
            printf("Message received (%d): %s\n", res, recvbuf);

            // enviar mensaje
            char respuesta[1000];
            leer_mensaje(registro, recvbuf, respuesta, modoLocal);
            enviarMensaje(&client, respuesta, registro, modoLocal);

            if (running == 0)
            {
                break;
            }
        }
        else if (!res)
        {
            // client disconnected
            printf("Cerrando conexion.\n");
            break;
        }
        else
        {
            printf("Error al recibir: %d\n", WSAGetLastError());
            break;
        }
    } while (res > 0);

    // shutdown client
    res = shutdown(client, SD_BOTH);
    if (res == SOCKET_ERROR)
    {
        printf("Error al apagar cliente: %d\n", WSAGetLastError());
    }
    closesocket(client);
    // =============================================

    // LIMPIEZA =====================================

    // apagar socket servidor
    closesocket(listener);

    // limpiar WSA
    res = WSACleanup();
    if (res)
    {
        printf("Error al limpiar socket: %d\n", res);
    }
    //==============================================
}

void horaActual(char *hora)
{
    time_t t;
    struct tm *tm;
    t = time(NULL);
    tm = localtime(&t);
    strftime(hora, 100, "%H:%M:%S", tm);
}

void socketCliente(FILE *registro, int modoLocal)
{
    int res;

    // INICIALIZACION ================================

    WSADATA wsaData; // configuration data
    res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res)
    {
        printf("Error al iniciar: %d\n", res);
    }

    // ===============================================

    // CONFIGURAR SOCKET CLIENTE ===========================

    // constructor socket
    SOCKET client;
    client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == INVALID_SOCKET)
    {
        printf("Error en constructor: %d\n", WSAGetLastError());
        WSACleanup();
    }

    // conectar a direccion
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ADDRESS);
    address.sin_port = htons(PORT);
    res = connect(client, (struct sockaddr *)&address, sizeof(address));
    if (res == SOCKET_ERROR || client == INVALID_SOCKET)
    {
        printf("Error al conectar: %d\n", WSAGetLastError());
        closesocket(client);
        WSACleanup();
    }
    else
    {
        // se envia el primer mensaje cuando se logro conectarse
        char mensaje[1000] = "";
        strcat(mensaje, "1;");
        // hora en que vamos a enviar el mensaje
        char hora[100];
        horaActual(hora);
        strcat(hora, ";");
        strcat(mensaje, hora);
        strcat(mensaje, "*;cliente;7;*;conectar;pendiente;*;*;*;*;*;#.");
        enviarMensaje(&client, mensaje, registro, modoLocal);
    }

    // definimos la variable a true
    running = !0; // true

    // ===============================================

    // LOOP PRINCIPAL =====================================

    // loop de recibir
    char recvbuf[BUFLEN];

    do
    {
        res = recv(client, recvbuf, BUFLEN, 0);
        recvbuf[res] = '\0';

        if (res > 0)
        {
            printf("Mensaje recibido (%d): %s\n", res, recvbuf);
            char respuesta[1000];
            leer_mensaje(registro, recvbuf, respuesta, modoLocal);
            enviarMensaje(&client, respuesta, registro, modoLocal);
        }
        else if (!res)
        {
            printf("Coneccion cerrada.\n");
            running = 0;
        }
        else
        {
            printf("Error al recibir: %d.\n", WSAGetLastError());
            running = 0;
        }

    } while (running && res > 0);
    running = 0;

    // ===============================================

    // LIMPIEZA =======================================

    res = shutdown(client, SD_BOTH);
    if (res == SOCKET_ERROR)
    {
        printf("Error al apagar: %d\n", WSAGetLastError());
        closesocket(client);
        WSACleanup();
    }

    closesocket(client);
    WSACleanup();

    // ===============================================
}

int marcaServidor(char *tiempo)
{
    int tiempoSegundos = 0;
    int h, m, s;
    sscanf(tiempo, "%d:%d:%d", &h, &m, &s);
    tiempoSegundos = (h * 3600) + (m * 60) + s;
    return tiempoSegundos;
}

int inferiorDerecho(int A[10][10], int x, int y, int buscar)
{
    int seFormaCuadrado = 0;
    int index = 1;
    // ciclo para buscar las posiciones de cuadrados
    while (index < 10)
    {
        if (A[x][y + index] == buscar && A[x + index][y] == buscar &&
            A[x + index][y + index] == buscar && x + index <= 9 && y + index <= 9)
        {
            if (A[x][y] != 0)
            {
                coordCuadrado[0] = x;
                coordCuadrado[1] = y;
                coordCuadrado[2] = x;
                coordCuadrado[3] = y + index;
                coordCuadrado[4] = x + index;
                coordCuadrado[5] = y;
                coordCuadrado[6] = x + index;
                coordCuadrado[7] = y + index;
            }

            seFormaCuadrado = 1;
        }
        index++;
    }

    return seFormaCuadrado;
}

int superiorDerecho(int A[10][10], int x, int y, int buscar)
{
    int seFormaCuadrado = 0;
    int index = 1;
    // ciclo para buscar las posiciones de cuadrados
    while (index < 10)
    {
        if (A[x - index][y] == buscar && A[x][y + index] == buscar && A[x - index][y + index] == buscar &&
            x - index >= 0 && y + index <= 9)
        {
            if (A[x][y] != 0)
            {
                coordCuadrado[0] = x;
                coordCuadrado[1] = y;
                coordCuadrado[2] = x - index;
                coordCuadrado[3] = y;
                coordCuadrado[4] = x;
                coordCuadrado[5] = y + index;
                coordCuadrado[6] = x - index;
                coordCuadrado[7] = y + index;
            }
            seFormaCuadrado = 1;
        }
        index++;
    }

    return seFormaCuadrado;
}

int inferiorIzquierdo(int A[10][10], int x, int y, int buscar)
{
    int seFormaCuadrado = 0;
    int index = 1;

    // ciclo para buscar las posiciones de cuadrados
    while (index < 10)
    {
        if (A[x][y - index] == buscar && A[x + index][y] == buscar && A[x + index][y - index] == buscar && x + index <= 9 && y - index >= 0)
        {
            if (A[x][y] != 0)
            {
                coordCuadrado[0] = x;
                coordCuadrado[1] = y;
                coordCuadrado[2] = x;
                coordCuadrado[3] = y - index;
                coordCuadrado[4] = x + index;
                coordCuadrado[5] = y;
                coordCuadrado[6] = x + index;
                coordCuadrado[7] = y - index;
            }

            seFormaCuadrado = 1;
        }
        index++;
    }

    return seFormaCuadrado;
}

int superiorIzquierdo(int A[10][10], int x, int y, int buscar)
{
    int seFormaCuadrado = 0;
    int index = 1;
    // ciclo para buscar las posiciones de cuadrados
    while (index < 10)
    {
        if (A[x][y - index] == buscar && A[x - index][y] == buscar &&
            A[x - index][y - index] == buscar && x - index >= 0 && y - index >= 0)
        {
            if (A[x][y] != 0)
            {
                coordCuadrado[0] = x;
                coordCuadrado[1] = y;
                coordCuadrado[2] = x;
                coordCuadrado[3] = y - index;
                coordCuadrado[4] = x - index;
                coordCuadrado[5] = y;
                coordCuadrado[6] = x - index;
                coordCuadrado[7] = y - index;
            }

            seFormaCuadrado = 1;
        }
        index++;
    }
    return seFormaCuadrado;
}

int conOrientacion(int A[10][10], int x, int y, int buscar) // para los cuadradros inclinadas
{
    int seFormaCuadrado = 0;
    int indexF = 1;
    int x_buscada = 0, y_buscada = 0;
    while (indexF + x < 10)
    {
        int indexC = 0;
        while (indexC + y < 10)
        {
            if (A[x + indexF][y - indexC] == buscar && y - indexC >= 0)
            {
                float lado = sqrt(pow((x) - (x + indexF), 2) + pow((y) - (y - indexC), 2));
                float diagonal = lado * sqrt(2);
                // en busqueda de la 3ra posicion q cumpla
                /*FORMULA----------> dis = raiz de (x_buscada - x)**2 + (y_buscada - y)**2 */
                int auxX = 0, auxY = 0, termino = 0;
                while (auxX < 10 || termino < 0)
                {
                    auxY = 0;
                    while (auxY < 10 || termino < 0)
                    {
                        float val = sqrt(pow(auxX - (x + indexF), 2) + pow(auxY - (y - indexC), 2));
                        if (diagonal == val && A[auxX][auxY] == buscar)
                        {
                            float lado2 = sqrt(pow((x) - (auxX), 2) + pow((y) - (auxY), 2));
                            if (lado == lado2)
                            {
                                x_buscada = auxX;
                                y_buscada = auxY;
                                termino = 1;
                            }
                        }
                        auxY++;
                    }
                    auxX++;
                }

                int f = 0;
                while (x + f < 10)
                {
                    int c = 0;
                    while (y + c < 10)
                    {
                        float val2 = sqrt(pow((x + f) - x, 2) + pow((y + c) - y, 2));
                        float lado3 = sqrt(pow((x + f) - (x + indexF), 2) + pow((y + c) - (y - indexC), 2));
                        float lado4 = sqrt(pow((x + f) - (x_buscada), 2) + pow((y + c) - (y_buscada), 2));
                        if (A[x + f][y + c] == buscar && x + f < 10 && y + c < 10 && diagonal == val2 && lado == lado3 && lado == lado4)
                        {
                            if (((x + f) == x + indexF && (y + c) != y - indexC) || ((x + f) != x + indexF && (y + c) == y - indexC) || ((x + f) != x + indexF && (y + c) != y - indexC))
                            {
                                if (((x + f == x_buscada) && (y + c) != y_buscada) || ((x + f != x_buscada) && (y + c) == y_buscada) || ((x + f) != x + indexF && (y + c) != y - indexC))
                                {
                                    if (((x + f == x) && (y + c) != y) || ((x + f != x) && (y + c) == y) || ((x + f) != x && (y + c) != y))
                                    {
                                        coordCuadrado[0] = x;
                                        coordCuadrado[1] = y;
                                        coordCuadrado[2] = x + indexF;
                                        coordCuadrado[3] = y - indexC;
                                        coordCuadrado[4] = x_buscada;
                                        coordCuadrado[5] = y_buscada;
                                        coordCuadrado[6] = x + f;
                                        coordCuadrado[7] = y + c;
                                        seFormaCuadrado = 1;
                                    }
                                }
                            }
                        }
                        c++;
                    }
                    f++;
                }
            }
            indexC++;
        }
        indexF++;
    }
    return seFormaCuadrado;
}

int validarPosicion(int tablero[10][10], int posicionFila, int posicionCol, int busqueda)
{
    int resII = 0;
    int resSI = 0;
    int resID = 0;
    int resSD = 0;
    int resOrientacion = 0;

    if (posicionCol < 10 - 1 && posicionFila < 10 - 1)
    {
        resID = inferiorDerecho(tablero, posicionFila, posicionCol, busqueda);
    }

    if (posicionCol < 10 - 1 && posicionFila >= 1)
    {
        resSD = superiorDerecho(tablero, posicionFila, posicionCol, busqueda);
    }

    if (posicionCol >= 1 && posicionFila < 10 - 1)
    {
        resII = inferiorIzquierdo(tablero, posicionFila, posicionCol, busqueda);
    }

    if (posicionCol >= 1 && posicionFila >= 1)
    {
        resSI = superiorIzquierdo(tablero, posicionFila, posicionCol, busqueda);
    }

    resOrientacion = conOrientacion(tablero, posicionFila, posicionCol, busqueda);

    if (resID || resSD || resII || resSI || resOrientacion)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int seFormoCuadrado(int A[10][10], int buscar)
{
    int seFormoCuadrado = 0;

    for (int i = 0; i < 10 - 1; i++)
    {
        for (int j = 0; j < 10 - 1; j++)
        {
            int posicionFila = i;
            int posicionCol = j;
            int resultado = 0;
            // hay que validar que la posicion tenga el elemento a buscar
            if (A[i][j] == buscar)
            {
                // verificarmos si se formo un cuadrado
                resultado = validarPosicion(A, posicionFila, posicionCol, buscar);

                // si se formo, imprimimos mensaje
                if (resultado)
                {
                    seFormoCuadrado = 1;
                }
            }
        }
    }
    return seFormoCuadrado;
}

int cantVacias(int A[10][10])
{
    int vacias = 0;

    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            if (A[i][j] == 0)
            {
                vacias++;
            }
        }
    }

    return vacias;
}

int verificarMensaje(char mensaje[])
{
    char validos[] = "0123456789;*:#.,[]";
    char pal[] = "servidorclienteconectarempezariniciarjugarfinalizarpendienteconectadoactivo finalizado exitoso finalizado fallido";
    for (int i = 0; i < strlen(mensaje); i++)
    {
        int b = 0;
        for (int j = 0; j < strlen(validos); j++)
        {
            if (mensaje[i] == validos[j])
            {
                b = 1;
                break;
            }
        }
        if (b == 0)
        {
            for (int j = 0; j < strlen(pal); j++)
            {
                if (mensaje[i] == pal[j])
                {
                    b = 1;
                    break;
                }
            }
        }
        if (b == 0)
        {
            return 1;
        }
    }
    return 0;
}

void verificarMatrices(int matrizOriginal[10][10], int matrizNueva[10][10], int posFila, int posCol)
{
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            if (matrizOriginal[i][j] != matrizNueva[i][j])
            {
                if (i != posFila && j != posCol)
                {
                    jugadaTrampa = 1;
                }
            }
        }
    }
}

DWORD WINAPI enviarMensaje(LPVOID lpParam, char mensaje[BUFLEN], FILE *registro, int modoLocal)
{
    SOCKET client = *(SOCKET *)lpParam;

    char sendbuf[BUFLEN];
    int sendbuflen, res;

    sendbuflen = strlen(mensaje);
    res = send(client, mensaje, sendbuflen, 0);

    if (res != sendbuflen)
    {
        printf("Send failed.\n");
    }

    if (strlen(mensaje) > 0)
    {
        printf("Mensaje ENVIADO: %s\n", mensaje);
    }

    if (verificarMensaje(mensaje) == 1)
    {
        mensajeValido = 0;
        memcpy(msjInvalido, mensaje, strlen(mensaje) + 1);
    }
    char delimitador[] = "[];#";
    char *token = strtok(mensaje, delimitador);
    int j = 0;
    char *aux;
    char salto = '\n';
    char *jugadas;

    int us = 0;
    int contrario = 0;

    // datos del mensaje a enviar
    char programa[20];
    char estadoEnviado[20];
    int duracionEnviado = 0;
    char tiempoEnviado[20];
    char origenEnviado[20];
    char destinoEnviado[20];
    char programaEnviado[20];
    char tableroNuestro[20];

    if (modoLocal)
    {
        strcpy(programa, "servidor");
        us = 1;
        contrario = 2;
    }
    else
    {
        strcpy(programa, "cliente");
        us = 2;
        contrario = 1;
    }

    if (strlen(mensaje) > 0)
    {
        fprintf(registro, "######Programa:%s\n", programa);
    }
    strcat(programa, ";");
    if (token != NULL)
    {
        while (token != NULL)
        {
            // Separamos el mensaje de acuerdo al valor de j
            if (j == 0) // id mensaje
            {
                fprintf(registro, "Mensaje-id:%s", token);
            }
            else if (j == 1) // marca temporal
            {
                fprintf(registro, "     Marca-de-tiempo:%s. ", token);
                strcpy(tiempoEnviado, token);
            }
            else if (j == 2) // duracion
            {
                fprintf(registro, "      Duracion:%s", token);
                duracionEnviado = atoi(token);
                if (duracionEnviado > 6)
                {
                    tardoMucho = 1;
                }
                fprintf(registro, "\nMensaje-tipo: enviado.");
            }
            else if (j == 3) // programa
            {
                strcpy(programaEnviado, token);
            }
            else if (j == 4) // origen
            {
                fprintf(registro, "      Mensaje-origen:%s.", token);
                strcpy(origenEnviado, token);
            }
            else if (j == 5) // destino
            {
                fprintf(registro, "      Mensaje-destino:%s.\n", token);
                strcpy(destinoEnviado, token);
            }
            else if (j == 6) // evento
            {
                fprintf(registro, "Evento:%s.", token);
                if (strcmp(token, "jugar") == 0)
                {
                    jugadasMio++;
                    duracionMio += duracionEnviado;
                }
            }
            else if (j == 7) // estado
            {
                fprintf(registro, "      Estado-juego:%s.\n", token);
                strcpy(estadoEnviado, token);
            }
            else if (j == 8) // jugada
            {
                jugadas = token;
                if (strcmp(token, "1") == 0)
                {
                    strcpy(inicioJuego, tiempoEnviado);
                }
            }
            else if (j == 9) // turno
            {
                fprintf(registro, "------------Turno-jugador:Jugador-%s. ", token);
                fprintf(registro, "Numero-jugada:%s.------------\n", jugadas);
            }
            else if (j == 10) // posicion en x
            {
                aux = token;
            }
            else if (j == 11) // posicion en y
            {
                fprintf(registro, "Casilla-jugada:(%s,%s).\n", aux, token);
            }
            else if (j == 12) // TABLERO!!!!!!!!
            {
                if (strlen(token) > 1)
                {
                    fprintf(registro, "Tablero-actual:\n");
                }
                else
                {
                    fprintf(registro, "Tablero-actual: *\n");
                }
                int cont = 0, // contador que hace los saltos de linea
                    k = 0;    // contador de caracteres dentro del tablero
                strcpy(tableroNuestro, token);
                /// guardamos el tablero en el archivo con el formato deseado
                while (k < strlen(token) && strlen(token) > 1) // mientas no leamos todo el tablero
                {
                    if (cont == 10)
                    {
                        fputc(salto, registro);
                        cont = 0;
                    }
                    if (cont == 9)
                    {
                        fputc(token[k], registro);
                    }
                    else
                    {
                        fputc(token[k], registro);
                        fputc(token[k + 1], registro);
                    }
                    k = k + 2;
                    cont++;
                }

                if (strlen(token) > 1)
                {
                    int fila = 0, columna = 0, p = 0, contador = 1;
                    char auxi[1];

                    while (p < strlen(token))
                    {

                        if (contador % 10 == 0) // para hacer el cambio de fila
                        {
                            auxi[0] = token[p];
                            matrizEnviada[fila][columna] = atoi(auxi) / 10;
                            fila++;
                            columna = 0;
                        }
                        else
                        {
                            auxi[0] = token[p];

                            matrizEnviada[fila][columna] = atoi(auxi) / 10;
                            columna++;
                        }
                        p = p + 2;
                        contador++;
                    }

                    // imprimir datos de matriz
                    for (int fila = 0; fila < 10; fila++)
                    {
                        for (int columna = 0; columna < 10; columna++)
                        {
                            printf("%d ", matrizEnviada[fila][columna]);
                        }
                        printf("\n");
                    }
                }
            }
            // Sólo en la primera pasamos la cadena; en las siguientes pasamos NULL
            token = strtok(NULL, delimitador);
            j++;
        }
    }
    fputc(salto, registro);

    if (strcmp(estadoEnviado, "finalizado exitoso") == 0)
    {
        printf("\n====================fin del juego====================\n");
        if (finalizadoLleno)
        {
            printf("Grupo Ganador: 7 y %s\n", destinoEnviado);
            printf("Jugador ganador: %s\n", "jugador 1 y jugador 2");
            printf("Modalidad ganadora: %s\n", "Local y Visita");
        }
        else
        {
            printf("Grupo Ganador: 7\n");
            if (strcmp(programa, "servidor;") == 0)
            {
                printf("Jugador ganador: %s\n", "jugador 1");
                printf("Modalidad ganadora: %s\n", "Local");
            }
            else
            {
                printf("Jugador ganador: %s\n", "jugador 2");
                printf("Modalidad ganadora: %s\n", "Visita");
            }
        }

        if (modoLocal)
        {
            printf("Total-jugadas-jugador1: %d\n", jugadasMio);
            printf("Total-jugadas-jugador2: %d\n", jugadasContrario);
            printf("Tiempo-promedio-jugadas-jugador1: %f\n", duracionMio / jugadasMio);
            printf("Tiempo-promedio-jugadas-jugador2: %f\n", duracionContrario / jugadasContrario);
        }
        else
        {
            printf("Total-jugadas-jugador1: %d\n", jugadasContrario);
            printf("Total-jugadas-jugador2: %d\n", jugadasMio);
            printf("Tiempo-promedio-jugadas-jugador1:%f\n", duracionContrario / jugadasContrario);
            printf("Tiempo-promedio-jugadas-jugador2: %f\n", duracionMio / jugadasMio);
        }
        printf("Inicio juego: %s\n", inicioJuego);
        printf("Fin juego: %s\n", tiempoEnviado);
        printf("Duracion total: %d\n", duracionMio + duracionContrario);
        if (finalizadoLleno != 1)
        {
            printf("Coordenadas del cuadrado: (%d,%d);(%d,%d);(%d,%d);(%d,%d)\n",
                   coordCuadrado[0] + 1, coordCuadrado[1] + 1, coordCuadrado[2] + 1, coordCuadrado[3] + 1, coordCuadrado[4] + 1,
                   coordCuadrado[5] + 1, coordCuadrado[6] + 1, coordCuadrado[7] + 1);
        }
        printf("\n");
        // agregamos al archivo el mensaje final
        fprintf(registro, "====================fin del juego====================\n");
        if (finalizadoLleno)
        {
            fprintf(registro, "Grupo Ganador: 7 y %s\n", destinoEnviado);
            fprintf(registro, "Jugador ganador: %s\n", "jugador 1 y jugador 2");
            fprintf(registro, "Modalidad ganadora: %s\n", "Local y Visita");
        }
        else
        {
            fprintf(registro, "Grupo-ganador: 7\n");
            if (strcmp(programa, "servidor;") == 0)
            {
                fprintf(registro, "Jugador-ganador: Jugador 1\n");
                fprintf(registro, "Modalidad-partida-ganada: Local\n");
            }
            else
            {
                fprintf(registro, "Jugador-ganador: Jugador 2\n");
                fprintf(registro, "Modalidad-partida-ganada: Visita\n");
            }
        }
        if (modoLocal)
        {
            fprintf(registro, "Total-jugadas-jugador1: %d\n", jugadasMio);
            fprintf(registro, "Total-jugadas-jugador2: %d\n", jugadasContrario);
            fprintf(registro, "Tiempo-promedio-jugadas-jugador1: %f\n", duracionMio / jugadasMio);
            fprintf(registro, "Tiempo-promedio-jugadas-jugador2: %f\n", duracionContrario / jugadasContrario);
        }
        else
        {
            fprintf(registro, "Total-jugadas-jugador1: %d\n", jugadasContrario);
            fprintf(registro, "Total-jugadas-jugador2: %d\n", jugadasMio);
            fprintf(registro, "Tiempo-promedio-jugadas-jugador1: %f\n", duracionContrario / jugadasContrario);
            fprintf(registro, "Tiempo-promedio-jugadas-jugador2: %f\n", duracionMio / jugadasMio);
        }
        fprintf(registro, "Inicio del juego: %s\n", inicioJuego);
        fprintf(registro, "Finalización del juego: %s\n", tiempoEnviado);
        fprintf(registro, "Duracion-total-del-juego: %d\n", duracionMio + duracionContrario);
        if (finalizadoLleno != 1)
        {
            fprintf(registro, "Coordenadas del cuadrado: (%d,%d);(%d,%d);(%d,%d);(%d,%d)\n",
                    coordCuadrado[0] + 1, coordCuadrado[1] + 1, coordCuadrado[2] + 1, coordCuadrado[3] + 1, coordCuadrado[4] + 1,
                    coordCuadrado[5] + 1, coordCuadrado[6] + 1, coordCuadrado[7] + 1);
        }
        running = 0;
    }
    else if (strcmp(estadoEnviado, "finalizado fallido") == 0)
    {
        printf("\n====================fin del juego====================\n");
        printf("Inicio juego: %s\n", inicioJuego);
        printf("Fin juego: %s\n", tiempoEnviado);
        printf("Duracion total: %d\n", duracionMio + duracionContrario);
        if (tardoMucho)
        {
            printf("Tardo mas de 6 segundos en responder\n");
        }
        else if (mensajeValido == 0)
        {
            printf("Motivo error: Caracter invalido en mensaje\n");
            printf("Mensaje error: %s\n", msjInvalido);
        }
        else if (jugadaTrampa)
        {
            printf("Motivo error: Se encontro trampa en la jugada\n");
        }

        // agregamos al archivo el mensaje final
        fprintf(registro, "====================fin del juego====================\n");
        fprintf(registro, "Inicio del juego: %s\n", inicioJuego);
        fprintf(registro, "Finalización del juego: %s\n", tiempoEnviado);
        fprintf(registro, "Duracion-total-del-juego: %d\n", duracionMio + duracionContrario);
        if (tardoMucho)
        {
            fprintf(registro, "Motivo error: Tardo mas de 6 segundos en responder\n");
        }
        else if (mensajeValido == 0)
        {
            fprintf(registro, "Motivo error: Caracter invalido en mensaje\n");
            fprintf(registro, "Mensaje error: %s\n", msjInvalido);
        }
        else if (jugadaTrampa)
        {
            fprintf(registro, "Motivo error: Se encontro trampa en la jugada\n");
        }
        running = 0;
    }
}

void leer_mensaje(FILE *registro, char mensaje[], char *respuesta, int modoLocal)
{
    // limpiamos la respuesta antes de concatenarle nuevos valores
    memset(respuesta, '\0', strlen(respuesta));
    if (verificarMensaje(mensaje) == 1)
    {
        mensajeValido = 0;
        memcpy(msjInvalido, mensaje, strlen(mensaje) + 1);
    }
    char delimitador[] = "[];#";
    char *token = strtok(mensaje, delimitador);
    int j = 0;
    char *aux;
    char salto = '\n';
    int contrario = 0;
    int us = 0;
    char *jugadas;
    // datos del mensaje
    int tiempoRecibido = 0;
    char tiempRecibidoStr[20];
    int duracionRecibido = 0;
    char eventoRecibido[20];
    char origenRecibido[20];
    char estadoRecibido[20];
    char jugadaRecibido[20];
    char turnoRecibido[3];
    char xRecibido[20];
    char yRecibido[20];
    char tableroRecibido[199];
    // datos para el nuevo mensaje
    char nuevoId[20];
    char duracion[20];
    char programa[20];
    char eventoEnviar[20];
    char origenEnviar[20];
    char destinoEnviar[20];
    char estadoEnviar[21];
    char jugadaEnviar[20];
    char turnoEnviar[3];
    char xEnviar[20];
    char yEnviar[20];
    char tableroEnviar[300];
    srand(getpid());
    strcpy(origenEnviar, "7;");
    if (modoLocal)
    {
        strcpy(programa, "cliente");
        us = 1;
        contrario = 2;
    }
    else
    {
        strcpy(programa, "servidor");
        us = 2;
        contrario = 1;
    }

    fprintf(registro, "######Programa:%s\n", programa);
    strcat(programa, ";");
    if (token != NULL)
    {
        while (token != NULL)
        {
            // Separamos el mensaje de acuerdo al valor de j
            if (j == 0) // id mensaje
            {
                // agregamos el valor al archivo
                fprintf(registro, "Mensaje-id:%s", token);
                // cambiamos el valor para concatenar al mensaje de respuesta
                int x = atoi(token);
                itoa(x + 1, nuevoId, 10); // convertimos el nuevo id a string
                strcat(nuevoId, ";");     // agregamos ; al atributoId
            }
            else if (j == 1) // marca temporal
            {
                strcpy(tiempRecibidoStr, token);
                tiempoRecibido = marcaServidor(token);
                fprintf(registro, "     Marca-de-tiempo:%s. ", token);
            }
            else if (j == 2) // duracion
            {
                fprintf(registro, "      Duracion:%s", token);
                duracionRecibido = atoi(token);
                fprintf(registro, "\nMensaje-tipo: recibido.");
            }
            else if (j == 4) // origen
            {
                fprintf(registro, "     Mensaje-origen:%s. ", token);
                strcpy(destinoEnviar, token);
                strcat(destinoEnviar, ";");
            }
            else if (j == 5) // destino
            {
                fprintf(registro, "     Mensaje-destino:%s.\n", token);
            }
            else if (j == 6) // evento
            {
                fprintf(registro, "Evento:%s.", token);
                strcpy(eventoRecibido, token);
            }
            else if (j == 7) // estado
            {
                fprintf(registro, "     Estado-juego:%s.\n", token);
                strcpy(estadoRecibido, token);
            }
            else if (j == 8) // jugada
            {
                jugadas = token;
                if (strcmp(token, "*") != 0)
                {
                    // cambiamos el valor para concatenar al mensaje de respuesta
                    int x = atoi(token);
                    itoa(x + 1, jugadaEnviar, 10); // convertimos el nuevo id a string
                    strcat(jugadaEnviar, ";");     // agregamos ; al atributoId
                }

                if (strcmp(token, "1") == 0)
                {
                    strcpy(inicioJuego, tiempRecibidoStr);
                }
            }
            else if (j == 9) // turno
            {
                fprintf(registro, "------------Turno-jugador:Jugador-%s. ", token);
                fprintf(registro, "Numero-jugada:%s.------------\n", jugadas);
            }
            else if (j == 10) // posicion en x
            {
                aux = token;
                strcpy(xRecibido, aux);
            }
            else if (j == 11) // posicion en y
            {
                strcpy(yRecibido, token);
                fprintf(registro, "Casilla-jugada:(%s,%s).\n", aux, token);
            }
            else if (j == 12) // TABLERO!!!!!!!!
            {
                if (strlen(token) > 1)
                {
                    fprintf(registro, "Tablero-actual:\n");
                }
                else
                {
                    fprintf(registro, "Tablero-actual: *\n");
                }
                strcpy(tableroRecibido, token);
                int cont = 0, // contador que hace los saltos de linea
                    k = 0;    // contador de caracteres dentro del tablero

                /// guardamos el tablero en el archivo con el formato deseado
                while (k < strlen(token) && strlen(token) > 1) // mientas no leamos todo el tablero
                {
                    if (cont == 10)
                    {
                        fputc(salto, registro);
                        cont = 0;
                    }
                    if (cont == 9)
                    {
                        fputc(token[k], registro);
                    }
                    else
                    {
                        fputc(token[k], registro);
                        fputc(token[k + 1], registro);
                    }
                    k = k + 2;
                    cont++;
                }
            }
            // Sólo en la primera pasamos la cadena; en las siguientes pasamos NULL
            token = strtok(NULL, delimitador);
            j++;
        }
    }
    fputc(salto, registro);

    if (modoLocal)
    {
        strcpy(programa, "servidor;");
    }
    else
    {
        strcpy(programa, "cliente;");
    }

    if (duracionRecibido > 6)
    {
        tardoMucho = 1;
    }

    if (strcmp(eventoRecibido, "conectar") == 0 && tardoMucho == 0 && mensajeValido != 0)
    {
        strcpy(eventoEnviar, "iniciar;");
        strcpy(estadoEnviar, "conectado;");
        strcpy(jugadaEnviar, "*;");
        strcpy(turnoEnviar, "*;");
        strcpy(xEnviar, "*;");
        strcpy(yEnviar, "*;");
        strcpy(tableroEnviar, "*;");

        // hora en que vamos a enviar el mensaje
        char hora[100];
        horaActual(hora);
        int tiempoEnviar = marcaServidor(hora);
        int duracionSegundos = tiempoEnviar - tiempoRecibido;

        itoa(duracionSegundos, duracion, 10);
        strcat(duracion, ";");
        strcat(hora, ";");
        // concatenamos todos los nuevos datos a respuesta
        strcat(respuesta, nuevoId);
        strcat(respuesta, hora);
        strcat(respuesta, duracion);
        strcat(respuesta, programa);
        strcat(respuesta, origenEnviar);
        strcat(respuesta, destinoEnviar);
        strcat(respuesta, eventoEnviar);
        strcat(respuesta, estadoEnviar);
        strcat(respuesta, jugadaEnviar);
        strcat(respuesta, turnoEnviar);
        strcat(respuesta, xEnviar);
        strcat(respuesta, yEnviar);
        strcat(respuesta, tableroEnviar);
        strcat(respuesta, "#.");
    }
    else if (strcmp(eventoRecibido, "iniciar") == 0 && tardoMucho == 0 && mensajeValido != 0)
    {
        strcpy(eventoEnviar, "empezar;");
        strcpy(estadoEnviar, "activo;");
        strcpy(jugadaEnviar, "*;");
        strcpy(turnoEnviar, "*;");
        strcpy(xEnviar, "*;");
        strcpy(yEnviar, "*;");
        strcpy(tableroEnviar, "*;");

        // hora en que vamos a enviar el mensaje
        char hora[100];
        horaActual(hora);
        int tiempoEnviar = marcaServidor(hora);
        int duracionSegundos = tiempoEnviar - tiempoRecibido;

        itoa(duracionSegundos, duracion, 10);
        strcat(duracion, ";");
        strcat(hora, ";");
        // concatenamos todos los nuevos datos a respuesta
        strcat(respuesta, nuevoId);
        strcat(respuesta, hora);
        strcat(respuesta, duracion);
        strcat(respuesta, programa);
        strcat(respuesta, origenEnviar);
        strcat(respuesta, destinoEnviar);
        strcat(respuesta, eventoEnviar);
        strcat(respuesta, estadoEnviar);
        strcat(respuesta, jugadaEnviar);
        strcat(respuesta, turnoEnviar);
        strcat(respuesta, xEnviar);
        strcat(respuesta, yEnviar);
        strcat(respuesta, tableroEnviar);
        strcat(respuesta, "#.");
    }
    else if (strcmp(eventoRecibido, "empezar") == 0)
    {
        strcpy(eventoEnviar, "jugar;");
        strcpy(estadoEnviar, "activo;");
        strcpy(jugadaEnviar, "1;");
        itoa(us, turnoEnviar, 10);
        strcat(turnoEnviar, ";");
        int matriz[10][10];

        // todos los elementos de matriz en cero
        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
            {

                matriz[i][j] = 0;
            }
        }
        // posicion en donde haremos la jugada
        int i = rand() % 10, j = rand() % 10;

        matriz[i][j] = us;
        // agregamos las posiciones a nuestras variables para luego enviar el mensaje
        itoa((i + 1), xEnviar, 10);
        itoa((j + 1), yEnviar, 10);

        // cargamos al tablero nuevo para luego agregarlo a la respuesta
        tableroEnviar[0] = '[';
        int f = 0, c = 0, val = 1, cont = 1;

        while (val < 200)
        {
            if (cont % 10 == 0) // para hacer el cambio de fila
            {
                tableroEnviar[val] = matriz[f][c] + '0';
                tableroEnviar[val + 1] = ',';
                f++;
                c = 0;
            }
            else
            {
                tableroEnviar[val] = matriz[f][c] + '0';
                tableroEnviar[val + 1] = ',';
                c++;
            }
            val = val + 2;
            cont++;
        }
        tableroEnviar[val - 1] = ']';
        tableroEnviar[val] = '\0';
        strcat(xEnviar, ";");
        strcat(yEnviar, ";");
        strcat(tableroEnviar, ";");

        // hora en que vamos a enviar el mensaje
        char hora[100];
        horaActual(hora);
        int tiempoEnviar = marcaServidor(hora);
        int duracionSegundos = tiempoEnviar - tiempoRecibido;

        itoa(duracionSegundos, duracion, 10);
        strcat(duracion, ";");
        strcat(hora, ";");
        // concatenamos todos los nuevos datos a respuesta
        strcat(respuesta, nuevoId);
        strcat(respuesta, hora);
        strcat(respuesta, duracion);
        strcat(respuesta, programa);
        strcat(respuesta, origenEnviar);
        strcat(respuesta, destinoEnviar);
        strcat(respuesta, eventoEnviar);
        strcat(respuesta, estadoEnviar);
        strcat(respuesta, jugadaEnviar);
        strcat(respuesta, turnoEnviar);
        strcat(respuesta, xEnviar);
        strcat(respuesta, yEnviar);
        strcat(respuesta, tableroEnviar);
        strcat(respuesta, "#.");
    }
    else if (strcmp(estadoRecibido, "activo") == 0 && strcmp(eventoRecibido, "jugar") == 0 && tardoMucho == 0 && mensajeValido != 0 && jugadaTrampa == 0)
    {
        int fila = 0, columna = 0, p = 0, contador = 1;
        char auxi[1];
        int matriz[10][10];
        while (p < strlen(tableroRecibido))
        {
            if (contador % 10 == 0) // para hacer el cambio de fila
            {
                auxi[0] = tableroRecibido[p];
                auxi[1] = '\0';
                matriz[fila][columna] = atoi(auxi);

                fila++;
                columna = 0;
            }
            else
            {
                auxi[0] = tableroRecibido[p];
                auxi[1] = '\0';
                matriz[fila][columna] = atoi(auxi);
                columna++;
            }
            p = p + 2;
            contador++;
        }
        // imprimir datos de matriz
        for (int fila = 0; fila < 10; fila++)
        {
            for (int columna = 0; columna < 10; columna++)
            {
                printf("%d ", matriz[fila][columna]);
            }
            printf("\n");
        }

        // verificar si hay trampa
        verificarMatrices(matrizEnviada, matriz, atoi(xRecibido) - 1, atoi(yRecibido) - 1);

        // cargamos datos del contrario
        jugadasContrario++;
        duracionContrario += duracionRecibido;

        int seFormo_contrario = seFormoCuadrado(matriz, contrario);
        int t_lleno = 1, row = 0;
        while (row < 10 || t_lleno != 0) // verificamos si el tablero está lleno
        {
            int col = 0;
            while (col < 10 || t_lleno != 0)
            {
                if (matriz[row][col] == 0)
                {
                    t_lleno = 0;
                }
                col++;
            }
            row++;
        }

        if (!seFormo_contrario && t_lleno == 0)
        {
            // posicion en donde haremos la jugada
            int ban = 0;
            int v = cantVacias(matriz); // contar la cantidad de posiciones con 0
            int contar = 0;
            int validador = 0;
            while (ban != 1)
            {
                int i = rand() % 10, j = rand() % 10;

                int resultado = 0;
                // hay que validar que la posicion no tenga elemento
                if (matriz[i][j] == 0)
                {
                    // verificarmos si se forma un cuadrado para nosotros
                    resultado = validarPosicion(matriz, i, j, us);
                    // si no se forma para ninguno, colocamos nuestro valor
                    if (!resultado)
                    {
                        matriz[i][j] = us;
                        // agregamos las posiciones a nuestras variables para luego enviar el mensaje
                        itoa((i + 1), xEnviar, 10);
                        itoa((j + 1), yEnviar, 10);
                        ban = 1;
                        validador = 1;
                    }
                    if (contar == v)
                    {
                        ban = 1;
                    }
                    contar++;
                }
            }
            // si sale igual, significa que ya no hay otras posiciones donde no se puede hacer cuadrado
            if (contar >= v && validador == 0)
            {
                int coloco = 0;
                for (int i = 0; i < 10 - 1; i++)
                {
                    for (int j = 0; j < 10 - 1; j++)
                    {
                        if (matriz[i][j] == 0 && coloco != 1)
                        {
                            matriz[i][j] = us;
                            coloco = 1;
                            itoa((i + 1), xEnviar, 10);
                            itoa((j + 1), yEnviar, 10);
                        }
                    }
                }
            }
            // cargamos al tablero nuevo para luego agregarlo a la respuesta
            tableroEnviar[0] = '[';
            int f = 0, c = 0, val = 1, cont = 1;

            while (val < 200)
            {
                if (cont % 10 == 0) // para hacer el cambio de fila
                {
                    tableroEnviar[val] = matriz[f][c] + '0';
                    tableroEnviar[val + 1] = ',';
                    f++;
                    c = 0;
                }
                else
                {
                    tableroEnviar[val] = matriz[f][c] + '0';
                    tableroEnviar[val + 1] = ',';
                    c++;
                }
                val = val + 2;
                cont++;
            }

            tableroEnviar[val - 1] = ']';
            tableroEnviar[val] = '\0';
            strcat(tableroEnviar, ";");
            strcat(xEnviar, ";");
            strcat(yEnviar, ";");
            strcpy(eventoEnviar, "jugar;");
            strcpy(estadoEnviar, "activo;");
            itoa(us, turnoEnviar, 10);
            strcat(turnoEnviar, ";");

            // hora en que vamos a enviar el mensaje
            char hora[100];
            horaActual(hora);
            int tiempoEnviar = marcaServidor(hora);
            int duracionSegundos = tiempoEnviar - tiempoRecibido;
            itoa(duracionSegundos, duracion, 10);
            strcat(duracion, ";");
            strcat(hora, ";");

            // concatenamos todos los nuevos datos a respuesta
            strcat(respuesta, nuevoId);
            strcat(respuesta, hora);
            strcat(respuesta, duracion);
            strcat(respuesta, programa);
            strcat(respuesta, origenEnviar);
            strcat(respuesta, destinoEnviar);
            strcat(respuesta, eventoEnviar);
            strcat(respuesta, estadoEnviar);
            strcat(respuesta, jugadaEnviar);
            strcat(respuesta, turnoEnviar);
            strcat(respuesta, xEnviar);
            strcat(respuesta, yEnviar);
            strcat(respuesta, tableroEnviar);
            strcat(respuesta, "#.");
        }
        else if (seFormo_contrario)
        {
            // hora en que vamos a enviar el mensaje
            char hora[100];
            horaActual(hora);
            int tiempoEnviar = marcaServidor(hora);
            int duracionSegundos = tiempoEnviar - tiempoRecibido;
            itoa(duracionSegundos, duracion, 10);
            strcat(duracion, ";");
            strcat(hora, ";");
            // concatenamos todos los nuevos datos a respuesta
            strcpy(eventoEnviar, "finalizar;");
            strcpy(estadoEnviar, "finalizado exitoso;");
            strcpy(turnoEnviar, "*;");
            strcpy(xEnviar, "*;");
            strcpy(yEnviar, "*;");
            strcpy(tableroEnviar, "*;");
            strcpy(jugadaEnviar, "*;");

            strcat(respuesta, nuevoId);
            strcat(respuesta, hora);
            strcat(respuesta, duracion);
            strcat(respuesta, programa);
            strcat(respuesta, origenEnviar);
            strcat(respuesta, destinoEnviar);
            strcat(respuesta, eventoEnviar);
            strcat(respuesta, estadoEnviar);
            strcat(respuesta, jugadaEnviar);
            strcat(respuesta, turnoEnviar);
            strcat(respuesta, xEnviar);
            strcat(respuesta, yEnviar);
            strcat(respuesta, tableroEnviar);
            strcat(respuesta, "#.");
            running = 0;
        }
        else if (t_lleno == 1)
        {
            // hora en que vamos a enviar el mensaje
            char hora[100];
            horaActual(hora);
            int tiempoEnviar = marcaServidor(hora);
            int duracionSegundos = tiempoEnviar - tiempoRecibido;
            itoa(duracionSegundos, duracion, 10);
            strcat(duracion, ";");
            strcat(hora, ";");
            // concatenamos todos los nuevos datos a respuesta
            strcpy(eventoEnviar, "finalizar;");
            strcpy(estadoEnviar, "finalizado exitoso;");
            strcpy(turnoEnviar, "*;");
            strcpy(xEnviar, "*;");
            strcpy(yEnviar, "*;");
            strcpy(tableroEnviar, "*;");
            strcpy(jugadaEnviar, "*;");

            strcat(respuesta, nuevoId);
            strcat(respuesta, hora);
            strcat(respuesta, duracion);
            strcat(respuesta, programa);
            strcat(respuesta, origenEnviar);
            strcat(respuesta, destinoEnviar);
            strcat(respuesta, eventoEnviar);
            strcat(respuesta, estadoEnviar);
            strcat(respuesta, jugadaEnviar);
            strcat(respuesta, turnoEnviar);
            strcat(respuesta, xEnviar);
            strcat(respuesta, yEnviar);
            strcat(respuesta, tableroEnviar);
            strcat(respuesta, "#.");
            finalizadoLleno = 1;
            running = 0;
        }
    }
    else if (strcmp(eventoRecibido, "finalizar") == 0)
    {
        if (strcmp(estadoRecibido, "finalizado exitoso") == 0)
        {
            int seFor = seFormoCuadrado(matrizEnviada, us);
            int seFor2 = seFormoCuadrado(matrizEnviada, contrario);
            printf("\n====================fin del juego====================\n");
            if (seFor == 0 && seFor2 == 0)
            {
                printf("Grupo Ganador: 7 y %s\n", destinoEnviar);
                printf("Jugador ganador: %s\n", "jugador 1 y jugador 2");
                printf("Modalidad ganadora: %s\n", "Visita y Local");
            }
            else
            {
                printf("Grupo Ganador: %s\n", destinoEnviar);
                if (strcmp(programa, "servidor;") == 0)
                {
                    printf("Jugador ganador: %s\n", "jugador 2");
                    printf("Modalidad ganadora: %s\n", "Visita");
                }
                else
                {
                    printf("Jugador ganador: %s\n", "jugador 1");
                    printf("Modalidad ganadora: %s\n", "Local");
                }
            }
            if (modoLocal)
            {
                printf("Total-jugadas-jugador1: %d\n", jugadasMio);
                printf("Total-jugadas-jugador2: %d\n", jugadasContrario);
                printf("Tiempo-promedio-jugadas-jugador1: %f\n", duracionMio / jugadasMio);
                printf("Tiempo-promedio-jugadas-jugador2: %f\n", duracionContrario / jugadasContrario);
            }
            else
            {
                printf("Total-jugadas-jugador1: %d\n", jugadasContrario);
                printf("Total-jugadas-jugador2: %d\n", jugadasMio);
                printf("Tiempo-promedio-jugadas-jugador1:%f\n", duracionContrario / jugadasContrario);
                printf("Tiempo-promedio-jugadas-jugador2: %f\n", duracionMio / jugadasMio);
            }
            printf("Inicio juego: %s\n", inicioJuego);
            printf("Fin de juego: %s\n", tiempRecibidoStr);
            printf("Duracion total: %d\n", duracionMio + duracionContrario);
            if (seFor != 0 || seFor2 != 0)
            {
                printf("Coordenadas del cuadrado: (%d,%d);(%d,%d);(%d,%d);(%d,%d)\n",
                       coordCuadrado[0] + 1, coordCuadrado[1] + 1, coordCuadrado[2] + 1, coordCuadrado[3] + 1, coordCuadrado[4] + 1,
                       coordCuadrado[5] + 1, coordCuadrado[6] + 1, coordCuadrado[7] + 1);
            }
            printf("\n");

            // agregamos al archivo el mensaje final
            fprintf(registro, "====================fin del juego====================\n");
            char *grupo = strtok(destinoEnviar, ";");
            if (seFor == 0 && seFor2 == 0)
            {
                fprintf(registro, "Grupo-ganador: 7 y %s\n", grupo);
                fprintf(registro, "Jugador-ganador: Jugador 1 y Jugado 2\n");
                fprintf(registro, "Modalidad-partida-ganada: Visita y Local\n");
            }
            else
            {
                fprintf(registro, "Grupo-ganador: %s\n", grupo);
                if (strcmp(programa, "servidor;") == 0)
                {
                    fprintf(registro, "Jugador-ganador: Jugador 2\n");
                    fprintf(registro, "Modalidad-partida-ganada: Visita\n");
                }
                else
                {
                    fprintf(registro, "Jugador-ganador: Jugador 1\n");
                    fprintf(registro, "Modalidad-partida-ganada: Local\n");
                }
            }
            if (modoLocal)
            {
                fprintf(registro, "Total-jugadas-jugador1: %d\n", jugadasMio);
                fprintf(registro, "Total-jugadas-jugador2: %d\n", jugadasContrario);
                fprintf(registro, "Tiempo-promedio-jugadas-jugador1: %f\n", duracionMio / jugadasMio);
                fprintf(registro, "Tiempo-promedio-jugadas-jugador2: %f\n", duracionContrario / jugadasContrario);
            }
            else
            {
                fprintf(registro, "Total-jugadas-jugador1: %d\n", jugadasContrario);
                fprintf(registro, "Total-jugadas-jugador2: %d\n", jugadasMio);
                fprintf(registro, "Tiempo-promedio-jugadas-jugador1: %f\n", duracionContrario / jugadasContrario);
                fprintf(registro, "Tiempo-promedio-jugadas-jugador2: %f\n", duracionMio / jugadasMio);
            }
            fprintf(registro, "Inicio del juego: %s\n", inicioJuego);
            fprintf(registro, "Finalización del juego: %s\n", tiempRecibidoStr);
            fprintf(registro, "Duracion-total-del-juego: %d\n", duracionMio + duracionContrario);
            if (seFor != 0 || seFor2 != 0)
            {
                fprintf(registro, "Coordenadas del cuadrado: (%d,%d);(%d,%d);(%d,%d);(%d,%d)\n",
                        coordCuadrado[0] + 1, coordCuadrado[1] + 1, coordCuadrado[2] + 1, coordCuadrado[3] + 1, coordCuadrado[4] + 1,
                        coordCuadrado[5] + 1, coordCuadrado[6] + 1, coordCuadrado[7] + 1);
            }
            //--------------------- */
        }
        else
        {
            printf("\n====================fin del juego====================\n");
            printf("Inicio juego: %s\n", inicioJuego);
            printf("Fin de juego: %s\n", tiempRecibidoStr);
            printf("Duracion total: %d\n", duracionMio + duracionContrario);
            if (tardoMucho)
            {
                printf("Motivo error: Tardo mas de 6 segundos en responder\n");
            }
            else if (mensajeValido == 0)
            {
                printf("Motivo error: Caracter invalido en mensaje\n");
                printf("Mensaje error: %s\n", msjInvalido);
            }
            else
            {
                printf("Motivo error: Se encontro trampa en la jugada\n");
            }

            // agregamos al archivo el mensaje final
            fprintf(registro, "====================fin del juego====================\n");
            fprintf(registro, "Inicio del juego: %s\n", inicioJuego);
            fprintf(registro, "Finalización del juego: %s\n", tiempRecibidoStr);
            fprintf(registro, "Duracion-total-del-juego: %d\n", duracionMio + duracionContrario);
            if (tardoMucho)
            {
                fprintf(registro, "Motivo error: Tardo mas de 6 segundos en responder\n");
            }
            else if (mensajeValido == 0)
            {
                fprintf(registro, "Motivo error: Caracter invalido en mensaje\n");
                fprintf(registro, "Mensaje error: %s\n", msjInvalido);
            }
            else
            {
                fprintf(registro, "Motivo error: Se encontro trampa en la jugada\n");
            }
        }

        running = 0;
    }
    else if (tardoMucho == 1 || mensajeValido == 0 || jugadaTrampa == 1)
    {
        // hora en que vamos a enviar el mensaje
        char hora[100];
        horaActual(hora);
        int tiempoEnviar = marcaServidor(hora);
        int duracionSegundos = tiempoEnviar - tiempoRecibido;
        itoa(duracionSegundos, duracion, 10);
        strcat(duracion, ";");
        strcat(hora, ";");
        // concatenamos todos los nuevos datos a respuesta
        strcpy(eventoEnviar, "finalizar;");
        strcpy(estadoEnviar, "finalizado fallido;");
        strcpy(turnoEnviar, "*;");
        strcpy(xEnviar, "*;");
        strcpy(yEnviar, "*;");
        strcpy(tableroEnviar, "*;");
        strcpy(jugadaEnviar, "*;");

        strcat(respuesta, nuevoId);
        strcat(respuesta, hora);
        strcat(respuesta, duracion);
        strcat(respuesta, programa);
        strcat(respuesta, origenEnviar);
        strcat(respuesta, destinoEnviar);
        strcat(respuesta, eventoEnviar);
        strcat(respuesta, estadoEnviar);
        strcat(respuesta, jugadaEnviar);
        strcat(respuesta, turnoEnviar);
        strcat(respuesta, xEnviar);
        strcat(respuesta, yEnviar);
        strcat(respuesta, tableroEnviar);
        strcat(respuesta, "#.");
        running = 0;
    }
}