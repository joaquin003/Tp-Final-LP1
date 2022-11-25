#include "main.h"

int main()
{
    int decisionMenu = 0;
    int modoLocal = 0;
    int modoVisita = 0;
    char modoActivo[100];
    char direccion[1000];
    char numJugador[100];
    FILE *registro;
    SOCKET recibe;
    // printf("ingrese la direccion y el nombre para el registro.txt\n----> ");
    // scanf("%s", direccion);
    // registro = fopen(direccion, "w"); // guarda el archivo en la direccion y con el nombre que el usuario desea

    while (decisionMenu != 5)
    {
        registro = fopen(direccion, "w");
        if (modoLocal && !modoVisita)
        {
            strcpy(modoActivo, "Modo Local");
            strcpy(numJugador, "1");
        }
        else if (!modoLocal && modoVisita)
        {
            strcpy(modoActivo, "Modo Visita");
            strcpy(numJugador, "2");
        }
        else
        {
            strcpy(modoActivo, "No configurado");
            strcpy(numJugador, "No configurado");
        }
        printf("\nID Grupo: 7            Jugador: %s\n", numJugador);
        printf("\nModo activado: %s\n\n", modoActivo);
        printf("MENU DEL PROGRAMA\n\n");
        printf("Opciones de navegacion:\n");
        printf("1- Empezar partida\n");
        printf("2- Configurar parametros\n");
        printf("3- Autores\n");
        printf("4- Ayuda\n");
        printf("5- Salir\nElegir una opcion (1-5): ");
        scanf("%d", &decisionMenu);

        if (decisionMenu == 1) // empezar partida
        {
            if (strcmp(modoActivo, "Modo Local") == 0 && registro != NULL)
            {
                socketServidor();
            }
            else if (strcmp(modoActivo, "Modo Visita") == 0 && registro != NULL)
            {
                socketCliente();
            }
            else
            {
                printf("\nAsegurese de  configurar el modo y la ruta del archivo para comenzar la partida\n");
            }
        }
        else if (decisionMenu == 2) // configurar parametros
        {
            int decisionParametros = 0;

            while (decisionParametros != 4)
            {
                printf("\nCONFIGURAR PARAMETROS\n\n");
                printf("Opciones de navegacion:\n");
                printf("1- Modo Local\n");
                printf("2- Modo Visita\n");
                printf("3- Directorio de archivos\n");
                printf("4- Ir atras\nElegir una opcion(1-4): ");
                scanf("%d", &decisionParametros);

                if (decisionParametros == 1) // modo local
                {
                    printf("\nCambiando a modo local...\n\n\n");
                    modoLocal = 1;
                    modoVisita = 0;
                }
                else if (decisionParametros == 2) // modo visita
                {
                    printf("\nCambiando a modo visita...\n\n\n");
                    modoVisita = 1;
                    modoLocal = 0;
                }
                else if (decisionParametros == 3) // directorio de archivos
                {
                    printf("Para ingresar la ruta, el nombre del directorio no debe contener espacios\n");
                    printf("\nIngrese el directorio de archivos de salida: ");
                    scanf("%s", direccion);
                    printf("\nLa direccion ingresada es: %s\n", direccion);
                    registro = fopen(direccion, "w");
                    /* validacion de la direccion del archivo*/
                    while (registro == NULL)
                    {
                        printf("\nError!!!! Por favor ingrese una carpeta y archivo de texto existente\n ");
                        printf("ingrese la direccion y el nombre para el registro.txt\n----> ");
                        scanf("%s", direccion);
                        registro = fopen(direccion, "w");
                    }
                }
                else if (decisionParametros == 4) // ir atras
                {
                    printf("Volviendo a menu...\n");
                }
                else
                {
                    printf("\nEl valor ingresado es incorrecto, elija entre 1 al 4\n");
                }
            }
        }
        else if (decisionMenu == 3) // autores
        {
            system("cls");
            printf("\nLos autores del programa son: \n");
            printf("Joaquin Manuel Uliambre Frutos\n");
            printf("Zinri Alice Bobabilla Peralta\n\n");
            system("pause");
        }
        else if (decisionMenu == 4) // ayuda
        {
        }
        else if (decisionMenu == 5) // salir
        {
            printf("\n\n\nSaliendo del programa...");
        }
        else
        {
            printf("\nEl valor ingresado es incorrecto, elija entre 1 al 5\n");
        }
    }

    fclose(registro);
    return 0;
}