#include <atomic>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>

// 1. BARRERA CON BUSY-WAITING Y MUTEX
class BarreraBusyWait {
private:
  int count;
  int thread_count;
  pthread_mutex_t mutex;

public:
  void init(int num_threads) {
    this->count = 0;
    this->thread_count = num_threads;
    pthread_mutex_init(&mutex, NULL);
  }

  void wait() {
    pthread_mutex_lock(&mutex);
    count++;
    pthread_mutex_unlock(&mutex);

    while (count < thread_count) {
#if defined(__x86_64__) || defined(_M_X64)
      __builtin_ia32_pause();
#endif
    }
  }

  void destroy() { pthread_mutex_destroy(&mutex); }
};

// 2. BARRERA CON SEMAFOROS POSIX
class BarreraSemaforo {
private:
  int count;
  int thread_count;
  pthread_mutex_t mutex;
  sem_t sem1;
  sem_t sem2;

public:
  void init(int num_threads) {
    this->count = 0;
    this->thread_count = num_threads;
    pthread_mutex_init(&mutex, NULL);
    sem_init(&sem1, 0, 0);
    sem_init(&sem2, 0, 1);
  }

  void wait() {
    pthread_mutex_lock(&mutex);
    count++;
    if (count == thread_count) {
      sem_wait(&sem2);
      sem_post(&sem1);
    }
    pthread_mutex_unlock(&mutex);

    sem_wait(&sem1);
    sem_post(&sem1);

    pthread_mutex_lock(&mutex);
    count--;
    if (count == 0) {
      sem_wait(&sem1);
      sem_post(&sem2);
    }
    pthread_mutex_unlock(&mutex);

    sem_wait(&sem2);
    sem_post(&sem2);
  }

  void destroy() {
    pthread_mutex_destroy(&mutex);
    sem_destroy(&sem1);
    sem_destroy(&sem2);
  }
};

// 3. BARRERA CON VARIABLES DE CONDICION
class BarreraCondVar {
private:
  int count;
  int thread_count;
  pthread_mutex_t mutex;
  pthread_cond_t cond;

public:
  void init(int num_threads) {
    this->count = 0;
    this->thread_count = num_threads;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
  }

  void wait() {
    pthread_mutex_lock(&mutex);
    count++;
    if (count == thread_count) {
      count = 0;
      pthread_cond_broadcast(&cond);
    } else {
      while (pthread_cond_wait(&cond, &mutex) != 0)
        ;
    }
    pthread_mutex_unlock(&mutex);
  }

  void destroy() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
  }
};

// MODULO DE VERIFICACION DE FASES
const int DEMO_FASES = 3;
const int DEMO_HILOS = 4;
std::atomic<int> hilos_en_fase[DEMO_FASES];
std::atomic<bool> violacion_detectada(false);

BarreraCondVar barrera_verif;

void *hilo_verificacion(void *arg) {
  long id = (long)arg;
  for (int fase = 0; fase < DEMO_FASES; ++fase) {
    if (fase > 0 && hilos_en_fase[fase - 1].load() != DEMO_HILOS) {
      violacion_detectada = true;
    }

    hilos_en_fase[fase]++;

    volatile double dummy = 0.0;
    for (int i = 0; i < (id + 1) * 30000; ++i)
      dummy = dummy + i * 0.0001;

    std::cout << "  [Hilo " << id << "] Llego a la barrera de la Fase "
              << (fase + 1)
              << " (Acumulados en fase: " << hilos_en_fase[fase].load()
              << ")\n";

    barrera_verif.wait();

    std::cout << "  ===> [Hilo " << id << "] en la barrera hacia Fase "
              << (fase + 2) << "\n";
  }
  pthread_exit(NULL);
}

void probar_verificacion_fases() {
  std::cout << "verificacion de fase\n";
  std::cout << "Iniciando prueba con " << DEMO_HILOS << " hilos y "
            << DEMO_FASES << " fases...\n";

  for (int f = 0; f < DEMO_FASES; ++f)
    hilos_en_fase[f] = 0;
  violacion_detectada = false;

  barrera_verif.init(DEMO_HILOS);
  std::vector<pthread_t> hilos(DEMO_HILOS);

  for (long i = 0; i < DEMO_HILOS; ++i) {
    pthread_create(&hilos[i], NULL, hilo_verificacion, (void *)i);
  }
  for (int i = 0; i < DEMO_HILOS; ++i) {
    pthread_join(hilos[i], NULL);
  }
  barrera_verif.destroy();

  std::cout << "\nresultado de verificacion de fases: "
            << (violacion_detectada
                    ? "ERROR: Violacion de orden detectada!"
                    : "EXITOSO (Ningún hilo adelanto fase sin los demas)")
            << "\n\n";
}

// BENCHMARK COMPARATIVO DE LAS 3 BARRERAS
const int BENCH_FASES = 25;
const int COMPUTO_POR_FASE = 40000;

struct BarrierBenchContext {
  int num_threads;
  int tipo_barrera; // 0: BusyWait, 1: Semaforo, 2: CondVar
  std::vector<BarreraBusyWait> *bw_barreras;
  BarreraSemaforo *sem_barrera;
  BarreraCondVar *cv_barrera;
};

void *worker_barrier_bench(void *arg) {
  BarrierBenchContext *ctx = static_cast<BarrierBenchContext *>(arg);

  for (int f = 0; f < BENCH_FASES; ++f) {
    // Carga computacional uniforme por fase
    volatile double sum = 0.0;
    for (int i = 0; i < COMPUTO_POR_FASE; ++i) {
      sum = sum + i * 0.0001;
    }

    // Sincronización en la barrera correspondiente
    if (ctx->tipo_barrera == 0) {
      (*ctx->bw_barreras)[f].wait();
    } else if (ctx->tipo_barrera == 1) {
      ctx->sem_barrera->wait();
    } else {
      ctx->cv_barrera->wait();
    }
  }
  pthread_exit(NULL);
}

double medir_barrera(int tipo, int num_threads) {
  std::vector<BarreraBusyWait> bw_list;
  BarreraSemaforo sem_b;
  BarreraCondVar cv_b;

  if (tipo == 0) {
    bw_list.resize(BENCH_FASES);
    for (int f = 0; f < BENCH_FASES; ++f)
      bw_list[f].init(num_threads);
  } else if (tipo == 1) {
    sem_b.init(num_threads);
  } else {
    cv_b.init(num_threads);
  }

  BarrierBenchContext ctx;
  ctx.num_threads = num_threads;
  ctx.tipo_barrera = tipo;
  ctx.bw_barreras = &bw_list;
  ctx.sem_barrera = &sem_b;
  ctx.cv_barrera = &cv_b;

  std::vector<pthread_t> hilos(num_threads);
  auto t_start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < num_threads; ++i) {
    pthread_create(&hilos[i], NULL, worker_barrier_bench, &ctx);
  }
  for (int i = 0; i < num_threads; ++i) {
    pthread_join(hilos[i], NULL);
  }

  auto t_end = std::chrono::high_resolution_clock::now();
  double elapsed = std::chrono::duration<double>(t_end - t_start).count();

  if (tipo == 0) {
    for (int f = 0; f < BENCH_FASES; ++f)
      bw_list[f].destroy();
  } else if (tipo == 1) {
    sem_b.destroy();
  } else {
    cv_b.destroy();
  }

  return elapsed;
}

int main() {
  std::cout << "parte 4: comparacion de las 3 barreras\n";

  probar_verificacion_fases();

  // experimentos de tiempo y sincronizacion
  std::vector<int> lista_hilos = {1, 2, 4, 6, 8, 10, 12};

  std::cout << " BENCHMARK: " << BENCH_FASES
            << " Fases con sincronización repetida\n";

  std::ofstream csv("parte4_resultados.csv");
  csv << "Threads,Fases,BusyWait_Time,Semaphore_Time,CondVar_Time\n";

  std::cout << std::left << std::setw(10) << "Threads" << std::setw(18)
            << "Busy-Wait (s)" << std::setw(18) << "Semaforo (s)"
            << std::setw(18) << "Cond-Var (s)" << std::setw(20)
            << "Mas Eficiente" << "\n";

  for (int p : lista_hilos) {
    double t_busy = medir_barrera(0, p);
    double t_sem = medir_barrera(1, p);
    double t_cv = medir_barrera(2, p);

    std::string mejor = "Cond-Var";
    if (t_busy <= t_sem && t_busy <= t_cv)
      mejor = "Busy-Wait";
    else if (t_sem <= t_busy && t_sem <= t_cv)
      mejor = "Semaforo";

    std::cout << std::left << std::setw(10) << p << std::setw(18) << std::fixed
              << std::setprecision(6) << t_busy << std::setw(18) << std::fixed
              << std::setprecision(6) << t_sem << std::setw(18) << std::fixed
              << std::setprecision(6) << t_cv << std::setw(20) << mejor << "\n";

    csv << p << "," << BENCH_FASES << "," << t_busy << "," << t_sem << ","
        << t_cv << "\n";
  }

  csv.close();
  std::cout << "\nresultados exportados a 'parte4_resultados.csv'.\n\n";

  return 0;
}
