#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <pthread.h>
#include <vector>

struct ThreadData {
  int thread_id;
  int num_threads;
  long long N;
};

double global_pi_sum = 0.0;
pthread_mutex_t mutex_pi;

double global_pi_unsync = 0.0;

// 1. Versión Secuencial
double calcular_pi_secuencial(long long N) {
  double suma = 0.0;
  for (long long k = 0; k < N; ++k) {
    double factor = (k % 2 == 0) ? 1.0 : -1.0;
    suma += factor / (2.0 * k + 1.0);
  }
  return 4.0 * suma;
}

// 2. funcion del hilo SIN sincronizacion
void *calcular_pi_race_thread(void *arg) {
  ThreadData *data = static_cast<ThreadData *>(arg);
  long long chunk = data->N / data->num_threads;
  long long inicio = data->thread_id * chunk;
  long long fin =
      (data->thread_id == data->num_threads - 1) ? data->N : inicio + chunk;

  // variable sin mutex
  for (long long k = inicio; k < fin; ++k) {
    double factor = (k % 2 == 0) ? 1.0 : -1.0;
    global_pi_unsync += factor / (2.0 * k + 1.0);
  }
  pthread_exit(NULL);
}

// 3. funcion del hilo CON sincronización
void *calcular_pi_thread(void *arg) {
  ThreadData *data = static_cast<ThreadData *>(arg);
  long long chunk = data->N / data->num_threads;
  long long inicio = data->thread_id * chunk;
  long long fin =
      (data->thread_id == data->num_threads - 1) ? data->N : inicio + chunk;

  // Acumulador local privado
  double suma_local = 0.0;
  for (long long k = inicio; k < fin; ++k) {
    double factor = (k % 2 == 0) ? 1.0 : -1.0;
    suma_local += factor / (2.0 * k + 1.0);
  }

  // uso del mutex
  pthread_mutex_lock(&mutex_pi);
  global_pi_sum += suma_local;
  pthread_mutex_unlock(&mutex_pi);

  pthread_exit(NULL);
}

// Funcion de evaluacion de tiempo
double ejecutar_pi_paralelo(long long N, int num_threads, double &pi_obtenido) {
  global_pi_sum = 0.0;
  pthread_mutex_init(&mutex_pi, NULL);

  std::vector<pthread_t> threads(num_threads);
  std::vector<ThreadData> t_data(num_threads);

  auto t_start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < num_threads; ++i) {
    t_data[i] = {i, num_threads, N};
    pthread_create(&threads[i], NULL, calcular_pi_thread, &t_data[i]);
  }

  for (int i = 0; i < num_threads; ++i) {
    pthread_join(threads[i], NULL);
  }

  auto t_end = std::chrono::high_resolution_clock::now();
  pi_obtenido = 4.0 * global_pi_sum;
  pthread_mutex_destroy(&mutex_pi);

  return std::chrono::duration<double>(t_end - t_start).count();
}

int main() {
  std::cout << "PARTE I: calculo de PI con pthreads\n\n";
  const double PI_REF = M_PI;

  // Demostración de condición de carrera vs sincronización
  std::cout
      << "[DEMOSTRACIÓN DE CONDICIÓN DE CARRERA (N = 10,000,000, 4 hilos)]\n";
  long long demo_N = 10000000LL;
  int demo_threads = 4;
  global_pi_unsync = 0.0;
  std::vector<pthread_t> race_threads(demo_threads);
  std::vector<ThreadData> race_data(demo_threads);
  for (int i = 0; i < demo_threads; ++i) {
    race_data[i] = {i, demo_threads, demo_N};
    pthread_create(&race_threads[i], NULL, calcular_pi_race_thread,
                   &race_data[i]);
  }
  for (int i = 0; i < demo_threads; ++i) {
    pthread_join(race_threads[i], NULL);
  }
  double pi_race = 4.0 * global_pi_unsync;
  std::cout << "  Pi Sin Sincronización (Carrera) : " << std::fixed
            << std::setprecision(10) << pi_race
            << " | Error: " << std::abs(pi_race - PI_REF) << " (CORRUPTO)\n";

  double pi_sync_demo = 0.0;
  ejecutar_pi_paralelo(demo_N, demo_threads, pi_sync_demo);
  std::cout << "  Pi Con Mutex + Suma Local       : " << pi_sync_demo
            << " | Error: " << std::abs(pi_sync_demo - PI_REF)
            << " (CORRECTO)\n\n";

  // Experimentos
  std::vector<long long> tamanos_N = {10000000LL, 100000000LL, 200000000LL};
  std::vector<int> lista_hilos = {1, 2, 4, 6, 8, 10, 12};

  std::ofstream csv("parte1_resultados.csv");
  csv << "N,Threads,Tiempo_Sec,Tiempo_Par,Speedup,Eficiencia,Error_Absoluto,Pi_"
         "Calculado\n";

  for (long long N : tamanos_N) {
    std::cout << " Evaluando N = " << N << " términos de Leibniz\n";

    // version secuencial
    auto t_start = std::chrono::high_resolution_clock::now();
    double pi_seq = calcular_pi_secuencial(N);
    auto t_end = std::chrono::high_resolution_clock::now();
    double tiempo_seq = std::chrono::duration<double>(t_end - t_start).count();

    std::cout << "Secuencial -> PI: " << pi_seq << " | Tiempo: " << std::fixed
              << std::setprecision(6) << tiempo_seq << " s"
              << " | Error: " << std::scientific << std::abs(pi_seq - PI_REF)
              << std::fixed << "\n\n";

    std::cout << std::left << std::setw(10) << "Threads" << std::setw(18)
              << "PI Calculado" << std::setw(14) << "Tiempo (s)"
              << std::setw(12) << "Speedup" << std::setw(12) << "Eficiencia"
              << std::setw(16) << "Error Absoluto" << "\n";

    for (int p : lista_hilos) {
      double pi_par = 0.0;
      double tiempo_par = ejecutar_pi_paralelo(N, p, pi_par);
      double error_abs = std::abs(pi_par - PI_REF);
      double speedup = tiempo_seq / tiempo_par;
      double eficiencia = speedup / p;

      std::cout << std::left << std::setw(10) << p << std::setw(18)
                << std::setprecision(10) << pi_par << std::setw(14)
                << std::setprecision(6) << tiempo_par << std::setw(12)
                << std::setprecision(4) << speedup << std::setw(12)
                << std::setprecision(4) << eficiencia << std::setw(16)
                << std::scientific << std::setprecision(6) << error_abs
                << std::fixed << "\n";

      csv << N << "," << p << "," << tiempo_seq << "," << tiempo_par << ","
          << speedup << "," << eficiencia << "," << error_abs << ","
          << std::setprecision(10) << pi_par << "\n";
    }
    std::cout << "\n";
  }

  csv.close();
  std::cout << "resultado en 'parte1_resultados.csv'.\n\n";
  return 0;
}
