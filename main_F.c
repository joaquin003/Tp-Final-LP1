#include "main.h"

int running = 0;

void socketServidor(FILE *registro, int modoLocal)
{
    printf("Hello world\n");

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

    // send welcome message
    char *welcome = "Bienvenido al servidor!";
    sendRes = send(client, welcome, strlen(welcome), 0);
    if (sendRes != strlen(welcome))
    {
        printf("Error sending %d\n", WSAGetLastError());
        shutdown(client, SD_BOTH);
        closesocket(client);
    }

    // receive messages
    char recvbuf[BUFLEN];

    do
    {
        res = recv(client, recvbuf, BUFLEN, 0);
        if (res > 0)
        {
            recvbuf[res] = '\0';
            printf("Message received (%d): %s\n", res, recvbuf);

            if (!memcmp(recvbuf, "/quit", 5 * sizeof(char)))
            {
                // received quit command
                printf("Closing connection.\n");
                break;
            }

            // echo message back
            char respuesta[1000];
            leer_mensaje(registro, recvbuf, respuesta, modoLocal);
            sendRes = send(client, respuesta, res, 0);
            if (sendRes != res)
            {
                printf("Error sending %d\n", WSAGetLastError());
                shutdown(client, SD_BOTH);
                closesocket(client);
                break;
            }
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

DWORD WINAPI sendThreadFunc(LPVOID lpParam)
{
    SOCKET client = *(SOCKET *)lpParam;

    char sendbuf[BUFLEN];
    int sendbuflen, res;

    while (running)
    {
        scanf("%s", sendbuf);

        if (!running)
        {
            break;
        }

        sendbuflen = strlen(sendbuf);
        res = send(client, sendbuf, sendbuflen, 0);

        if (res != sendbuflen)
        {
            printf("Send failed.\n");
            break;
        }
        else if (!memcmp(sendbuf, "/leave", 6))
        {
            running = 0;
            break;
        }
    }
}

DWORD WINAPI enviarMensaje(LPVOID lpParam, char mensaje[BUFLEN])
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
}

void socketCliente(FILE *registro, int modoLocal)
{
    printf("Hello World\n");

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

    // set as running
    printf("Successfully connected to %s: %d\n", ADDRESS, PORT);
    running = !0; // true

    // ===============================================

    // MAIN LOOP =====================================

    // start send thread
    DWORD thrdId;
    HANDLE sendThread = CreateThread(NULL, 0, sendThreadFunc, &client, 0, &thrdId);

    if (sendThread)
    {
        printf("Send thread started with thread ID: %d\n", thrdId);
    }
    else
    {
        printf("Send thread failed: %d\n", GetLastError());
    }

    // receive loop
    char recvbuf[BUFLEN];

    do
    {
        res = recv(client, recvbuf, BUFLEN, 0);
        recvbuf[res] = '\0';

        if (res > 0)
        {
            printf("Mensaje recibido (%d): %s\n", res, recvbuf);
            if (strcmp(recvbuf, "Bienvenido al servidor!") == 0)
            {
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
                strcat(mensaje, "*;cliente;2;*;conectar;pendiente;*;*;*;*;*;#.");
                enviarMensaje(&client, mensaje);
            }
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

    // connection finished, terminate send thread
    if (CloseHandle(sendThread))
    {
        printf("Send thread closed successfully\n");
    }

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

void leer_mensaje(FILE *registro, char mensaje[], char *respuesta, int modoLocal)
{
    // limpiamos la respuesta antes de concatenarle nuevos valores
    memset(respuesta, '\0', strlen(respuesta));

    char delimitador[] = "[];#";
    char *token = strtok(mensaje, delimitador);
    int j = 0;
    char *aux;
    char salto = '\n';

    // datos del mensaje
    int tiempoRecibido = 0;

    // datos para el nuevo mensaje
    char nuevoId[20];
    char duracion[20];
    char programa[20];

    if (modoLocal)
    {
        strcpy(programa, "servidor;");
    }
    else
    {
        strcpy(programa, "cliente;");
    }

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
                fprintf(registro, "#####Mensaje-id:%s\n", token);

                // cambiamos el valor para concatenar al mensaje de respuesta
                int x = atoi(token);
                itoa(x + 1, nuevoId, 10); // convertimos el nuevo id a string
                strcat(nuevoId, ";");     // agregamos ; al atributoId
            }
            else if (j == 1) // marca temporal
            {
                printf("Token: %s\n", token);
                tiempoRecibido = marcaServidor(token);
            }
            else if (j == 2) // duracion
            {
                printf("Token: %s\n", token);
            }
            else if (j == 3) // programa
            {
                printf("Token: %s\n", token);
            }
            else if (j == 4) // origen
            {
                printf("Token: %s\n", token);
                fprintf(registro, "Mensaje-origen:%s. ", token);
            }
            else if (j == 5) // destino
            {
                printf("Token: %s\n", token);
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
                printf("Token: %s\n", token);
                fprintf(registro, "Numero-jugada:%s.**********\n", token);
            }
            else if (j == 9) // turno
            {
                printf("Token: %s\n", token);
                fprintf(registro, "**********Turno-jugada:Jugador-%s. ", token);
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
}
