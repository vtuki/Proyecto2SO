#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

typedef struct {
    int *buffer;
    int head, tail, count, capacity;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    FILE *log_file;
    bool producers_finished;
} MonitorColaCircular;



// Inicializa la cola circular
MonitorColaCircular* crear_monitor_cola(int initial_size, const char *log_filename) {
    MonitorColaCircular *cola = (MonitorColaCircular *)malloc(sizeof(MonitorColaCircular));
    cola->buffer = (int *)malloc(initial_size * sizeof(int));
    cola->capacity = initial_size;
    cola->head = cola->tail = cola->count = 0;
    pthread_mutex_init(&cola->mutex, NULL);
    pthread_cond_init(&cola->not_empty, NULL);
    pthread_cond_init(&cola->not_full, NULL);
    cola->log_file = fopen(log_filename, "w");
    fprintf(cola->log_file, "Inicio del log.\n");
    cola->producers_finished = false;
    return cola;
}

// Destruye la cola circular y libera recursos
void destruir_monitor_cola(MonitorColaCircular *cola) {
    free(cola->buffer);
    fclose(cola->log_file);
    pthread_mutex_destroy(&cola->mutex);
    pthread_cond_destroy(&cola->not_empty);
    pthread_cond_destroy(&cola->not_full);
    free(cola);
}

// Duplica o reduce el tamaño de la cola
void resize(MonitorColaCircular *cola, int new_capacity) {
    int *new_buffer = (int *)malloc(new_capacity * sizeof(int));
    for (int i = 0; i < cola->count; i++) {
        new_buffer[i] = cola->buffer[(cola->head + i) % cola->capacity];
    }
    free(cola->buffer);
    cola->buffer = new_buffer;
    cola->head = 0;
    cola->tail = cola->count;
    cola->capacity = new_capacity;
}

// Agrega un elemento a la cola
void agregar(MonitorColaCircular *cola, int item) {
    pthread_mutex_lock(&cola->mutex);
    while (cola->count == cola->capacity) {
        resize(cola, cola->capacity * 2);
        fprintf(cola->log_file, "La cola se duplicó a: %d\n", cola->capacity);
    }

    cola->buffer[cola->tail] = item;
    cola->tail = (cola->tail + 1) % cola->capacity;
    cola->count++;
    fprintf(cola->log_file, "Agregado: %d, Tamaño actual: %d\n", item, cola->count);

    pthread_cond_signal(&cola->not_empty);
    pthread_mutex_unlock(&cola->mutex);
}

// Extrae un elemento de la cola
int extraer(MonitorColaCircular *cola) {
    pthread_mutex_lock(&cola->mutex);
    while (cola->count == 0 && !cola->producers_finished) {
        pthread_cond_wait(&cola->not_empty, &cola->mutex);
    }
    if (cola->count == 0 && cola->producers_finished) {
        pthread_mutex_unlock(&cola->mutex);
        return -1;
    }

    int item = cola->buffer[cola->head];
    cola->head = (cola->head + 1) % cola->capacity;
    cola->count--;
    fprintf(cola->log_file, "Extraído: %d, Tamaño actual: %d\n", item, cola->count);

    if (cola->count <= cola->capacity / 4 && cola->capacity > 1) {
        resize(cola, cola->capacity / 2);
        fprintf(cola->log_file, "La cola se redujo a: %d\n", cola->capacity);
    }

    pthread_cond_signal(&cola->not_full);
    pthread_mutex_unlock(&cola->mutex);
    return item;
}

// Marca el fin de los productores
void finalizar_productores(MonitorColaCircular *cola) {
    pthread_mutex_lock(&cola->mutex);
    cola->producers_finished = true;
    pthread_cond_broadcast(&cola->not_empty);
    pthread_mutex_unlock(&cola->mutex);
}

// Función de productor
void* productor(void *arg) {
    MonitorColaCircular *cola = (MonitorColaCircular *)arg;
    for (int i = 0; i < 20; i++) {  // Cada productor genera 20 items
        agregar(cola, i + rand() % 100);  // Generar un número aleatorio
        usleep(50000);  // Simular trabajo (50ms)
    }
    return NULL;
}

// Función de consumidor
void* consumidor(void *arg) {
    MonitorColaCircular *cola = (MonitorColaCircular *)arg;
    while (true) {
        int item = extraer(cola);
        if (item == -1) break;  // No hay más elementos y productores terminaron
        usleep(100000);  // Simular procesamiento (100ms)
    }
    return NULL;
}

// Función principal
#include <getopt.h>

int main(int argc, char* argv[]) {
    int num_productores = 10; // Valor por defecto
    int num_consumidores = 5; // Valor por defecto
    int tam_inicial_cola = 50; // Valor por defecto
    int tiempo_espera = 1;     // Valor por defecto en segundos

    int opt;
    while ((opt = getopt(argc, argv, "p:c:s:t:")) != -1) {
        switch (opt) {
            case 'p':
                num_productores = atoi(optarg);
                break;
            case 'c':
                num_consumidores = atoi(optarg);
                break;
            case 's':
                tam_inicial_cola = atoi(optarg);
                break;
            case 't':
                tiempo_espera = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Uso: %s -p <num_productores> -c <num_consumidores> -s <tam_inicial_cola> -t <tiempo_espera>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    MonitorColaCircular *cola = crear_monitor_cola(tam_inicial_cola, "log.txt");

    // Crear hilos de productores
    pthread_t productores[num_productores];
    for (int i = 0; i < num_productores; i++) {
        pthread_create(&productores[i], NULL, productor, cola);
    }

    // Crear hilos de consumidores
    pthread_t consumidores[num_consumidores];
    for (int i = 0; i < num_consumidores; i++) {
        pthread_create(&consumidores[i], NULL, consumidor, cola);
    }

    // Esperar a que terminen los productores
    for (int i = 0; i < num_productores; i++) {
        pthread_join(productores[i], NULL);
    }
    finalizar_productores(cola);

    // Esperar a que terminen los consumidores
    for (int i = 0; i < num_consumidores; i++) {
        pthread_join(consumidores[i], NULL);
    }

    printf("Simulacion finalizada.\nRevise log.txt para el registro de eventos.\n");
    destruir_monitor_cola(cola);
    return 0;
}
