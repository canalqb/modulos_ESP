# CanalQb ESP32 Hub - Firmware

## Descrição

Firmware completo para ESP32 com sistema dual-mode:

- **Fase 1 (Modo AP):** Portal de configuração Wi-Fi offline
- **Fase 2 (Modo STA):** Servidor Espelho (Streaming Proxy) do GitHub

## Características

### Modo AP (Configuração)
- SSID: `CanalQb-ESP32`
- IP: `192.168.4.1`
- Captive Portal automático
- Scanner de redes Wi-Fi
- Interface UTF-8 responsiva
- Modal de senha com toggle de visibilidade
- Terminal de log em tempo real
- Countdown de redirecionamento

### Modo STA (Operação)
- Streaming Proxy do GitHub
- Mantém IP local na barra de endereços
- Fallback automático para modo AP se perder conexão
- Suporte a Bluetooth (opcional)
- Configurações persistentes em LittleFS

## Instalação

### Pré-requisitos
- PlatformIO
- ESP32 (DOIT DevKit V1 ou similar)
- Cabo USB

### Passos

1. **Formatar Flash (OBRIGATÓRIO):**
   ```bash
   pio run --target erase
   ```

2. **Compilar e Gravar:**
   ```bash
   pio run --target upload
   ```

3. **Monitor Serial:**
   ```bash
   pio device monitor
   ```

## Configuração Inicial

1. Conecte-se ao Wi-Fi `CanalQb-ESP32`
2. Abra o navegador em `http://192.168.4.1`
3. Selecione sua rede Wi-Fi
4. Digite a senha (se necessário)
5. Aguarde a conexão e o redirecionamento

## Estrutura de Arquivos

```
firmware/
|
|-- src/
|   |-- main.cpp              # Firmware principal
|-- platformio.ini            # Configuração PlatformIO
|-- huge_app.csv              # Tabela de partições
|-- README.md                 # Este arquivo
```

## APIs

### Modo AP
- `GET /api/scan` - Lista redes Wi-Fi
- `GET /api/connect?ssid=X&pass=Y` - Conectar à rede
- `GET /api/restart` - Reiniciar ESP32

### Modo STA
- `GET /` - Streaming Proxy do GitHub
- `GET /*` - Proxy universal para arquivos estáticos

## Configurações Salvas

As configurações são salvas em `/config.json` no LittleFS:

```json
{
  "wifi": {
    "ssid": "sua_rede",
    "password": "sua_senha"
  },
  "network": {
    "hostname": "canalqb_esp32"
  },
  "bluetooth": {
    "enabled": true,
    "name": "CanalQb-ESP32",
    "pin": "1234"
  }
}
```

## Regras Importantes

- **NUNCA** redirecionar o navegador para URL externa
- **NUNCA** usar `alert()` ou `confirm()` - usar modais
- **SEMPRE** formatar flash antes de gravar
- Streaming Proxy mantém IP local na barra de endereços

## Troubleshooting

### Não conecta ao Wi-Fi
- Verifique a senha e o SSID
- Reinicie o ESP32 (desligue e ligue)
- Tente novamente o processo de configuração

### Página não carrega
- Verifique se está conectado ao `CanalQb-ESP32`
- Tente acessar `http://192.168.4.1` diretamente
- Limpe o cache do navegador

### Perda de conexão
- O ESP32 retorna automaticamente para modo AP
- Reconecte-se ao `CanalQb-ESP32`
- Reconfigure o Wi-Fi se necessário

## Licença

@CanalQb 2026 - Todos os direitos reservados
