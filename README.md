# Laboratorio de Programación con Pthreads: Concurrencia y Sincronización

Este repositorio contiene la solución completa a todas las actividades propuestas en la guía de laboratorio: **Cálculo de $\pi$**, **Multiplicación Matriz-Vector**, **Lista Enlazada Concurrente** e **Implementación de Barreras**.

---

## Entorno de Ejecución y Hardware

Los experimentos fueron ejecutados bajo el siguiente entorno computacional:
* **Procesador (CPU):** AMD Ryzen 5 5625U with Radeon Graphics.
* **Arquitectura:** x86_64 (6 núcleos físicos, 12 hilos por SMT/Hyper-Threading).
* **Frecuencia Base / Boost:** 2.30 GHz (Boost dinámico hasta ~4.39 GHz).
* **Jerarquía de Caché:** L1d: 192 KiB, L1i: 192 KiB, L2: 3 MiB, L3: 16 MiB compartida.
* **Sistema Operativo:** Linux x86_64.
* **Compilador:** GCC (`g++`) con soporte C++17 y optimización `-O3 -pthread`.
* **Python:** Python 3 con librerías `matplotlib` y `pandas`.

---

## Instrucciones de Compilación y Ejecución

El proyecto incluye un [`Makefile`](file:///home/JustLuis/Documents/paralela/lab06/Makefile) que simplifica el flujo de trabajo:

### 3.1. Compilación
Para compilar los cuatro programas optimizados con Pthreads:
```bash
make
```
*(O de forma individual: `make parte1_pi`, `make parte2_matvec`, `make parte3_lista`, `make parte4_barreras`)*.

### 3.2. Ejecución de los Experimentos
Para ejecutar todas las partes en orden secuencial y actualizar automáticamente los archivos `.csv`:
```bash
make run
```
Para ejecutar una parte en particular:
```bash
make run1    # Ejecuta únicamente la Parte I (Cálculo de Pi)
make run2    # Ejecuta únicamente la Parte II (Multiplicación Matriz-Vector)
make run3    # Ejecuta únicamente la Parte III (Lista Enlazada)
make run4    # Ejecuta únicamente la Parte IV (Barreras)
```

### 3.3. Generación de Gráficas
Una vez generados los archivos CSV, ejecute:
```bash
make plot
# O directamente: python3 generar_graficas.py
```
Todas las figuras de alta resolución (300 DPI) se guardarán en el directorio `graficas/`.

### 3.4. Limpieza
* Para limpiar ejecutables y archivos binarios: `make clean`
* Para limpiar ejecutables, CSVs y la carpeta `graficas/`: `make clean-all`

---


