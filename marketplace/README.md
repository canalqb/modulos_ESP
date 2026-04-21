# Marketplace - CanalQB ESP32

Bem-vindo ao Marketplace de módulos para o CanalQB ESP32!

## Estrutura

Cada pasta neste diretório representa um módulo disponível para instalação:

```
marketplace/
  alexa/           # Integração com Amazon Alexa
  samsung/         # Integração com Samsung SmartThings
  scheduler/       # Agendador de tarefas
  whatsapp/        # Integração com WhatsApp
```

## Formato do Módulo

Cada módulo deve conter:
- `module.json` - Metadados e configuração do módulo
- Arquivos de implementação (.js, .json, etc.)

## Instalação

Os módulos são instalados através da interface "Loja de Módulos" no painel do ESP32.

## Desenvolvimento

Para adicionar novos módulos:
1. Crie uma nova pasta com o nome do módulo
2. Adicione o arquivo `module.json` com os metadados
3. Implemente os arquivos necessários
4. Teste a instalação através do marketplace

---
*CanalQb ESP32 Marketplace*
