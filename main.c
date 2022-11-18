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
    // printf("ingrese la direccion y el nombre para el registro.txt\n----> ");
    // scanf("%s", direccion);
    // registro = fopen(direccion, "w"); // guarda el archivo en la direccion y con el nombre que el usuario desea

    while (decisionMenu != 5)
    {
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
                    printf("Cambiando a modo local...\n");
                    modoLocal = 1;
                    modoVisita = 0;
                }
                else if (decisionParametros == 2) // modo visita
                {
                    printf("Cambiando a modo visita...\n");
                    modoVisita = 1;
                    modoLocal = 0;
                }
                else if (decisionParametros == 3) // directorio de archivos
                {
                    printf("Ingrese el directorio de archivos de salida: ");
                    scanf("%s", direccion);
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
            printf("\nLos autores del programa son: \n");
            printf("Joaquin Manuel Uliambre Frutos\n");
            printf("Zinri Alice Bobabilla Peralta\n");
        }
        else if (decisionMenu == 4) // ayuda
        {
        }
        else if (decisionMenu == 5) // salir
        {
            printf("Saliendo del programa...");
        }
        else
        {
            printf("\nEl valor ingresado es incorrecto, elija entre 1 al 5\n");
        }
    }

    // fclose(registro);
    return 0;
}