#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define HASH_SIZE 101 // Tamaño de la tabla hash (ajustable)

// Nodo para la lista en el hash table
typedef struct PageNode {
    int page;
    struct PageNode *next;
} PageNode;

// Estructura de la tabla hash
typedef struct HashTable {
    PageNode *buckets[HASH_SIZE];
} HashTable;

// Crear tabla hash
HashTable* createHashTable() {
    HashTable *table = (HashTable *)malloc(sizeof(HashTable));
    for (int i = 0; i < HASH_SIZE; i++) {
        table->buckets[i] = NULL;
    }
    return table;
}

// Hash function
int hashFunction(int page) {
    return page % HASH_SIZE;
}

// Insertar página en la tabla hash
void insertPage(HashTable *table, int page) {
    int index = hashFunction(page);
    PageNode *newNode = (PageNode *)malloc(sizeof(PageNode));
    newNode->page = page;
    newNode->next = table->buckets[index];
    table->buckets[index] = newNode;
}

// Verificar si la página existe en la tabla
bool isPageInTable(HashTable *table, int page) {
    int index = hashFunction(page);
    PageNode *current = table->buckets[index];
    while (current) {
        if (current->page == page) {
            return true;
        }
        current = current->next;
    }
    return false;
}

// Eliminar página de la tabla hash
void removePage(HashTable *table, int page) {
    int index = hashFunction(page);
    PageNode *current = table->buckets[index];
    PageNode *prev = NULL;

    while (current) {
        if (current->page == page) {
            if (prev) {
                prev->next = current->next;
            } else {
                table->buckets[index] = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

typedef struct Queue {
    int *pages;      // Arreglo para almacenar las páginas
    int front;       // Índice del frente de la cola
    int rear;        // Índice del final de la cola
    int size;        // Tamaño actual de la cola
    int capacity;    // Capacidad máxima de la cola
} Queue;

// Crear una nueva cola
Queue* createQueue(int capacity) {
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    queue->pages = (int *)malloc(capacity * sizeof(int));
    queue->front = 0;
    queue->rear = -1;
    queue->size = 0;
    queue->capacity = capacity;
    return queue;
}

// Insertar un elemento en la cola
void enqueue(Queue *queue, int page) {
    if (queue->size == queue->capacity) {
        return; // La cola está llena
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->pages[queue->rear] = page;
    queue->size++;
}

// Eliminar un elemento del frente de la cola
int dequeue(Queue *queue) {
    if (queue->size == 0) {
        return -1; // La cola está vacía
    }
    int page = queue->pages[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size--;
    return page;
}

// Verificar si la cola está llena
int isFull(Queue *queue) {
    return queue->size == queue->capacity;
}

void fifoAlgorithm(int marcos, int *referencias, int count) {
    Queue *memoria = createQueue(marcos);  // Crear la cola para los marcos
    HashTable *table = createHashTable();  // Crear la tabla de páginas
    int fallos = 0;                        // Contador de fallos de página

    for (int i = 0; i < count; i++) {
        int pagina = referencias[i];

        if (!isPageInTable(table, pagina)) { // Si la página no está en memoria
            fallos++;

            if (isFull(memoria)) { // Si la memoria está llena
                int paginaEliminada = dequeue(memoria);
                removePage(table, paginaEliminada);
            }

            enqueue(memoria, pagina); // Añadir la nueva página
            insertPage(table, pagina);
        }
    }

    printf("Numero de fallos de pagina: %d\n", fallos);

    // Liberar memoria
    free(memoria->pages);
    free(memoria);
    free(table);
}

#include <limits.h>

// Algoritmo LRU
void lruAlgorithm(int marcos, int *referencias, int count) {
    int memoria[marcos];   // Arreglo para representar los marcos
    int tiempo[marcos];    // Tiempos de uso para cada marco
    int fallos = 0;

    for (int i = 0; i < marcos; i++) {
        memoria[i] = -1;   // Inicializar los marcos como vacíos
        tiempo[i] = 0;     // Inicializar tiempos
    }

    for (int t = 0; t < count; t++) {
        int pagina = referencias[t];
        int encontrado = 0;

        // Verificar si la página ya está en memoria
        for (int i = 0; i < marcos; i++) {
            if (memoria[i] == pagina) {
                encontrado = 1;
                tiempo[i] = t; // Actualizar el tiempo de uso
                break;
            }
        }

        if (!encontrado) { // Página no encontrada (fallo)
            fallos++;
            int lru = 0; // Índice de la página menos recientemente usada

            for (int i = 1; i < marcos; i++) {
                if (tiempo[i] < tiempo[lru]) {
                    lru = i;
                }
            }

            memoria[lru] = pagina;  // Reemplazar página menos usada
            tiempo[lru] = t;        // Actualizar tiempo
        }
    }

    printf("Numero de fallos de pagina: %d\n", fallos);
}

void clockAlgorithm(int marcos, int *referencias, int count) {
    int memoria[marcos];     // Arreglo para los marcos
    int bits[marcos];        // Bits de referencia
    int puntero = 0;         // Puntero del reloj
    int fallos = 0;

    for (int i = 0; i < marcos; i++) {
        memoria[i] = -1;     // Inicializar los marcos como vacíos
        bits[i] = 0;         // Inicializar bits de referencia
    }

    for (int i = 0; i < count; i++) {
        int pagina = referencias[i];
        int encontrado = 0;

        // Verificar si la página ya está en memoria
        for (int j = 0; j < marcos; j++) {
            if (memoria[j] == pagina) {
                encontrado = 1;
                bits[j] = 1; // Actualizar el bit de referencia
                break;
            }
        }

        if (!encontrado) { // Fallo de página
            fallos++;

            while (bits[puntero] == 1) { // Buscar página reemplazable
                bits[puntero] = 0;
                puntero = (puntero + 1) % marcos;
            }

            // Reemplazar página
            memoria[puntero] = pagina;
            bits[puntero] = 1;
            puntero = (puntero + 1) % marcos;
        }
    }

    printf("Numero de fallos de pagina: %d\n", fallos);
}

void optimalAlgorithm(int marcos, int *referencias, int count) {
    int memoria[marcos];   // Arreglo para los marcos
    int fallos = 0;

    for (int i = 0; i < marcos; i++) {
        memoria[i] = -1;   // Inicializar los marcos como vacíos
    }

    for (int i = 0; i < count; i++) {
        int pagina = referencias[i];
        int encontrado = 0;

        // Verificar si la página ya está en memoria
        for (int j = 0; j < marcos; j++) {
            if (memoria[j] == pagina) {
                encontrado = 1;
                break;
            }
        }

        if (!encontrado) { // Fallo de página
            fallos++;
            int reemplazo = -1;
            int maxDistancia = -1;

            for (int j = 0; j < marcos; j++) {
                if (memoria[j] == -1) { // Marco vacío
                    reemplazo = j;
                    break;
                }

                // Calcular la próxima aparición
                int distancia = INT_MAX;
                for (int k = i + 1; k < count; k++) {
                    if (referencias[k] == memoria[j]) {
                        distancia = k;
                        break;
                    }
                }

                if (distancia > maxDistancia) {
                    maxDistancia = distancia;
                    reemplazo = j;
                }
            }

            memoria[reemplazo] = pagina; // Reemplazar página
        }
    }

    printf("Numero de fallos de pagina: %d\n", fallos);
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        printf("Uso: ./mvirtual -m <marcos> -a <algoritmo> -f <archivo>\n");
        return 1;
    }

    int marcos = 0;        // Número de marcos de memoria
    char algoritmo[10];    // Algoritmo de reemplazo
    char archivo[50];      // Archivo con referencias de memoria

    // Leer parámetros
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0) {
            marcos = atoi(argv[++i]);  // Leer número de marcos
        } else if (strcmp(argv[i], "-a") == 0) {
            strcpy(algoritmo, argv[++i]);  // Leer el algoritmo
        } else if (strcmp(argv[i], "-f") == 0) {
            strcpy(archivo, argv[++i]);  // Leer el nombre del archivo
        }
    }

    if (marcos <= 0) {
        printf("Numero de marcos invalido.\n");
        return 1;
    }

    // Leer referencias de memoria del archivo
    FILE *file = fopen(archivo, "r");
    if (!file) {
        printf("No se pudo abrir el archivo %s.\n", archivo);
        return 1;
    }

    int referencias[1000];  // Arreglo para almacenar referencias
    int count = 0;          // Contador de referencias
    while (fscanf(file, "%d", &referencias[count]) != EOF) {
        count++;
    }
    fclose(file);

    // Llamar al algoritmo correspondiente
    if (strcmp(algoritmo, "FIFO") == 0) {
        fifoAlgorithm(marcos, referencias, count);
    } else if (strcmp(algoritmo, "LRU") == 0) {
        lruAlgorithm(marcos, referencias, count);
    } else if (strcmp(algoritmo, "CLOCK") == 0) {
        clockAlgorithm(marcos, referencias, count);
    } else if (strcmp(algoritmo, "OPT") == 0) {
        optimalAlgorithm(marcos, referencias, count);
    } else {
        printf("Algoritmo no reconocido: %s\n", algoritmo);
        return 1;
    }

    return 0;
}