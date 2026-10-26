
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
  int rows;
  int cols;
  const std::vector<double> *A;
  const std::vector<double> *x;
  std::vector<double> *y;
};

// version secuencial
void mat_vec_secuencial(int rows, int cols, const std::vector<double> &A,
                        const std::vector<double> &x, std::vector<double> &y) {
  for (int i = 0; i < rows; ++i) {
    double sum = 0.0;
    int row_offset = i * cols;
    for (int j = 0; j < cols; ++j) {
      sum += A[row_offset + j] * x[j];
    }
    y[i] = sum;
  }
}

// funcion del Hilo pthread
void *mat_vec_thread(void *arg) {
  ThreadData *data = static_cast<ThreadData *>(arg);
  int rows_per_thread = data->rows / data->num_threads;
  int start_row = data->thread_id * rows_per_thread;
  int end_row = (data->thread_id == data->num_threads - 1)
                    ? data->rows
                    : start_row + rows_per_thread;

  const auto &A = *(data->A);
  const auto &x = *(data->x);
  auto &y = *(data->y);

  for (int i = start_row; i < end_row; ++i) {
    double sum = 0.0;
    int row_offset = i * data->cols;
    for (int j = 0; j < data->cols; ++j) {
      sum += A[row_offset + j] * x[j];
    }
    y[i] = sum;
  }

  pthread_exit(NULL);
}

//  evaluacion secuencial y paralelo
bool verificar_resultados(const std::vector<double> &y_seq,
                          const std::vector<double> &y_par, double tol = 1e-9) {
  if (y_seq.size() != y_par.size())
    return false;
  for (size_t i = 0; i < y_seq.size(); ++i) {
    if (std::abs(y_seq[i] - y_par[i]) > tol) {
      return false;
    }
  }
  return true;
}

int main() {
  std::cout << "parte 2: multiplicacion matriz-vector";

  std::vector<int> tamanos = {1000, 2000, 4000, 8000};
  std::vector<int> hilos_lista = {1, 2, 4, 6, 8, 10, 12};

  std::ofstream csv("parte2_resultados.csv");
  csv << "N,Threads,Tiempo_Sec,Tiempo_Par,Speedup,Eficiencia,Verificado\n";

  for (int N : tamanos) {
    int rows = N;
    int cols = N;

    std::cout << " Experimento: Matriz cuadrada de " << rows << " x " << cols
              << " ("
              << (double)(rows * cols * sizeof(double)) / (1024.0 * 1024.0)
              << " MB)\n";

    // inicializacion de datos
    std::vector<double> A(rows * cols);
    std::vector<double> x(cols);
    std::vector<double> y_seq(rows, 0.0);
    std::vector<double> y_par(rows, 0.0);

    for (int i = 0; i < rows * cols; ++i)
      A[i] = (i % 100) * 0.01 + 1.0;
    for (int j = 0; j < cols; ++j)
      x[j] = (j % 50) * 0.02 + 0.5;

    // medicion secuencial
    auto t_start = std::chrono::high_resolution_clock::now();
    mat_vec_secuencial(rows, cols, A, x, y_seq);
    auto t_end = std::chrono::high_resolution_clock::now();
    double tiempo_seq = std::chrono::duration<double>(t_end - t_start).count();

    std::cout << "Tiempo Secuencial de referencia : " << std::fixed
              << std::setprecision(6) << tiempo_seq << " s\n\n";

    std::cout << std::left << std::setw(10) << "Threads" << std::setw(14)
              << "Correcto" << std::setw(16) << "Tiempo (s)" << std::setw(14)
              << "Speedup" << std::setw(14) << "Eficiencia" << "\n";

    for (int num_threads : hilos_lista) {
      std::fill(y_par.begin(), y_par.end(), 0.0);
      std::vector<pthread_t> threads(num_threads);
      std::vector<ThreadData> t_data(num_threads);

      t_start = std::chrono::high_resolution_clock::now();

      for (int i = 0; i < num_threads; ++i) {
        t_data[i] = {i, num_threads, rows, cols, &A, &x, &y_par};
        pthread_create(&threads[i], NULL, mat_vec_thread, &t_data[i]);
      }

      for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
      }

      t_end = std::chrono::high_resolution_clock::now();
      double tiempo_par =
          std::chrono::duration<double>(t_end - t_start).count();

      bool correcto = verificar_resultados(y_seq, y_par);
      double speedup = tiempo_seq / tiempo_par;
      double eficiencia = speedup / num_threads;

      std::cout << std::left << std::setw(10) << num_threads << std::setw(14)
                << (correcto ? "SI (VERIF)" : "ERROR") << std::setw(16)
                << std::setprecision(6) << tiempo_par << std::setw(14)
                << std::setprecision(4) << speedup << std::setw(14)
                << std::setprecision(4) << eficiencia << "\n";

      csv << N << "," << num_threads << "," << tiempo_seq << "," << tiempo_par
          << "," << speedup << "," << eficiencia << "," << (correcto ? 1 : 0)
          << "\n";
    }
    std::cout << "\n";
  }

  csv.close();
  std::cout << "resultado en 'parte2_resultados.csv'.\n\n";
  return 0;
}
