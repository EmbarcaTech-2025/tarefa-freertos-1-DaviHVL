
# Tarefa: Roteiro de FreeRTOS #1 - EmbarcaTech 2025

Autor: **Davi Henrique Vieira Lima**

Curso: Residência Tecnológica em Sistemas Embarcados

Instituição: EmbarcaTech - HBr

Brasília, Junho de 2025

---

# 🔁 Sistema Multitarefa com FreeRTOS

Projeto de controle multitarefa usando FreeRTOS com três tarefas concorrentes: LED RGB, buzzer e botões, executado na BitDogLab com Raspberry Pi Pico W.

---

## 🎯 Objetivo do projeto

- Controlar um LED RGB que alterna entre as cores vermelha, verde e azul a cada 500ms.
- Ativar um buzzer com bipes curtos a cada 1 segundo.
- Monitorar dois botões:
  - Botão A: pausa/retoma a tarefa do LED RGB.
  - Botão B: pausa/retoma a tarefa do buzzer.
- Utilizar as funções: `vTaskCreate()`, `vTaskSuspend()`, `vTaskResume()` e `vTaskDelay()`.

---

## 🧩 Componentes usados

- **Raspberry Pi Pico**
- **LED RGB**
- **Buzzer**
- **Dois Push-Buttons**

---

## 📂 Estrutura do Projeto
```text
tarefa-freertos-1-DaviHVL/
├── FreeRTOS/                        # Kernel do FreeRTOS clonado
│   └── ...                          # Arquivos internos do sistema operacional    
├── include/
│   └── FreeRTOSConfig.h             # Configurações do FreeRTOS para o projeto   
├── .gitignore                       # Ignora determinados Arquivos
├── main.c                           # Arquivo Principal
├── CMakeLists.txt                   # Configuração de Build
├── pico_sdk_import.cmake            # Configuração Automática do Ambiente de Compilação
└── README.md                        # README
```

---
## ⚡ Pinagem

| Componente  | Pino GPIO  |
|-------------|------------|
| LED Verde   | 11         |
| LED Azul    | 12         |
| LED Vermelho| 13         |
| Buzzer      | 21         |
| Botão A     | 5          |
| Botão B     | 6          |

---

## ⚙️ Como compilar e executar
Os comandos a seguir são necessários para gerar o .uf2:
```
  mkdir build
  cd build
  cmake ..
  make
```
Depois, copie o `.uf2` gerado para a Pico em modo bootloader.

Alternativamente, pode-se seguir os seguintes passos:
1. **Configuração inicial**  
   Certifique-se de ter o **Raspberry Pi Pico SDK** corretamente instalado no seu sistema, assim como o **CMake** e uma estrutura de projeto compatível.

2. **Clone ou copie os arquivos do projeto**

3. **Monte a estrutura do projeto**

4. **Aperte o ícone `Compile Project`** para compilar o projeto.

5. **Conecte o Pico via USB segurando o botão BOOTSEL** e aperte o ícone `Run Project (USB)` para gravar o projeto.

---

## 📈 Resultados esperados ou observados

Ao executar o código, espera-se o seguinte comportamento:

1.  **Inicialização do Sistema**:
    * Após a energização da BitDogLab, o sistema inicializa os periféricos: LED RGB, buzzer e botões.

    * As tarefas FreeRTOS são criadas e iniciadas com sucesso.

    * O LED RGB começa a alternar ciclicamente entre **vermelho, verde e azul** a cada 500 ms.

    * O buzzer emite **bipes curtos a cada 1 segundo**.


2.  **Suspensão e Retomada do LED RGB (Botão A)**:

    Ao pressionar o **Botão A**:
    * Se o LED RGB estiver alternando suas cores, a tarefa responsável pelo LED RGB é suspensa imediatamente e o LED RGB permenece fixo na sua última cor (imediatamente antes do presionamento do botão).
    * Se o LED RGB **NÃO** estiver alternando suas cores, a tarefa responsável pelo LED RGB é retomada imediatamente e, assim, o LED RGB volta a alternar ciclicamente suas cores.

3.  **Suspensão e Retomada do Buzzer (Botão B)**:

    Ao pressionar o **Botão B**:
    * Se o buzzer estiver apitando, a tarefa responsável pelo buzzer é suspensa imediatamente e, assim, o som do beep cessa completamente..
    * Se o buzzer **NÃO** não estiver apitando, a tarefa responsável pelo buzzer é retomada imediatamente e, assim, os beeps periódicos voltam a ser emitidos normalmente.

4.  **Execução Concorrente e Controle Responsivo**:

    * As tarefas funcionam de forma independente e concorrente.

    * A suspensão e retomada das tarefas são imediatas e sem afetar as demais execuções.

    * O sistema se mantém em funcionamento contínuo, respondendo aos comandos dos botões indefinidamente.

---

## 🔍 Reflexão 
### 1. **O que acontece se todas as tarefas tiverem a mesma prioridade?**

Quando todas as tarefas têm a mesma prioridade, o **escalonador** geralmente opera usando um mecanismo de **round-robin** (fatiamento de tempo). Dessa forma, todas tarefas terão **acesso equitativo à CPU**, sendo alternadas com base no tempo de fatia do sistema (time-slicing). Quando o tempo de uma tarefa se esgota, o escalonador preemptivamente a suspende e permite que a próxima tarefa de mesma prioridade na fila de "prontas para executar" comece a rodar.  **Para o usuário, isso cria a ilusão de que todas as tarefas estão executando "simultaneamente"**, embora na realidade elas estejam compartilhando o tempo do processador.

### 2. **Qual tarefa consome mais tempo da CPU?**
A tarefa que consome mais tempo da CPU é a `buttons_task()` em razão de:

* Ser executada com maior frequência 

* Apresentar mais lógica condicional para tratar os eventos dos botões

* Gerenciar as outras tarefas (suspend/resume)

* Nunca se suspensa

### 3. **Quais seriam os riscos de usar polling sem prioridades?**
Os riscos associados a isso são os seguintes:
* **Desperdício de Ciclos de CPU**: a tarefa de polling consome sua fatia de tempo da CPU verificando repetidamente uma condição (`gpio_get()`), mesmo que nada tenha mudado, gerando um maior consumo de energia e menor capacidade de processamento disponível para o resto do sistema.
* **Perda de Eventos Rápidos**: Se um evento ocorrer e desaparecer no intervalo entre duas verificações da tarefa de polling, o evento será permanentemente perdido, levando à falta de confiabilidade.
* **Alta Latência**: o tempo entre a ocorrência de um evento e a sua detecção será alta, ocasionando falta de responsividade.
* **"Starvation" de Tarefas Críticas**: Se há duas tarefas de mesma prioridade e uma tarefa crítica precisar de mais tempo de CPU do que a sua fatia permite para terminar um cálculo importante, ela será interrompida no meio para que a tarefa de polling possa rodar. Assim, a tarefa crítica pode nunca conseguir tempo suficiente para completar seu trabalho a tempo, um fenômeno chamado **starvation**.

---

## 📜 Licença
GNU GPL-3.0.
