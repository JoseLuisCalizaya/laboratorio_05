CXX = g++
CXXFLAGS = -O3 -pthread -Wall -Wextra -std=c++17
PYTHON = python3

BIN_PARTE1 = parte1_pi
BIN_PARTE2 = parte2_matvec
BIN_PARTE3 = parte3_lista
BIN_PARTE4 = parte4_barreras

TARGETS = $(BIN_PARTE1) $(BIN_PARTE2) $(BIN_PARTE3) $(BIN_PARTE4)

.PHONY: all clean clean-all run run1 run2 run3 run4 plot help

all: $(TARGETS)
	@echo " Compilación completada con éxito para todas las partes."
	@echo " Ejecute 'make run' para correr los experimentos."
	@echo " Ejecute 'make plot' para generar las gráficas."

$(BIN_PARTE1): parte1_pi.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN_PARTE2): parte2_matvec.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN_PARTE3): parte3_lista.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN_PARTE4): parte4_barreras.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

run: all
	@echo "\n>>> Ejecutando Parte I..."
	./$(BIN_PARTE1)
	@echo "\n>>> Ejecutando Parte II..."
	./$(BIN_PARTE2)
	@echo "\n>>> Ejecutando Parte III..."
	./$(BIN_PARTE3)
	@echo "\n>>> Ejecutando Parte IV..."
	./$(BIN_PARTE4)
	@echo " Ahora puede ejecutar 'make plot' para generar las gráficas."

# Ejecuciones individuales
run1: $(BIN_PARTE1)
	./$(BIN_PARTE1)

run2: $(BIN_PARTE2)
	./$(BIN_PARTE2)

run3: $(BIN_PARTE3)
	./$(BIN_PARTE3)

run4: $(BIN_PARTE4)
	./$(BIN_PARTE4)

# Generación de gráficas vía Python
plot:
	$(PYTHON) generar_graficas.py

# Limpieza de ejecutables y archivos temporales
clean:
	rm -f $(TARGETS) *.o a.out

# Limpieza total incluyendo CSVs y gráficas
clean-all: clean
	rm -f *.csv
	rm -rf graficas/

# Ayuda
help:
	@echo "Opciones disponibles:"
	@echo "  make          - Compila todos los programas (Partes I, II, III y IV)"
	@echo "  make run      - Compila y ejecuta todos los experimentos generando los CSV"
	@echo "  make plot     - Genera las gráficas a partir de los archivos CSV"
	@echo "  make run1     - Ejecuta únicamente la Parte I (Cálculo de Pi)"
	@echo "  make run2     - Ejecuta únicamente la Parte II (Matriz-Vector)"
	@echo "  make run3     - Ejecuta únicamente la Parte III (Lista Enlazada)"
	@echo "  make run4     - Ejecuta únicamente la Parte IV (Barreras)"
	@echo "  make clean    - Elimina ejecutables y archivos objeto"
	@echo "  make clean-all- Elimina ejecutables, CSVs y la carpeta graficas/"
