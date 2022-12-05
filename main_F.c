#include "main.h"

int running = 0;

void socketServidor(FILE *registro, int modoLocal)
{
    int res, sendRes;

    // INITIALIZATION ==============================
    WSADATA wsaData; // configuration data
    res = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (res)
    {
        printf("Startup failed: %d\n", res);
    }
    //==============================================

    // SETUP SERVER =================================

    // constructor socket
    SOCKET listener;
    listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listener == INVALID_SOCKET)
    {
        printf("Error with construction: %d\n", WSAGetLastError());
        WSACleanup();
    }

    // bind to address
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ADDRESS);
    address.sin_port = htons(PORT);
    res = bind(listener, (struct sockaddr *)&address, sizeof(address));

    if (res == SOCKET_ERROR)
    {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(listener);
        WSACleanup();
    }

    // set as a listener
    res = listen(listener, SOMAXCONN);
    if (res == SOCKET_ERROR)
    {
        printf("Listen failed: %d\n", WSAGetLastError());
        closesocket(listener);
        WSACleanup();
    }
    //==============================================

    printf("Accepting on %s:%d\n", ADDRESS, PORT);

    // HANDLE A CLIENT =============================

    // accept client socket
    SOCKET client;
    struct sockaddr_in clientAddr;
    int clientAddrlen;
    client = accept(listener, NULL, NULL);
    if (client == INVALID_SOCKET)
    {
        printf("Could not accept: %d\n", WSAGetLastError());
        closesocket(listener);
        WSACleanup();
    }

    // get client information
    getpeername(client, (struct sockaddr *)&clientAddr, (socklen_t *)&clientAddr);
    printf("Client connected at %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

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
        }
        else if (!res)
        {
            // client disconnected
            printf("Closing connection.\n");
            break;
        }
        else
        {
            printf("Receive failed: %d\n", WSAGetLastError());
            break;
        }
    } while (res > 0);

    // shutdown client
    res = shutdown(client, SD_BOTH);
    if (res == SOCKET_ERROR)
    {
        printf("Client shutdown failed: %d\n", WSAGetLastError());
    }
    closesocket(client);
    // =============================================

    // CLEANUP =====================================

    // shut down server socket
    closesocket(listener);

    // cleanup WSA
    res = WSACleanup();
    if (res)
    {
        printf("Cleanup failed: %d\n", res);
    }
    //==============================================

    printf("Shutting down. \nGood night.\n");
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

    printf("Mensaje ENVIADO: %s\n", mensaje);

    char delimitador[] = "[];#";
    char *token = strtok(mensaje, delimitador);
    int j = 0;
    char *aux;
    char salto = '\n';
    char *jugadas;
    char programa[20];
    if (modoLocal)
    {
        strcpy(programa, "servidor;");
    }
    else
    {
        strcpy(programa, "cliente;");
    }

    fprintf(registro, "######Programa:%s\n", programa);

    if (token != NULL)
    {
        while (token != NULL)
        {
            //////
            /*
            Separamos el mensaje de acuerdo al valor de j
            */
            if (j == 0) // id mensaje
            {
                printf("Token: %s\n", token);
                // agregamos el valor al archivo
                fprintf(registro, "Mensaje-id:%s", token);
            }
            else if (j == 1) // marca temporal
            {
                printf("Token: %s\n", token);
                fprintf(registro, "     Marca-de-tiempo:%s. ", token);
            }
            else if (j == 2) // duracion
            {
                printf("Token: %s\n", token);
                fprintf(registro, "      Duracion:%s");
            }
            else if (j == 3) // programa
            {
                printf("Token: %s\n", token);
            }
            else if (j == 4) // origen
            {
                printf("Token origen: %s\n", token);
                fprintf(registro, "\nMensaje-origen:%s. ", token);
            }
            else if (j == 5) // destino
            {
                printf("Token destino: %s\n", token);
                fprintf(registro, "Mensaje-destino:%s.\n", token);
            }
            else if (j == 6) // evento
            {
                printf("Token: %s\n", token);
                fprintf(registro, "Evento:%s. ", token);
            }
            else if (j == 7) // estado
            {
                printf("Token: %s\n", token);
                fprintf(registro, "Estado-juego:%s.\n", token);
            }
            else if (j == 8) // jugada
            {
                printf("Token jugada: %s\n", token);
                jugadas = token;
                // fprintf(registro, "Numero-jugada:%s.**********\n", token);
            }
            else if (j == 9) // turno
            {
                printf("Token: %s\n", token);
                // fprintf(registro, "**********Turno-jugada:Jugador-%s. ", token);
                fprintf(registro, "------------Turno-jugador:Jugador-%s. ", token);
                fprintf(registro, "Numero-jugada:%s.------------\n", jugadas);
            }
            else if (j == 10) // posicion en x
            {
                aux = token;
            }
            else if (j == 11) // posicion en y
            {
                printf("Token: %s\n", token);
                fprintf(registro, "Casilla-jugada:(%s,%s).\n", aux, token);
            }
            else if (j == 12) // TABLERO!!!!!!!!
            {
                fprintf(registro, "Tablero-actual:\n");
                int cont = 0, // contador que hace los saltos de linea
                    k = 0;    // contador de caracteres dentro del tablero
                printf("Token: %s\n", token);
                /// guardamos el tablero en el archivo con el formato deseado
                while (k < strlen(token)) // mientas no leamos todo el tablero
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
}

void socketCliente(FILE *registro, int modoLocal)
{
    int res;

    // INITIALIZATION ================================

    WSADATA wsaData; // configuration data
    res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res)
    {
        printf("Startup failed: %d\n", res);
    }

    // ===============================================

    // SETUP CLIENT SOCKET ===========================

    // constructor socket
    SOCKET client;
    client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == INVALID_SOCKET)
    {
        printf("Error with construction: %d\n", WSAGetLastError());
        WSACleanup();
    }

    // connect to address
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ADDRESS);
    address.sin_port = htons(PORT);
    res = connect(client, (struct sockaddr *)&address, sizeof(address));
    if (res == SOCKET_ERROR || client == INVALID_SOCKET)
    {
        printf("Connect failed: %d\n", WSAGetLastError());
        closesocket(client);
        WSACleanup();
    }
    else
    {
        // se envia el primer mensaje cuando se logro conectarse
        char mensaje[1000] = "";
        strcat(mensaje, "1;");
        // hora en que vamos a enviar el mensaje
        time_t t;
        struct tm *tm;
        char hora[100];

        t = time(NULL);
        tm = localtime(&t);
        strftime(hora, 100, "%H:%M:%S", tm);

        strcat(hora, ";");
        strcat(mensaje, hora);
        strcat(mensaje, "*;cliente;7;*;conectar;pendiente;*;*;*;*;*;#.");
        enviarMensaje(&client, mensaje, registro, modoLocal);
    }

    // set as running
    running = !0; // true

    // ===============================================

    // MAIN LOOP =====================================

    // receive loop
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
            printf("Connection closed.\n");
            running = 0;
        }
        else
        {
            printf("Receive failed: %d.\n", WSAGetLastError());
            running = 0;
        }

    } while (running && res > 0);
    running = 0;

    // ===============================================

    // CLEANUP =======================================

    res = shutdown(client, SD_BOTH);
    if (res == SOCKET_ERROR)
    {
        printf("Shutdown failed: %d\n", WSAGetLastError());
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
            seFormaCuadrado = 1;
        }
        index++;
    }

    return seFormaCuadrado;
}

// para los cuadradros inclinadas
int conOrientacion(int A[10][10], int x, int y, int buscar)
{
    int seFormaCuadrado = 0;
    int indexF = 0;
    /*
---------------------- */

    int x_buscada = 0, y_buscada = 0;
    while (indexF + x < 10)
    {
        int indexC = 0;
        while (indexC + y < 10)
        {
            // printf("\nbuscando....%d (%d,%d)",A[x+indexF][abs(y-indexC)],x+indexF,abs(y-indexC));
            if (A[x + indexF][abs(y - indexC)] == buscar)
            {
                float dist = sqrt(pow((x) - (x + indexF), 2) + pow((y)-abs(y - indexC), 2));
                float disCuadrado = pow(dist, 2);
                // en busqueda de la 3ra posicion q cumpla
                /*FORMULA----------> dis**2 = (x_buscada - x)**2 + (y_buscada - y)**2 */
                while (x_buscada < 10)
                {
                    y_buscada = 0;
                    while (y_buscada < 10)
                    {
                        if (disCuadrado == pow(x_buscada - x, 2) + pow(y_buscada - y, 2))
                        {
                            break;
                        }
                        y_buscada++;
                    }

                    x_buscada++;
                }
            }

            if (A[x + indexF][y + indexC] == buscar && A[x_buscada][y_buscada] == buscar && A[x][y] == buscar)
            {
                seFormaCuadrado = 1;
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

// comprobar si el tablero coincide con la posicion que agrego y viceversa
int validarJugada(int original[10][10], int nuevo[10][10], int x, int y, int jugador)
{
    original[x][y] = jugador;
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            if (original[i][j] != nuevo[i][j])
            {
                return 0;
            }
        }
    }
    return 1;
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
                    printf("\n EN SE FORMA\n");
                    printf("\n POSICION DONDE FORMA  ----> (%d,%d)\n", i, j);
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

void leer_mensaje(FILE *registro, char mensaje[], char *respuesta, int modoLocal)
{
    // limpiamos la respuesta antes de concatenarle nuevos valores
    memset(respuesta, '\0', strlen(respuesta));

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

    strcpy(origenEnviar, "7;");
    if (modoLocal)
    {
        strcpy(programa, "cliente;");
        us = 1;
        contrario = 2;
    }
    else
    {
        strcpy(programa, "servidor;");
        us = 2;
        contrario = 1;
    }

    fprintf(registro, "######Programa:%s\n", programa);

    if (token != NULL)
    {
        while (token != NULL)
        {
            //////
            /*
            Separamos el mensaje de acuerdo al valor de j
            */
            if (j == 0) // id mensaje
            {
                printf("Token: %s\n", token);
                // agregamos el valor al archivo
                fprintf(registro, "Mensaje-id:%s", token);

                // cambiamos el valor para concatenar al mensaje de respuesta
                int x = atoi(token);
                itoa(x + 1, nuevoId, 10); // convertimos el nuevo id a string
                strcat(nuevoId, ";");     // agregamos ; al atributoId
            }
            else if (j == 1) // marca temporal
            {
                printf("Token: %s\n", token);
                tiempoRecibido = marcaServidor(token);
                fprintf(registro, "     Marca-de-tiempo:%s. ", token);
            }
            else if (j == 2) // duracion
            {
                printf("Token: %s\n", token);
                fprintf(registro, "      Duracion:%s");
            }
            else if (j == 3) // programa
            {
                printf("Token: %s\n", token);
            }
            else if (j == 4) // origen
            {
                printf("Token origen: %s\n", token);
                fprintf(registro, "\nMensaje-origen:%s. ", token);

                strcpy(destinoEnviar, token);
                strcat(destinoEnviar, ";");
            }
            else if (j == 5) // destino
            {
                printf("Token destino: %s\n", token);
                fprintf(registro, "Mensaje-destino:%s.\n", token);
            }
            else if (j == 6) // evento
            {
                printf("Token: %s\n", token);
                fprintf(registro, "Evento:%s. ", token);

                strcpy(eventoRecibido, token);
            }
            else if (j == 7) // estado
            {
                printf("Token: %s\n", token);
                fprintf(registro, "Estado-juego:%s.\n", token);

                strcpy(estadoRecibido, token);
            }
            else if (j == 8) // jugada
            {
                printf("Token jugada: %s\n", token);
                jugadas = token;

                if (strcmp(token, "*") != 0)
                {
                    // cambiamos el valor para concatenar al mensaje de respuesta
                    int x = atoi(token);
                    itoa(x + 1, jugadaEnviar, 10); // convertimos el nuevo id a string
                    strcat(jugadaEnviar, ";");     // agregamos ; al atributoId
                }
            }
            else if (j == 9) // turno
            {
                printf("Token: %s\n", token);
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
                printf("Token: %s\n", token);
                strcpy(yRecibido, token);
                fprintf(registro, "Casilla-jugada:(%s,%s).\n", aux, token);
            }
            else if (j == 12) // TABLERO!!!!!!!!
            {
                fprintf(registro, "Tablero-actual:\n");
                strcpy(tableroRecibido, token);
                int cont = 0, // contador que hace los saltos de linea
                    k = 0;    // contador de caracteres dentro del tablero
                printf("Token: %s\n", token);
                /// guardamos el tablero en el archivo con el formato deseado
                while (k < strlen(token)) // mientas no leamos todo el tablero
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

    if (strcmp(eventoRecibido, "conectar") == 0)
    {
        strcpy(eventoEnviar, "iniciar;");
        strcpy(estadoEnviar, "conectado;");
        strcpy(jugadaEnviar, "*;");
        strcpy(turnoEnviar, "*;");
        strcpy(xEnviar, "*;");
        strcpy(yEnviar, "*;");
        strcpy(tableroEnviar, "*;");

        // hora en que vamos a enviar el mensaje
        time_t t;
        struct tm *tm;
        char hora[100];

        t = time(NULL);
        tm = localtime(&t);
        strftime(hora, 100, "%H:%M:%S", tm);

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
    else if (strcmp(eventoRecibido, "iniciar") == 0)
    {
        strcpy(eventoEnviar, "empezar;");
        strcpy(estadoEnviar, "activo;");
        strcpy(jugadaEnviar, "*;");
        strcpy(turnoEnviar, "*;");
        strcpy(xEnviar, "*;");
        strcpy(yEnviar, "*;");
        strcpy(tableroEnviar, "*;");

        // hora en que vamos a enviar el mensaje
        time_t t;
        struct tm *tm;
        char hora[100];

        t = time(NULL);
        tm = localtime(&t);
        strftime(hora, 100, "%H:%M:%S", tm);

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
        int ban = 0;

        while (ban != 1)
        {
            int i = rand() % 10, j = rand() % 10;

            int resultado = 0;
            int resultadoContrario = 0;

            // hay que validar que la posicion no tenga elemento
            if (matriz[i][j] == 0)
            {
                // verificarmos si se forma un cuadrado para nosotros
                resultado = validarPosicion(matriz, i, j, us);

                // verificamos si se forma un cuadrado para el contrario
                resultadoContrario = validarPosicion(matriz, i, j, contrario);

                // si no se forma para ninguno, colocamos nuestro valor
                if (!resultado && !resultadoContrario)
                {
                    matriz[i][j] = us;
                    // agregamos las posiciones a nuestras variables para luego enviar el mensaje
                    itoa(i, xEnviar, 10);
                    itoa(j, yEnviar, 10);
                    ban = 1;
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
        strcat(xEnviar, ";");
        strcat(yEnviar, ";");
        strcat(tableroEnviar, ";");

        // hora en que vamos a enviar el mensaje
        time_t t;
        struct tm *tm;
        char hora[100];

        t = time(NULL);
        tm = localtime(&t);
        strftime(hora, 100, "%H:%M:%S", tm);

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

    else if (strcmp(estadoRecibido, "activo") == 0 && strcmp(eventoRecibido, "jugar") == 0)
    {

        int fila = 0, columna = 0, p = 0, contador = 1;
        char auxi[1];
        int matriz[10][10];

        while (p < strlen(tableroRecibido))
        {

            if (contador % 10 == 0) // para hacer el cambio de fila
            {
                auxi[0] = tableroRecibido[p];

                matriz[fila][columna] = atoi(auxi);

                fila++;
                columna = 0;
            }
            else
            {
                auxi[0] = tableroRecibido[p];

                matriz[fila][columna] = atoi(auxi);
                columna++;
            }
            p = p + 2;
            contador++;
        }
        printf("soy matriz\n");
        // imprimir datos de matriz
        for (int fila = 0; fila < 10; fila++)
        {
            for (int columna = 0; columna < 10; columna++)
            {
                printf("%d ", matriz[fila][columna]);
            }
            printf("\n");
        }

        /*
        if (validarJugada(matriz,tableroRecibido,xRecibido,yRecibido,contrario)!=0);
        {
            int xNuevo= atoi(xRecibido);
            int yNuevo= atoi(yRecibido);
            matriz[xNuevo][yNuevo]=contrario;
        }
        */

        int seFormo_contrario = seFormoCuadrado(matriz, contrario);
        int seFormo_nosotros = seFormoCuadrado(matriz, us);
        int t_lleno = 1, row = 0;
        while (row < 10 || t_lleno != 0)
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

        if (!seFormo_contrario && !seFormo_nosotros && t_lleno == 0)
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
                // int resultadoContrario = 0;

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
                        itoa(i, xEnviar, 10);
                        itoa(j, yEnviar, 10);
                        ban = 1;
                        printf("\ncoloqueee\n");
                        validador = 1;
                    }
                    if (contar == v)
                    {
                        ban = 1;
                    }
                    contar++;
                }
            }
            printf("\ncontador de vacias %d\n vacias: %d\n", contar, v);
            // si sale igual, significa que ya no hay otras posiciones donde no se puede hacer cuadrado
            if (contar >= v && validador == 0)
            {
                printf("\nENTRO EN EL VACIOO\n");
                int coloco = 0;
                for (int i = 0; i < 10 - 1; i++)
                {
                    for (int j = 0; j < 10 - 1; j++)
                    {
                        if (matriz[i][j] == 0 && coloco != 1)
                        {
                            matriz[i][j] = us;
                            coloco = 1;
                            itoa(i, xEnviar, 10);
                            itoa(j, yEnviar, 10);
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
            // printf("\n TABLERO-ENVIAR%s\n",tableroEnviar);
            strcpy(eventoEnviar, "jugar;");
            strcpy(estadoEnviar, "activo;");
            itoa(us, turnoEnviar, 10);
            strcat(turnoEnviar, ";");

            // hora en que vamos a enviar el mensaje
            time_t t;
            struct tm *tm;
            char hora[100];

            t = time(NULL);
            tm = localtime(&t);
            strftime(hora, 100, "%H:%M:%S", tm);

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
        else if (!seFormo_nosotros)
        {
            printf("\nperdio contrario\n");

            // hora en que vamos a enviar el mensaje
            time_t t;
            struct tm *tm;
            char hora[100];

            t = time(NULL);
            tm = localtime(&t);
            strftime(hora, 100, "%H:%M:%S", tm);

            int tiempoEnviar = marcaServidor(hora);
            int duracionSegundos = tiempoEnviar - tiempoRecibido;

            itoa(duracionSegundos, duracion, 10);
            strcat(duracion, ";");
            strcat(hora, ";");
            // concatenamos todos los nuevos datos a respuesta
            strcpy(eventoEnviar, "finalizar.;");
            strcpy(estadoEnviar, "finalizado exitoso.;");
            strcpy(turnoEnviar, "*;");
            strcpy(xEnviar, "*;");
            strcpy(yEnviar, "*;");
            strcpy(tableroEnviar, "*;");

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
        else if (seFormo_nosotros)
        {
            printf("\ngano contrario\n");

            // hora en que vamos a enviar el mensaje
            time_t t;
            struct tm *tm;
            char hora[100];

            t = time(NULL);
            tm = localtime(&t);
            strftime(hora, 100, "%H:%M:%S", tm);

            int tiempoEnviar = marcaServidor(hora);
            int duracionSegundos = tiempoEnviar - tiempoRecibido;

            itoa(duracionSegundos, duracion, 10);
            strcat(duracion, ";");
            strcat(hora, ";");
            // concatenamos todos los nuevos datos a respuesta
            strcpy(eventoEnviar, "finalizar.;");
            strcpy(estadoEnviar, "finalizado exitoso.;");
            strcpy(turnoEnviar, "*;");
            strcpy(xEnviar, "*;");
            strcpy(yEnviar, "*;");
            strcpy(tableroEnviar, "*;");

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
            printf("tablero lleno");
            running = 0;
        }
    }
    else if (strcmp(eventoRecibido, "finalizar.") == 0)
    {
        printf("termino");
        running = 0;
    }
}