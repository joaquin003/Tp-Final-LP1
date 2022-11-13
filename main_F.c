#include "main.h"

int running = 0;

void socketServidor()
{
    printf("Hello, world!\n");

    int res, sendRes;

    // INITIALIZATION ===========================
    WSADATA wsaData; // configuration data
    res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res)
    {
        printf("Startup failed: %d\n", res);
    }
    // ==========================================

    // SETUP SERVER =============================

    // construct socket
    SOCKET listener;
    listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == INVALID_SOCKET)
    {
        printf("Error with construction: %d\n", WSAGetLastError());
        WSACleanup();
    }

    // setup for multiple connections
    char multiple = !0;
    res = setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &multiple, sizeof(multiple));
    if (res < 0)
    {
        printf("Multiple client setup failed: %d\n", WSAGetLastError());
        closesocket(listener);
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
    // ==========================================

    printf("Accepting on %s:%d\n", ADDRESS, PORT);

    // MAIN LOOP ================================

    // variables
    fd_set socketSet;            // set of active clients
    SOCKET clients[MAX_CLIENTS]; // array of clients
    int curNoClients = 0;        // active slots in the array
    SOCKET sd, max_sd;           // placeholders
    struct sockaddr_in clientAddr;
    int clientAddrlen;
    char running = !0; // server state

    char recvbuf[BUFLEN];

    char *welcome = "Welcome to the server :)\n";
    int welcomeLength = strlen(welcome);
    char *full = "Sorry, the server is full :(\n";
    int fullLength = strlen(full);
    char *goodbye = "Goodnight.\n";
    int goodbyeLength = strlen(goodbye);

    // clear client array
    memset(clients, 0, MAX_CLIENTS * sizeof(SOCKET));

    while (running)
    {
        // clear the set
        FD_ZERO(&socketSet);

        // add listener socket
        FD_SET(listener, &socketSet);

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            // socket
            sd = clients[i];

            if (sd > 0)
            {
                // add an active client to the set
                FD_SET(sd, &socketSet);
            }

            if (sd > max_sd)
            {
                max_sd = sd;
            }
        }

        int activity = select(max_sd + 1, &socketSet, NULL, NULL, NULL);
        if (activity < 0)
        {
            continue;
        }

        // determine if listener has activity
        if (FD_ISSET(listener, &socketSet))
        {
            // accept connection
            sd = accept(listener, NULL, NULL);
            if (sd == INVALID_SOCKET)
            {
                printf("Error accepting: %d\n", WSAGetLastError());
            }

            // get client information
            getpeername(sd, (struct sockaddr *)&clientAddr, &clientAddrlen);
            printf("Client connected at %s:%d\n",
                   inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

            // add to array
            if (curNoClients >= MAX_CLIENTS)
            {
                printf("Full\n");

                // send overflow message
                sendRes = send(sd, full, fullLength, 0);
                if (sendRes != fullLength)
                {
                    printf("Error sending: %d\n", WSAGetLastError());
                }

                shutdown(sd, SD_BOTH);
                closesocket(sd);
            }
            else
            {
                // scan through list
                int i;
                for (i = 0; i < MAX_CLIENTS; i++)
                {
                    if (!clients[i])
                    {
                        clients[i] = sd;
                        printf("Added to the list at index %d\n", i);
                        curNoClients++;
                        break;
                    }
                }

                // send welcome
                sendRes = send(sd, welcome, welcomeLength, 0);
                if (sendRes != welcomeLength)
                {
                    printf("Error sending: %d\n", WSAGetLastError());
                    shutdown(sd, SD_BOTH);
                    closesocket(sd);
                    clients[i] = 0;
                    curNoClients--;
                }
            }
        }

        // iterate through clients
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (!clients[i])
            {
                continue;
            }

            sd = clients[i];
            // determine if client has activity
            if (FD_ISSET(sd, &socketSet))
            {
                // get message
                res = recv(sd, recvbuf, BUFLEN, 0);
                if (res > 0)
                {
                    // print message
                    recvbuf[res] = '\0';
                    printf("Mensaje recibido (%d): %s\n", res, recvbuf);

                    // test if quit command
                    if (!memcmp(recvbuf, "/quit", 5 * sizeof(char)))
                    {
                        running = 0; // false
                        break;
                    }

                    // echo message
                    /*sendRes = send(sd, recvbuf, res, 0);
                    if (sendRes == SOCKET_ERROR)
                    {
                        printf("Echo failed: %d\n", WSAGetLastError());
                        shutdown(sd, SD_BOTH);
                        closesocket(sd);
                        clients[i] = 0;
                        curNoClients--;
                    }*/
                }
                else
                {
                    // close message
                    getpeername(sd, (struct sockaddr *)&clientAddr, &clientAddrlen);
                    printf("Client disconnected at %s:%d\n",
                           inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

                    shutdown(sd, SD_BOTH);
                    closesocket(sd);
                    clients[i] = 0;
                    curNoClients--;
                }
            }
        }
    }

    // ==========================================

    // CLEANUP ==================================

    // disconnect all clients
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] > 0)
        {
            // active client
            sendRes = send(clients[i], goodbye, goodbyeLength, 0);

            shutdown(clients[i], SD_BOTH);
            closesocket(clients[i]);
            clients[i] = 0;
        }
    }

    // shut down server socket
    closesocket(listener);

    // cleanup WSA
    res = WSACleanup();
    if (res)
    {
        printf("Cleanup failed: %d\n", res);
    }
    // ==========================================

    printf("Shutting down.\nGood night.\n");
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

void socketCliente()
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

void leer_mensaje(FILE *registro, char mensaje[]){
    char delimitador[] = "[];#";
    char *token = strtok(mensaje, delimitador);
    int j=0;
    char *aux;
    char salto='\n';
    if(token != NULL){
        while(token != NULL){
            ////// 
            /*
            Separamos el mensaje de acuerdo al valor de j
            */
            if (j==0)
            {
                printf("Token: %s\n", token);
                fprintf(registro,"#####Mensaje-id:%s\n",token);
            }
            else if (j==1)
            {
                printf("Token: %s\n", token);
                fprintf(registro,"Mensaje-origen:%s. ",token);
            }else if (j==2)
            {
                printf("Token: %s\n", token);
                fprintf(registro,"Mensaje-destino:%s.\n",token);
            }else if (j==3)
            {
                printf("Token: %s\n", token);
                fprintf(registro,"Evento:%s. ",token);
            }else if (j==4)
            {
                printf("Token: %s\n", token);
                fprintf(registro,"Estado-juego:%s.\n",token);
            }else if (j==5)
            {
                printf("Token: %s\n", token);
                fprintf(registro,"**********Turno-jugada:Jugador-%s. ",token);
            }else if (j==6)
            {
                printf("Token: %s\n", token);
                fprintf(registro,"Numero-jugada:%s.**********\n",token);
            
            }else if (j==7) //posicion en x
            {
                aux=token;
            }else if (j==8) //posicion en y
            {
                printf("Token: %s\n", token);
                fprintf(registro,"Casilla-jugada:(%s,%s).\n",aux,token);

            }else if (j==9)     // TABLERO!!!!!!!!
            {
                int cont=0, //contador que hace los saltos de linea
                    k=0;    //contador de caracteres dentro del tablero
                printf("Token: %s\n", token);
                /// guardamos el tablero en el archivo con el formato deseado
                while (k<strlen(token)) //mientas no leamos todo el tablero
                {
                    if (cont==10)
                    {
                        fputc(salto,registro);
                        cont=0;
                    }
                    if (cont==9)
                    {                     
                        fputc(token[k],registro);
                    }
                    else
                    {
                        fputc(token[k],registro);
                        fputc(token[k+1],registro);
                    }
                    k=k+2;
                    cont++;                
                }  
                
            }
            // Sólo en la primera pasamos la cadena; en las siguientes pasamos NULL
            token = strtok(NULL, delimitador);
            j++;
        }
    }
}
