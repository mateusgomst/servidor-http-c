CC=gcc
CFLAGS=-Wall -std=c99 -g

# Define os diretórios
INCLUDE_DIR=include
SRC_DIR=src

# Adiciona a flag -I para que o compilador procure cabeçalhos no diretório 'include'
# Agora o CFLAGS fica mais completo
CFLAGS += -I$(INCLUDE_DIR)

# Nome do executável
TARGET=servidor

# Encontra todos os arquivos .c no diretório src
SOURCES=$(wildcard $(SRC_DIR)/*.c)
# Gera a lista de arquivos objeto (.o) correspondentes
OBJECTS=$(SOURCES:$(SRC_DIR)/%.c=%.o)

FILES_DIR=./files

# --- Regras ---

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS)

# Regra genérica para compilar .c de src/ para .o na raiz
%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

setup:
	mkdir -p $(FILES_DIR)
	echo "Arquivo de exemplo inicial." > $(FILES_DIR)/exemplo.txt

run: all setup
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJECTS)

clean-all: clean
	rm -rf $(FILES_DIR)