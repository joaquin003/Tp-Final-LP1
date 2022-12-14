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
    registro = fopen(direccion, "w");
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
        printf("\nModo activo: %s\n\n", modoActivo);
        printf("\t\tMENU DEL PROGRAMA\n\n");
        printf("Opciones de navegacion:\n");
        printf("   1- Empezar partida\n");
        printf("   2- Configurar parametros\n");
        printf("   3- Autores\n");
        printf("   4- Ayuda\n");
        printf("   5- Salir\n\nElegir una opcion (1-5)>: ");
        scanf("%d", &decisionMenu);

        if (decisionMenu == 1) // empezar partida
        {
            if (strcmp(modoActivo, "Modo Local") == 0 && registro != NULL)
            {
                socketServidor(registro, modoLocal);
            }
            else if (strcmp(modoActivo, "Modo Visita") == 0 && registro != NULL)
            {
                socketCliente(registro, modoLocal);
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
                printf("\n\t\tCONFIGURAR PARAMETROS\n\n");
                printf("Opciones de navegacion:\n");
                printf("   1- Modo Local\n");
                printf("   2- Modo Visita\n");
                printf("   3- Directorio de archivos\n");
                printf("   4- Ir atras\n\nElegir una opcion(1-4)>: ");
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
                    if (modoLocal == 0 && modoVisita == 0)
                    {
                        printf("\nSeleccione el modo antes de ingresar la ruta\n");
                    }
                    else
                    {
                        printf("la ruta no debe tener espacios\n");
                        printf("\nCual es directorio de archivos de salida?>: ");
                        scanf("%s", direccion);
                        if (modoLocal == 1)
                        {
                            strcat(direccion, "std_servidor_grupo_7.txt");
                        }
                        else if (modoVisita == 1)
                        {
                            strcat(direccion, "std_cliente_grupo_7.txt");
                        }
                        registro = fopen(direccion, "w");
                        /* validacion de la direccion del archivo*/
                        while (registro == NULL)
                        {
                            printf("\nError!!!! Por favor ingrese una carpeta existente\n ");
                            printf("ingrese la direccion y el nombre para el registro.txt\n----> ");
                            scanf("%s", direccion);
                            if (modoLocal == 1)
                            {
                                strcat(direccion, "std_servidor_grupo_7.txt");
                            }
                            else if (modoVisita == 1)
                            {
                                strcat(direccion, "std_cliente_grupo_7.txt");
                            }
                            registro = fopen(direccion, "w");
                        }
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
            system("cls");
            printf("\nAntes de comenzar la partida usted debe hacer: \n");
            printf("1) Elegir si va a jugar como Local o Visitante \n");
            printf("Para cambiar esos valores vaya a Configuracion de parametros \n");
            printf("2) Indicar la ruta para el registro...(la ruta no debe tener espacios)\n");
            printf("Para cambiar esos valores vaya a Configuracion de parametros \n\n");
            printf("Para comenzar la partida seleccione Empezar partida luego de completar los pasos anteriores\n\n");
            system("pause");
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