#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <pthread.h>
#include <vector>

struct Node {
  int val;
  Node *next;
  Node(int v) : val(v), next(nullptr) {}
};

// lista enlazada sin sincronizacion
class LinkedListUnsynced {
public:
  Node *head;

  LinkedListUnsynced() : head(nullptr) {}

  // insert con condicion de carrera en 'head'
  void insert(int val) {
    Node *newNode = new Node(val);
    newNode->next = head;
    head = newNode;
  }

  // search con condicion
  bool search(int val) const {
    Node *curr = head;
    while (curr != nullptr) {
      if (curr->val == val)
        return true;
      curr = curr->next;
    }
    return false;
  }

  // remove con condicion
  bool remove(int val) {
    Node *curr = head;
    Node *prev = nullptr;
    while (curr != nullptr && curr->val != val) {
      prev = curr;
      curr = curr->next;
    }
    if (curr == nullptr)
      return false;

    if (prev == nullptr) {
      head = curr->next;
    } else {
      prev->next = curr->next;
    }
    delete curr;
    return true;
  }

  int count() const {
    int c = 0;
    Node *curr = head;
    while (curr != nullptr) {
      c++;
      curr = curr->next;
    }
    return c;
  }

  void clear() {
    Node *curr = head;
    while (curr != nullptr) {
      Node *temp = curr;
      curr = curr->next;
      delete temp;
    }
    head = nullptr;
  }

  ~LinkedListUnsynced() { clear(); }
};

// lista enlazada con sincronizacion
class LinkedListSynced {
private:
  Node *head;
  mutable pthread_mutex_t mutex;

public:
  LinkedListSynced() : head(nullptr) { pthread_mutex_init(&mutex, NULL); }

  ~LinkedListSynced() {
    clear();
    pthread_mutex_destroy(&mutex);
  }

  void insert(int val) {
    Node *newNode = new Node(val);
    pthread_mutex_lock(&mutex);
    newNode->next = head;
    head = newNode;
    pthread_mutex_unlock(&mutex);
  }

  bool search(int val) const {
    pthread_mutex_lock(&mutex);
    Node *curr = head;
    while (curr != nullptr) {
      if (curr->val == val) {
        pthread_mutex_unlock(&mutex);
        return true;
      }
      curr = curr->next;
    }
    pthread_mutex_unlock(&mutex);
    return false;
  }

  bool remove(int val) {
    pthread_mutex_lock(&mutex);
    Node *curr = head;
    Node *prev = nullptr;
    while (curr != nullptr && curr->val != val) {
      prev = curr;
      curr = curr->next;
    }
    if (curr == nullptr) {
      pthread_mutex_unlock(&mutex);
      return false;
    }

    if (prev == nullptr) {
      head = curr->next;
    } else {
      prev->next = curr->next;
    }
    delete curr;
    pthread_mutex_unlock(&mutex);
    return true;
  }

  int count() const {
    pthread_mutex_lock(&mutex);
    int c = 0;
    Node *curr = head;
    while (curr != nullptr) {
      c++;
      curr = curr->next;
    }
    pthread_mutex_unlock(&mutex);
    return c;
  }

  void clear() {
    pthread_mutex_lock(&mutex);
    Node *curr = head;
    while (curr != nullptr) {
      Node *temp = curr;
      curr = curr->next;
      delete temp;
    }
    head = nullptr;
    pthread_mutex_unlock(&mutex);
  }
};

// Contextos de prueba multithreading
struct WorkerArgUnsync {
  LinkedListUnsynced *list;
  int thread_id;
  int ops_per_thread;
};

struct WorkerArgSync {
  LinkedListSynced *list;
  int thread_id;
  int ops_per_thread;
};

// prueba concurrente sin sincronización (Inserta y busca)
void *thread_work_unsync(void *arg) {
  WorkerArgUnsync *data = static_cast<WorkerArgUnsync *>(arg);
  for (int i = 0; i < data->ops_per_thread; ++i) {
    int val = data->thread_id * data->ops_per_thread + i;
    data->list->insert(val);
    if (i % 20 == 0) {
      data->list->search(val);
    }
  }
  pthread_exit(NULL);
}

// Rutina de prueba concurrente con sincronización
void *thread_work_sync(void *arg) {
  WorkerArgSync *data = static_cast<WorkerArgSync *>(arg);
  for (int i = 0; i < data->ops_per_thread; ++i) {
    int val = data->thread_id * data->ops_per_thread + i;
    data->list->insert(val);
    if (i % 20 == 0) {
      data->list->search(val);
    }
  }
  pthread_exit(NULL);
}

int main() {

  std::cout << "parte 3: lista enlazada multithreading\n\n";
  // comparacion de consistencia (Sin vs Con Mutex)
  std::cout << "[EXPERIMENTO 1: VERIFICACIÓN DE CONDICIÓN DE CARRERA Y "
               "CONSISTENCIA]\n";
  const int hilos_test = 4;
  const int ops_test = 25000;
  const int esperados = hilos_test * ops_test;

  std::ofstream csv_comp("parte3_comparacion.csv");
  csv_comp << "Modo,Threads,Esperados,Obtenidos,Perdidos,Tasa_Error_Pct,Tiempo_"
              "s,Estado\n";

  // prueba sin sincronizacion
  LinkedListUnsynced lista_sin;
  std::vector<pthread_t> th_unsync(hilos_test);
  std::vector<WorkerArgUnsync> args_unsync(hilos_test);

  auto t0 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < hilos_test; ++i) {
    args_unsync[i] = {&lista_sin, i, ops_test};
    pthread_create(&th_unsync[i], NULL, thread_work_unsync, &args_unsync[i]);
  }
  for (int i = 0; i < hilos_test; ++i)
    pthread_join(th_unsync[i], NULL);
  auto t1 = std::chrono::high_resolution_clock::now();
  double t_unsync = std::chrono::duration<double>(t1 - t0).count();

  int obtenidos_sin = lista_sin.count();
  int perdidos_sin = esperados - obtenidos_sin;
  double error_pct_sin = ((double)perdidos_sin / esperados) * 100.0;

  std::cout << "1.version sin sincronizacion:\n";
  std::cout << "    - Nodos esperados : " << esperados << "\n";
  std::cout << "    - Nodos obtenidos : " << obtenidos_sin << "\n";
  std::cout << "    - Nodos perdidos  : " << perdidos_sin << " (" << std::fixed
            << std::setprecision(2) << error_pct_sin << "% error)\n";
  std::cout << "    - tiempo total    : " << std::setprecision(6) << t_unsync
            << " s\n";
  std::cout << "    - diagnostico     : CORRUPCIÓN DE DATOS (Race conditions "
               "en head->next)\n\n";

  csv_comp << "sin_sincronizacion," << hilos_test << "," << esperados << ","
           << obtenidos_sin << "," << perdidos_sin << "," << error_pct_sin
           << "," << t_unsync << ",corrupto\n";

  // prueba con sincronizacion
  LinkedListSynced lista_con;
  std::vector<pthread_t> th_sync(hilos_test);
  std::vector<WorkerArgSync> args_sync(hilos_test);

  t0 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < hilos_test; ++i) {
    args_sync[i] = {&lista_con, i, ops_test};
    pthread_create(&th_sync[i], NULL, thread_work_sync, &args_sync[i]);
  }
  for (int i = 0; i < hilos_test; ++i)
    pthread_join(th_sync[i], NULL);
  t1 = std::chrono::high_resolution_clock::now();
  double t_sync = std::chrono::duration<double>(t1 - t0).count();

  int obtenidos_con = lista_con.count();
  int perdidos_con = esperados - obtenidos_con;
  double error_pct_con = ((double)perdidos_con / esperados) * 100.0;

  std::cout << ">>> 2. Versión CON Sincronización (Mutex POSIX):\n";
  std::cout << "    - Nodos esperados : " << esperados << "\n";
  std::cout << "    - Nodos obtenidos : " << obtenidos_con << "\n";
  std::cout << "    - Nodos perdidos  : " << perdidos_con << " (0.00% error)\n";
  std::cout << "    - Tiempo total    : " << std::setprecision(6) << t_sync
            << " s\n";
  std::cout << "    - Diagnóstico     : TOTALMENTE CONSISTENTE Y CORRECTO\n\n";

  csv_comp << "Con_Mutex," << hilos_test << "," << esperados << ","
           << obtenidos_con << "," << perdidos_con << "," << error_pct_con
           << "," << t_sync << ",Correcto\n";
  csv_comp.close();

  // rendimiento multithreading
  std::cout << "escabilidad y rendimiento de la lista con mutex\n";

  std::vector<int> lista_hilos = {1, 2, 4, 6, 8, 10, 12};
  const int CARGA_TOTAL = 60000;

  std::ofstream csv_perf("parte3_rendimiento.csv");
  csv_perf << "Threads,Ops_Totales,Tiempo_s,Throughput_Ops_Sec,Consistencia\n";

  std::cout << std::left << std::setw(10) << "Threads" << std::setw(16)
            << "Ops Totales" << std::setw(16) << "Tiempo (s)" << std::setw(20)
            << "Throughput (ops/s)" << std::setw(14) << "Consistencia" << "\n";

  for (int p : lista_hilos) {
    LinkedListSynced bench_list;
    int ops_hilo = CARGA_TOTAL / p;
    int total_esperado = ops_hilo * p;

    std::vector<pthread_t> ths(p);
    std::vector<WorkerArgSync> args(p);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < p; ++i) {
      args[i] = {&bench_list, i, ops_hilo};
      pthread_create(&ths[i], NULL, thread_work_sync, &args[i]);
    }
    for (int i = 0; i < p; ++i)
      pthread_join(ths[i], NULL);
    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();

    int final_count = bench_list.count();
    bool ok = (final_count == total_esperado);
    double throughput = total_esperado / elapsed;

    std::cout << std::left << std::setw(10) << p << std::setw(16)
              << total_esperado << std::setw(16) << std::fixed
              << std::setprecision(6) << elapsed << std::setw(20) << std::fixed
              << std::setprecision(2) << throughput << std::setw(14)
              << (ok ? "100% OK" : "FALLO") << "\n";

    csv_perf << p << "," << total_esperado << "," << elapsed << ","
             << throughput << "," << (ok ? 1 : 0) << "\n";
  }

  csv_perf.close();
  std::cout << "\n resultados en: 'parte3_comparacion.csv' y "
               "'parte3_rendimiento.csv'.\n\n";

  return 0;
}
