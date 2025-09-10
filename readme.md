
# Servidor HTTP em C para Transferência de Arquivos

## Visão Geral

Este projeto é uma implementação de um servidor HTTP simples, desenvolvido em Linguagem C, utilizando a API de Sockets POSIX. O servidor é capaz de atender requisições GET para listar e servir arquivos contidos em um diretório local, permitindo que usuários visualizem e baixem esses arquivos diretamente pelo navegador.

O objetivo principal é demonstrar os conceitos fundamentais de programação de rede, manipulação do protocolo HTTP e organização de um projeto C modular.

## Principais Funcionalidades

  - **Listagem dinâmica de arquivos**: Gera uma página HTML em tempo real com links para todos os arquivos disponíveis no diretório `files/`.
  - **Download de arquivos**: Permite que qualquer arquivo listado seja baixado pelo cliente através de uma requisição GET.
  - **Roteamento de requisições GET**:
      - `GET /`: Retorna a página HTML com a lista de arquivos.
      - `GET /nome-do-arquivo.ext`: Inicia o download do arquivo especificado.
  - **Cabeçalhos HTTP**: Envia cabeçalhos HTTP essenciais, como `Content-Type`, `Content-Length` e `Content-Disposition`, para garantir o comportamento correto no navegador.

## Arquitetura do Projeto

Todo o código-fonte está contido no diretório `src/` para manter a organização e separação clara entre código e outros arquivos do projeto. A arquitetura é modular, dividindo as responsabilidades em diferentes arquivos.

  - `src/`
      - **`server.c`**: Ponto de entrada da aplicação (`main`). É responsável por inicializar o socket, realizar o `bind` na porta 8080, colocar o servidor em modo de escuta (`listen`) e gerenciar o loop principal que aceita novas conexões (`accept`).
      - **`http_handler.h` / `http_handler.c`**: Módulo responsável por tratar uma conexão de cliente. Ele lê a requisição HTTP, extrai o método e o caminho, e atua como um roteador, decidindo qual ação tomar com base na requisição recebida.
      - **`file_handler.h` / `file_handler.c`**: Contém a lógica de negócio para manipulação de arquivos. Suas funções são responsáveis por gerar a página HTML, servir um arquivo para download e processar uploads de arquivos.

## Como Funciona

O fluxo de operação do servidor segue os seguintes passos:

1.  **Inicialização**: O `server.c` cria um socket TCP, o associa à porta 8080 e o prepara para receber até 10 conexões pendentes.
2.  **Loop Principal**: O servidor entra em um loop infinito, aguardando por novas conexões de clientes com a chamada bloqueante `accept()`.
3.  **Tratamento da Conexão**: Quando um cliente (navegador) se conecta, a função `handle_connection` de `http_handler.c` é chamada.
4.  **Parsing da Requisição**: A requisição HTTP enviada pelo navegador é lida e seu método (GET) e caminho (`/` ou `/arquivo.ext`) são extraídos.
5.  **Roteamento**:
      - Se o caminho for `/`, a função `send_file_list_page` de `file_handler.c` é chamada para gerar e enviar a página HTML.
      - Se o caminho for para um arquivo específico, a função `send_file_download` de `file_handler.c` é chamada para enviar os cabeçalhos apropriados e o conteúdo do arquivo.
6.  **Resposta e Fechamento**: Após o envio da resposta completa, a conexão com o cliente é fechada, e o servidor volta a aguardar por novas conexões.

## Como Executar

Para compilar e executar o projeto, você precisará de um compilador C (como GCC ou Clang) e do utilitário `make`.

### Pré-requisitos

  - GCC ou Clang
  - Make

### Passos

1.  **Clone o repositório:**

    ```bash
    git clone <URL_DO_SEU_REPOSITORIO>
    cd <NOME_DA_PASTA_DO_PROJETO>
    ```

2.  **Compile o projeto:**
    O `Makefile` já está configurado para encontrar os arquivos na pasta `src`. Execute o comando:

    ```bash
    make
    ```

    Isso irá compilar todos os arquivos `.c` e gerar um executável chamado `servidor` na raiz do projeto.

3.  **Execute o servidor:**
    Para iniciar o servidor e criar automaticamente o diretório `files/` com um arquivo de exemplo, use o comando:

    ```bash
    make run
    ```

    Você verá a mensagem `Servidor rodando em http://localhost:8080` no terminal.

4.  **Acesse pelo navegador:**
    Abra seu navegador de internet e acesse o endereço:

    ```
    http://localhost:8080
    ```

    Você verá a página de boas-vindas com a lista de arquivos disponíveis para download.

### Comandos Adicionais

  - **Para limpar os arquivos de compilação (arquivos `.o` e o executável):**
    ```bash
    make clean
    ```
  - **Para limpar tudo, incluindo o diretório `files/`:**
    ```bash
    make clean-all
    ```