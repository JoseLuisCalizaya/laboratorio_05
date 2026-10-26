#!/usr/bin/env python3
"""
===============================================================================
GENERADOR DE GRÁFICAS DE RENDIMIENTO Y CONCURRENCIA
Laboratorio Pthreads: Cálculo de PI, Matriz-Vector, Lista Enlazada y Barreras
===============================================================================
"""

import os
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# Configuración estética general
plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')
plt.rcParams['font.sans-serif'] = 'DejaVu Sans'
plt.rcParams['axes.edgecolor'] = '#cccccc'
plt.rcParams['axes.linewidth'] = 0.8
plt.rcParams['grid.color'] = '#e0e0e0'
plt.rcParams['grid.linestyle'] = '--'
plt.rcParams['grid.alpha'] = 0.7

OUTPUT_DIR = "graficas"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def graficar_parte1():
    print("[+] Generando gráficas de Parte I (Cálculo de Pi)...")
    if not os.path.exists("parte1_resultados.csv"):
        print("    [!] 'parte1_resultados.csv' no encontrado. Omitiendo...")
        return

    df = pd.read_csv("parte1_resultados.csv")
    tamanos = df["N"].unique()
    colores = ['#1f77b4', '#ff7f0e', '#2ca02c']

    # 1. Gráfica de Tiempo vs Hilos
    fig, ax = plt.subplots(figsize=(8, 5), dpi=300)
    for i, n in enumerate(tamanos):
        sub = df[df["N"] == n]
        ax.plot(sub["Threads"], sub["Tiempo_Par"], marker='o', linewidth=2, color=colores[i % len(colores)], label=f'N = {n:,}')
    ax.set_title(r"Parte I: Tiempo de Ejecución vs Número de Hilos (Cálculo de $\pi$)", fontsize=13, fontweight='bold', pad=12)
    ax.set_xlabel("Número de Hilos (Pthreads)", fontsize=11)
    ax.set_ylabel("Tiempo de Ejecución (s)", fontsize=11)
    ax.set_xticks(sorted(df["Threads"].unique()))
    ax.legend(frameon=True, facecolor='white', framealpha=0.9)
    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, "parte1_tiempo.png"))
    plt.close()

    # 2. Gráfica de Speedup vs Hilos
    fig, ax = plt.subplots(figsize=(8, 5), dpi=300)
    hilos = sorted(df["Threads"].unique())
    ax.plot(hilos, hilos, 'k--', linewidth=1.5, label='Speedup Ideal Lineal ($S = p$)')
    for i, n in enumerate(tamanos):
        sub = df[df["N"] == n]
        ax.plot(sub["Threads"], sub["Speedup"], marker='s', linewidth=2, color=colores[i % len(colores)], label=f'N = {n:,}')
    ax.set_title("Parte I: Speedup vs Número de Hilos (Aceleración)", fontsize=13, fontweight='bold', pad=12)
    ax.set_xlabel("Número de Hilos (Pthreads)", fontsize=11)
    ax.set_ylabel("Speedup ($S = T_{sec} / T_{par}$)", fontsize=11)
    ax.set_xticks(hilos)
    ax.legend(frameon=True, facecolor='white', framealpha=0.9)
    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, "parte1_speedup.png"))
    plt.close()

    # 3. Gráfica de Eficiencia vs Hilos
    fig, ax = plt.subplots(figsize=(8, 5), dpi=300)
    ax.axhline(1.0, color='gray', linestyle='--', linewidth=1.2, label='Eficiencia Ideal (1.0)')
    for i, n in enumerate(tamanos):
        sub = df[df["N"] == n]
        ax.plot(sub["Threads"], sub["Eficiencia"], marker='^', linewidth=2, color=colores[i % len(colores)], label=f'N = {n:,}')
    ax.set_title("Parte I: Eficiencia vs Número de Hilos", fontsize=13, fontweight='bold', pad=12)
    ax.set_xlabel("Número de Hilos (Pthreads)", fontsize=11)
    ax.set_ylabel("Eficiencia ($E = S / p$)", fontsize=11)
    ax.set_xticks(hilos)
    ax.set_ylim(0, 1.15)
    ax.legend(frameon=True, facecolor='white', framealpha=0.9)
    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, "parte1_eficiencia.png"))
    plt.close()

def graficar_parte2():
    print("[+] Generando gráficas de Parte II (Multiplicación Matriz-Vector)...")
    if not os.path.exists("parte2_resultados.csv"):
        print("    [!] 'parte2_resultados.csv' no encontrado. Omitiendo...")
        return

    df = pd.read_csv("parte2_resultados.csv")
    tamanos = df["N"].unique()
    palette = ['#2ca02c', '#1f77b4', '#ff7f0e', '#d62728']

    # 1. Tiempo vs Hilos
    fig, ax = plt.subplots(figsize=(8, 5), dpi=300)
    for i, n in enumerate(tamanos):
        sub = df[df["N"] == n]
        ax.plot(sub["Threads"], sub["Tiempo_Par"], marker='o', linewidth=2, color=palette[i % len(palette)], label=f'Matriz {n}x{n}')
    ax.set_title("Parte II: Tiempo vs Hilos (Multiplicación Matriz-Vector)", fontsize=13, fontweight='bold', pad=12)
    ax.set_xlabel("Número de Hilos", fontsize=11)
    ax.set_ylabel("Tiempo de Ejecución (s)", fontsize=11)
    ax.set_xticks(sorted(df["Threads"].unique()))
    ax.legend(frameon=True, facecolor='white', framealpha=0.9)
    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, "parte2_tiempo.png"))
    plt.close()

    # 2. Speedup vs Hilos
    fig, ax = plt.subplots(figsize=(8, 5), dpi=300)
    hilos = sorted(df["Threads"].unique())
    ax.plot(hilos, hilos, 'k--', linewidth=1.5, label='Speedup Ideal ($S = p$)')
    for i, n in enumerate(tamanos):
        sub = df[df["N"] == n]
        ax.plot(sub["Threads"], sub["Speedup"], marker='s', linewidth=2, color=palette[i % len(palette)], label=f'Matriz {n}x{n}')
    ax.set_title("Parte II: Speedup vs Hilos (Matriz-Vector)", fontsize=13, fontweight='bold', pad=12)
    ax.set_xlabel("Número de Hilos", fontsize=11)
    ax.set_ylabel("Speedup ($S = T_{sec} / T_{par}$)", fontsize=11)
    ax.set_xticks(hilos)
    ax.legend(frameon=True, facecolor='white', framealpha=0.9)
    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, "parte2_speedup.png"))
    plt.close()

    # 3. Eficiencia vs Hilos
    fig, ax = plt.subplots(figsize=(8, 5), dpi=300)
    ax.axhline(1.0, color='gray', linestyle='--', linewidth=1.2, label='Eficiencia Ideal (1.0)')
    for i, n in enumerate(tamanos):
        sub = df[df["N"] == n]
        ax.plot(sub["Threads"], sub["Eficiencia"], marker='^', linewidth=2, color=palette[i % len(palette)], label=f'Matriz {n}x{n}')
    ax.set_title("Parte II: Eficiencia vs Hilos (Matriz-Vector)", fontsize=13, fontweight='bold', pad=12)
    ax.set_xlabel("Número de Hilos", fontsize=11)
    ax.set_ylabel("Eficiencia ($E = S / p$)", fontsize=11)
    ax.set_xticks(hilos)
    ax.set_ylim(0, 1.2)
    ax.legend(frameon=True, facecolor='white', framealpha=0.9)
    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, "parte2_eficiencia.png"))
    plt.close()

def graficar_parte3():
    print("[+] Generando gráficas de Parte III (Lista Enlazada Multithreading)...")
    
    # 1. Gráfica de Consistencia / Integridad
    if os.path.exists("parte3_comparacion.csv"):
        df_comp = pd.read_csv("parte3_comparacion.csv")
        fig, ax = plt.subplots(figsize=(7, 5), dpi=300)
        labels = ['Sin Sincronización\n(Condición de Carrera)', 'Con Sincronización\n(Mutex POSIX)']
        esperados = df_comp["Esperados"].values
        obtenidos = df_comp["Obtenidos"].values
        perdidos = df_comp["Perdidos"].values

        x = np.arange(len(labels))
        width = 0.35

        bars1 = ax.bar(x - width/2, esperados, width, label='Nodos Esperados', color='#4575b4')
        bars2 = ax.bar(x + width/2, obtenidos, width, label='Nodos Obtenidos', color=['#d73027', '#1a9850'])

        ax.set_ylabel("Cantidad de Nodos", fontsize=11)
        ax.set_title("Parte III: Consistencia e Integridad de la Lista Enlazada", fontsize=13, fontweight='bold', pad=12)
        ax.set_xticks(x)
        ax.set_xticklabels(labels, fontsize=10)
        ax.legend(frameon=True, facecolor='white', framealpha=0.9)

        # Añadir etiquetas con el valor
        for bar in bars1:
            h = bar.get_height()
            ax.annotate(f'{int(h):,}', xy=(bar.get_x() + bar.get_width() / 2, h), xytext=(0, 3),
                        textcoords="offset points", ha='center', va='bottom', fontsize=9)
        for i, bar in enumerate(bars2):
            h = bar.get_height()
            txt = f'{int(h):,}' if perdidos[i] == 0 else f'{int(h):,}\n(-{perdidos[i]:,})'
            ax.annotate(txt, xy=(bar.get_x() + bar.get_width() / 2, h), xytext=(0, 3),
                        textcoords="offset points", ha='center', va='bottom', fontsize=9,
                        fontweight='bold' if perdidos[i] > 0 else 'normal')

        plt.tight_layout()
        plt.savefig(os.path.join(OUTPUT_DIR, "parte3_consistencia.png"))
        plt.close()

    # 2. Gráfica de Rendimiento y Throughput con Mutex
    if os.path.exists("parte3_rendimiento.csv"):
        df_perf = pd.read_csv("parte3_rendimiento.csv")
        fig, ax1 = plt.subplots(figsize=(8, 5), dpi=300)
        color = '#d95f02'
        ax1.set_xlabel('Número de Hilos (Pthreads)', fontsize=11)
        ax1.set_ylabel('Tiempo de Ejecución (s)', color=color, fontsize=11)
        line1 = ax1.plot(df_perf["Threads"], df_perf["Tiempo_s"], color=color, marker='o', linewidth=2, label='Tiempo (s)')
        ax1.tick_params(axis='y', labelcolor=color)
        ax1.set_xticks(df_perf["Threads"])

        ax2 = ax1.twinx()
        color = '#7570b3'
        ax2.set_ylabel('Throughput (ops / seg)', color=color, fontsize=11)
        line2 = ax2.plot(df_perf["Threads"], df_perf["Throughput_Ops_Sec"], color=color, marker='s', linewidth=2, linestyle='--', label='Throughput')
        ax2.tick_params(axis='y', labelcolor=color)
        ax2.grid(False)

        lines = line1 + line2
        labels = [l.get_label() for l in lines]
        ax1.legend(lines, labels, loc='center right', frameon=True, facecolor='white', framealpha=0.9)
        plt.title("Parte III: Costo de Contención de Mutex Global vs Número de Hilos", fontsize=13, fontweight='bold', pad=12)
        plt.tight_layout()
        plt.savefig(os.path.join(OUTPUT_DIR, "parte3_rendimiento.png"))
        plt.close()

def graficar_parte4():
    print("[+] Generando gráfica UNIFICADA de Parte IV (Comparación de las 3 Barreras)...")
    if not os.path.exists("parte4_resultados.csv"):
        print("    [!] 'parte4_resultados.csv' no encontrado. Omitiendo...")
        return

    df = pd.read_csv("parte4_resultados.csv")
    hilos = df["Threads"]

    fig, ax = plt.subplots(figsize=(9, 5.5), dpi=300)
    
    # Graficar las 3 barreras en el mismo gráfico
    ax.plot(hilos, df["BusyWait_Time"], marker='o', color='#e41a1c', linewidth=2.2, markersize=7, label='1. Busy-Waiting + Mutex (Spin-waiting)')
    ax.plot(hilos, df["Semaphore_Time"], marker='^', color='#377eb8', linewidth=2.2, markersize=7, label='2. Semáforos POSIX (Bloqueo Pasivo)')
    ax.plot(hilos, df["CondVar_Time"], marker='s', color='#4daf4a', linewidth=2.2, markersize=7, label='3. Variables de Condición (pthread_cond_t)')

    ax.set_title("Parte IV: Comparativa de Rendimiento de las 3 Barreras (25 Fases)", fontsize=13, fontweight='bold', pad=14)
    ax.set_xlabel("Número de Hilos (Pthreads)", fontsize=11)
    ax.set_ylabel("Tiempo Total de Ejecución y Sincronización (s)", fontsize=11)
    ax.set_xticks(hilos)
    ax.legend(frameon=True, facecolor='white', framealpha=0.95, loc='upper left', fontsize=10)

    # Anotaciones descriptivas
    plt.tight_layout()
    plt.savefig(os.path.join(OUTPUT_DIR, "parte4_comparacion_barreras.png"))
    plt.close()
    print("    -> Guardada exitosamente: 'graficas/parte4_comparacion_barreras.png'")

if __name__ == "__main__":
    print("=================================================================")
    print("        INICIANDO GENERACIÓN DE GRÁFICAS DE RESULTADOS           ")
    print("=================================================================")
    graficar_parte1()
    graficar_parte2()
    graficar_parte3()
    graficar_parte4()
    print("=================================================================")
    print("   TODAS LAS GRÁFICAS HAN SIDO GENERADAS EN LA CARPETA 'graficas/'")
    print("=================================================================")
