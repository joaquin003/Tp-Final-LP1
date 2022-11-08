#include "main.h"

int main()
{
    int decision = 0;
    int x;

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
        }
        else if (decision == 2)
        {
            socketCliente();
        }
    }

    return 0;
}