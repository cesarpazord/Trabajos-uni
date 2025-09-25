#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

// Recursos disponibles (Café, Azúcar, Agua caliente)
char *Recursos[3]={"Café", "Azúcar", "Agua caliente"};

// Ingredientes puestos en la mesa (inicialmente no hay ingredientes)
int IngredienteMesa1=-1;
int IngredienteMesa2=-1;

// Cantidades disponibles de los recursos
int CafeCant=20;
int AzucarCant=20;
int AguaCant=20;

// Variables de sincronización para los hilos
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex para proteger acceso a recursos
pthread_cond_t cond_global = PTHREAD_COND_INITIALIZER;  // Condición para sincronizar hilos

// Bandera que indica si el programa debe terminar
int terminado=0;

// Hilo del agente: selecciona dos ingredientes aleatorios y los pone en la mesa
void *AgenteIngredientes(void *arg) {
    while (terminado == 0) {
        pthread_mutex_lock(&mutex);  // Bloquea el mutex para acceder a las variables compartidas

        // Verifica si alguno de los recursos está agotado
        if (CafeCant <= 0 || AzucarCant <= 0 || AguaCant <= 0) {
            terminado=1;  // Marca el proceso como terminado
            pthread_cond_broadcast(&cond_global);  // Despierta a todos los hilos esperando
            pthread_mutex_unlock(&mutex);  // Desbloquea el mutex
            break;  // Sale del ciclo ya que los ingredientes se han agotado
        }

        int ingrediente1;
        int ingrediente2;
        do {
            ingrediente1=rand() % 3;  // Selecciona aleatoriamente el primer ingrediente
            ingrediente2=rand() % 3;  // Selecciona aleatoriamente el segundo ingrediente
        } while (ingrediente1 == ingrediente2);  // Asegura que sean distintos

        IngredienteMesa1=ingrediente1;  // Asigna el primer ingrediente a la mesa
        IngredienteMesa2=ingrediente2;  // Asigna el segundo ingrediente a la mesa

        // Muestra los ingredientes que ha puesto el agente en la mesa
        printf("\n---Los ingredientes que nos da el Agente son [%s y %s]---\n", Recursos[ingrediente1], Recursos[ingrediente2]);

        pthread_cond_broadcast(&cond_global);  // Despierta a todos los hilos esperando que los ingredientes estén disponibles
        pthread_mutex_unlock(&mutex);  // Desbloquea el mutex para permitir el acceso de otros hilos
        sleep(1);  // Espera 1 segundo antes de colocar más ingredientes
    }

    // Mensaje de finalización del agente cuando se quedan sin recursos
    printf("El Agente se quedó sin ingredientes.\n");
    return NULL;  // Finaliza el hilo
}

// Hilo de cada cafetero: espera a que los ingredientes estén en la mesa y prepara café
void *Cafeteros(void *arg) {
    int cafetero=*(int *)arg;  // Cada cafetero tiene un ingrediente fijo (0: café, 1: azúcar, 2: agua)

    while (terminado == 0) {
        pthread_mutex_lock(&mutex);  // Bloquea el mutex para acceder a las variables compartidas

        // Espera a que haya ingredientes en la mesa y que el ingrediente que le corresponde esté disponible
        while (terminado == 0 && (IngredienteMesa1 == -1 || IngredienteMesa2 == -1 || IngredienteMesa1 == cafetero || IngredienteMesa2 == cafetero)) {
            pthread_cond_wait(&cond_global, &mutex);  // Espera hasta que los ingredientes estén disponibles
        }

        // Si el proceso ha terminado, sale del ciclo
        if (terminado) {
            pthread_mutex_unlock(&mutex);  // Desbloquea el mutex antes de terminar
            break;
        }

        // Prepara café usando el ingrediente que le corresponde y los otros dos ingredientes en la mesa
        printf("El cafetero %d con [%s] prepara café usando [%s] y [%s]\n", (cafetero + 1), Recursos[cafetero], Recursos[IngredienteMesa1], Recursos[IngredienteMesa2]);

        // Resta el ingrediente correspondiente al cafetero
        if (cafetero == 0) {
            CafeCant=CafeCant - 1;  // Resta una unidad de café
        } else if (cafetero == 1) {
            AzucarCant=AzucarCant - 1;  // Resta una unidad de azúcar
        } else {
            AguaCant=AguaCant - 1;  // Resta una unidad de agua caliente
        }

        // Muestra la cantidad de ingredientes restantes
        printf("---Cantidad de ingredientes restantes:--- \n->Café=%d\n->Azúcar=%d\n->Agua caliente=%d\n \n", CafeCant, AzucarCant, AguaCant);

        pthread_mutex_unlock(&mutex);  // Desbloquea el mutex para permitir que otros hilos accedan
        sleep(1);  // Espera 1 segundo antes de intentar preparar más café
    }

    // Mensaje cuando el cafetero termina su trabajo
    printf("Cafetero con [%s] ha terminado.\n", Recursos[cafetero]);
    return NULL;  // Finaliza el hilo
}

int main() {
    srand(time(NULL));  // Inicializa el generador de números aleatorios

    pthread_t hiloAgente;  // Hilo para el agente
    pthread_t hilosCafeteros[3];  // Hilos para los tres cafeteros
    int ingredienteCafetero[3]={0, 1, 2};  // Cada cafetero tiene un recurso asignado (0: café, 1: azúcar, 2: agua)

    // Crea el hilo para el agente
    pthread_create(&hiloAgente, NULL, AgenteIngredientes, NULL);

    // Crea los hilos para los cafeteros
    for (int i = 0; i < 3; i++) {
        pthread_create(&hilosCafeteros[i], NULL, Cafeteros, &ingredienteCafetero[i]);
    }

    // Espera a que todos los hilos terminen
    pthread_join(hiloAgente, NULL);  // Espera a que el hilo del agente termine
    for (int i = 0; i < 3; i++) {
        pthread_join(hilosCafeteros[i], NULL);  // Espera a que cada hilo de cafetero termine
    }

    // Mensaje final cuando el programa termina
    printf("Programa finalizado.\n");
    return 0;  // Termina el programa
}

