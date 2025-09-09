# Servidor HTTP em C para Transferência de Arquivos

Este projeto consiste em implementar um **servidor HTTP em C** capaz de:

1. Servir uma **página HTML** com a listagem dos arquivos disponíveis em uma pasta do servidor.  
   - O usuário poderá acessar pelo navegador (browser) e visualizar os arquivos.

2. Permitir que o usuário selecione e **baixe os arquivos** pelo protocolo HTTP (usando apenas o browser, sem cliente dedicado).  

3. Tratar requisições HTTP básicas:  
   - **GET /** → Retorna a página HTML inicial.  
   - **GET /arquivo.ext** → Retorna o arquivo solicitado para download.  

## O que deve ser feito:

- Criar um socket TCP em C que escute na porta configurada (ex.: 8080).  
- Implementar o parser mínimo de requisições HTTP (método, caminho e versão).  
- Mapear a pasta de arquivos e gerar dinamicamente a página HTML listando os arquivos disponíveis.  
- Retornar os cabeçalhos HTTP corretos (ex.: `Content-Type`, `Content-Length`).  
- Enviar o conteúdo do arquivo solicitado quando o usuário clicar no link.  

## Observações

- O cliente será apenas o **navegador web**.  
- Não é necessário implementar POST, apenas GET.  
- O servidor deve ser capaz de servir múltiplos arquivos, um de cada vez.  

---
