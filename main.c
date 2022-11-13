#include "main.h"

int main()
{
    int decision = 0;
    int x;
    FILE *registro;
    char direccion[1000];
    printf("ingrese la direccion y el nombre para el registro.txt\n----> ");
    scanf("%s",direccion);
    registro=fopen(direccion,"w");  //guarda el archivo en la direccion y con el nombre que el usuario desea
    while (decision != 3)
    {
        printf("\nBienvenido al juego del HIP\n");
        printf("Elija una opcion:\n");
        printf("1- Ingresar como servidor\n");
        printf("2- Ingresar como cliente\n");
        printf("3- Salir\n");
        scanf("%d", &decision);

        if (decision == 1)
        {
            socketServidor();
            char mensaje[303];
            printf("Ingrese el mensaje... \n ----> ");
            scanf("%s",mensaje);
            //char cadena[] = "[1];[cliente-grupo2-jugador2];[servidor-grupo1-jugador1];[Jugar];[activo];[1];[1];[3];[4];[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];#";
            leer_mensaje(registro,mensaje);
        }
        else if (decision == 2)
        {
            socketCliente();
        }
    }
    fclose(registro);
    return 0;
}