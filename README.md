# Proyecto2SO

# P1) Simulación Productores-Consumidores

Este proyecto implementa una simulación de productores y consumidores utilizando una cola circular protegida por un monitor (mutex y variables de condición) en C. La simulación permite configurar el número de productores, consumidores, tamaño inicial de la cola y tiempo de espera máximo para los consumidores.

## Características
- Productores generan elementos aleatorios y los colocan en la cola.
- Consumidores extraen elementos de la cola para procesarlos.
- La cola puede ajustar dinámicamente su tamaño (doblar o reducir según necesidad).
- Log de eventos almacenado en `log.txt`.

---

## Requisitos
- **Compilador C** compatible con POSIX (`gcc` recomendado).
- Biblioteca de hilos (`pthread`).

---

## Uso

### 1. Compilar
Ejecuta el siguiente comando en tu terminal para compilar el programa:

```bash
gcc -o simulapc p1.c -lpthread


# P2) Simulador de Memoria Virtual con Paginación

Este proyecto implementa un simulador de memoria virtual que utiliza paginación y algoritmos de reemplazo de páginas. El programa está escrito en lenguaje C y permite evaluar el rendimiento de diferentes estrategias de reemplazo mediante el cálculo de fallos de página.

## Características

El simulador soporta los siguientes algoritmos de reemplazo de páginas:

- **FIFO (First In, First Out)**
- **LRU (Least Recently Used)**
- **Reloj Simple**
- **Óptimo**

## Requisitos

- Compilador GCC (GNU Compiler Collection)
- Archivo de referencias de memoria (`referencias.txt`)

## Compilación

1. Asegúrate de tener GCC instalado en tu sistema.
2. Compila el programa ejecutando el siguiente comando en la terminal:
   ```bash
   gcc p2.c -o mvirtual

