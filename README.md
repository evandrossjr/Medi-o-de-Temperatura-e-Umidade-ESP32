# Projeto ESP32 - Medição de Temperatura e Umidade

Este projeto envia dados de temperatura e umidade lidos de um sensor DHT11 para um backend Java (Spring Boot) via Wi-Fi.

## Como usar

1. Clone o repositório
2. Copie o arquivo `secrets_template.h` como `secrets.h`
3. Preencha com seu SSID, senha e IP do backend
4. Compile e envie para o ESP32

## Endpoints esperados

- `POST /medicao/tempatual`
- `POST /medicao/historico`
