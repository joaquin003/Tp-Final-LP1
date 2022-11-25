#include "main.h"

int running = 0;

void socketServidor(FILE *registro)
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
    char *welcome = "Welcome to the server :)";
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
            leer_mensaje(registro, recvbuf);
            sendRes = send(client, recvbuf, res, 0);
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

void socketCliente(FILE *registro)
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
            enviarMensaje(&client, "1;15:00:05;*;cliente;2;*;conectar;pendiente;*;*;*;*;*;#.");
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

void leer_mensaje(FILE *registro, char mensaje[])
{
    char delimitador[] = "[];#";
    char *token = strtok(mensaje, delimitador);
    int j = 0;
    char *aux;
    char salto = '\n';
    if (token != NULL)
    {
        while (token != NULL)
        {
            //////
            /*
            Separamos el mensaje de acuerdo al valor de j
            */
            if (j == 0)
            {
                printf("Token: %s\n", token);
                fprintf(registro, "#####Mensaje-id:%s\n", token);
            }
            else if (j == 1)
            {
                printf("Token: %s\n", token);
                fprintf(registro, "Mensaje-origen:%s. ", token);
            }
            else if (j == 2)
            {
                printf("Token: %s\n", token);
                fprintf(registro, "Mensaje-destino:%s.\n", token);
            }
            else if (j == 3)
            {
                printf("Token: %s\n", token);
                fprintf(registro, "Evento:%s. ", token);
            }
            else if (j == 4)
            {
                printf("Token: %s\n", token);
                fprintf(registro, "Estado-juego:%s.\n", token);
            }
            else if (j == 5)
            {
                printf("Token: %s\n", token);
                fprintf(registro, "**********Turno-jugada:Jugador-%s. ", token);
            }
            else if (j == 6)
            {
                printf("Token: %s\n", token);
                fprintf(registro, "Numero-jugada:%s.**********\n", token);
            }
            else if (j == 7) // posicion en x
            {
                aux = token;
            }
            else if (j == 8) // posicion en y
            {
                printf("Token: %s\n", token);
                fprintf(registro, "Casilla-jugada:(%s,%s).\n", aux, token);
            }
            else if (j == 9) // TABLERO!!!!!!!!
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
}
