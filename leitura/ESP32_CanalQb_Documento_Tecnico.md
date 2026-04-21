# CanalQb ESP32 — Documento Técnico de Referência
**Versão:** Final | **Canal:** canalqb.com.br | **GitHub:** github.com/canalqb/modulos_ESP

---

## ⚠️ REGRAS ABSOLUTAS (Nunca ignorar)

1. **SEMPRE formate (Erase Flash) antes de gravar qualquer firmware.**
2. **NUNCA altere o que está funcionando.** Ao adicionar uma feature nova, isole a mudança.
3. **NUNCA redirecione o navegador para URL externa.** O ESP32 deve agir como Servidor Espelho (Streaming Proxy).
4. **NUNCA use `alert()` ou `confirm()`.** Sempre usar modais customizados e `showToast()`.
5. **Todos os ícones devem ser `fas` (Font Awesome Solid).** Nunca `bi-`, `fa-`, ou emoji inline.
6. Sequência obrigatória de gravação: `pio run --target erase` → `pio run --target upload`.

---

## 🏗️ Arquitetura Geral

### Plataforma
- **Hardware:** ESP32 (DOIT DevKit V1 ou similar)
- **Framework:** Arduino (PlatformIO)
- **Linguagem:** C/C++
- **Armazenamento:** LittleFS (sistema de arquivos) + NVS Preferences (configurações rápidas)
- **Config persistente:** `/config.json` no LittleFS
- **Partição:** `huge_app.csv` (3MB para o código — necessário por causa do Bluetooth)

### Repositório GitHub
- **URL:** `https://github.com/canalqb/modulos_ESP`
- **Pasta do ambiente web:** `/ambiente/index.html`
- **Loja de módulos:** `/market.json`
- **Pasta de módulos:** `/whatsapp/`, `/telegram/`, `/scheduler/`, `/alexa/`, `/samsung/`

---

## 📡 FASE 1 — Modo AP (Access Point) — Configuração Inicial

### Condição de Ativação
O ESP32 entra em modo AP quando:
- Não existe SSID salvo no `config.json`, **OU**
- O SSID salvo falha na conexão após 30 segundos de tentativa.

### Configurações do AP
```
SSID: CanalQb-ESP32
IP:   192.168.4.1
DHCP: ativo
Hostname: canalqb_esp32
```

### Captive Portal
O servidor DNS interno deve interceptar **qualquer** domínio e redirecionar para `192.168.4.1`. Handlers obrigatórios para garantir abertura automática no Android e Windows:
- `GET /generate_204` → 302 para `http://canalqb.com.br`
- `GET /connecttest.txt` → texto `Microsoft Connect Test`
- `GET /ncsi.txt` → texto `Microsoft NCSI`
- `GET /hotspot-detect.html` → redirect
- `GET /redirect` → redirect
- `onNotFound` → 302 para `http://192.168.4.1`
- Se o Host da requisição for `canalqb.com.br`, servir o HTML de setup local diretamente (sem mudar URL no navegador)

### Interface Local (Embutida no Firmware — PROGMEM)
A interface de setup é **100% offline**, embutida no firmware, sem dependência externa.

**Componentes obrigatórios da tela:**
1. Logo/header CanalQb
2. Lista de SSIDs (altura 10 linhas com scroll, atualização automática)
3. Modal de senha (ao clicar em uma rede)
4. Terminal/log em tempo real (caixa preta, texto verde)
5. Modal de sucesso (IP + MAC)

**Comportamento do Scanner:**
- Iniciar scan assíncrono automaticamente 1 segundo após carregar a página
- API `GET /api/scan` retorna JSON com lista de redes
- Se scan ainda em andamento, retornar HTTP 202
- Botão de "Atualizar Redes" para novo scan manual
- SSIDs devem ser completos, sem truncamento
- Exibir ícone de sinal e cadeado (rede protegida)
- Tempo por canal: 110ms

**Modal de senha:**
- Botão "Conectar" + botão "Cancelar"
- Tecla `Enter` aciona conexão
- Input com suporte UTF-8 completo

**Fluxo de conexão (via AJAX — sem recarregar página):**
1. Usuário clica na rede → abre modal de senha
2. Usuário digita senha → clica Conectar (ou Enter)
3. Terminal começa a mostrar log em tempo real
4. API `GET /api/connect?ssid=X&pass=Y` — ESP32 tenta conectar
5. **Se senha errada:** modal fecha, terminal mostra erro, usuário pode tentar novamente
6. **Se conectar:** retorna JSON `{"ip":"192.168.0.22","mac":"AA:BB:CC:DD:EE:FF","gateway":"192.168.0.1"}`

**Sequência obrigatória após sucesso (ORDEM INEGOCIÁVEL):**
```
1. Modal de sucesso abre com IP e MAC → PERMANECE ABERTO
2. Cronômetro de 10 segundos de redirecionamento inicia IMEDIATAMENTE
3. Após 1 segundo (não antes): dispara GET /api/restart
4. ESP32 aguarda 3 segundos e reinicia + oculta AP
5. Navegador redireciona para o IP (ex: http://192.168.0.22)
```
> ⚠️ O modal de sucesso NUNCA fecha automaticamente. Ele fica visível até o browser redirecionar.

**Endpoint /api/restart:**
- Aguarda 1s interno antes de chamar `WiFi.softAPdisconnect(true)`
- Depois `WiFi.mode(WIFI_STA)`
- Depois espera 3s e `ESP.restart()`

---

## 🌐 FASE 2 — Modo Station (STA) — Operação Normal

### Condição de Ativação
ESP32 reinicia com SSID válido salvo → conecta ao Wi-Fi do usuário → modo STA ativo.

### Comportamento do AP após reconexão
- Se conectar em menos de 30s → `WiFi.mode(WIFI_STA)` — AP desaparece completamente
- Se não conectar em 30s → AP permanece ativo para o usuário reconfigurar
- Se conectar após os 30s → AP é ocultado na hora

### Servidor Espelho (Streaming Proxy — V15)
**Princípio fundamental:** Quando o usuário acessa `http://192.168.0.22`, a barra de endereços NÃO muda. O ESP32 busca o conteúdo do GitHub e entrega ao navegador pedaço por pedaço.

```
Usuario digita: http://192.168.0.22
ESP32 faz:      GET https://raw.githack.com/canalqb/modulos_ESP/main/ambiente/index.html?ip=192.168.0.22
ESP32 entrega:  HTML stream para o navegador
Browser vê:     http://192.168.0.22 (URL não muda)
```

**Implementação do Streaming Proxy:**
```cpp
// handleRoot — Fase 2
WiFiClientSecure client;
client.setInsecure();
HTTPClient http;
String url = "https://raw.githack.com/canalqb/modulos_ESP/main/ambiente/index.html?ip=" + WiFi.localIP().toString();
http.begin(client, url);
http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
http.setTimeout(10000);
int code = http.GET();
if (code == 200) {
  WiFiClient* stream = http.getStreamPtr();
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html; charset=utf-8", "");
  uint8_t buf[4096];
  int len;
  while ((len = stream->readBytes(buf, sizeof(buf))) > 0) {
    server.sendContent_P((char*)buf, len);
  }
} else {
  // Fallback: servir página de setup local
  server.send(200, "text/html; charset=utf-8", local_setup_html);
}
http.end();
```

**Proxy Universal (onNotFound):**
Qualquer arquivo solicitado ao IP local que não seja uma rota de API deve ser buscado na pasta `/ambiente/` do GitHub com MIME type correto:
```
/style.css    → text/css
/script.js    → application/javascript
/logo.png     → image/png
/favicon.ico  → image/x-icon (servir /ambiente/logo.ico do GitHub)
```

---

## 🔌 APIs Internas do ESP32

| Endpoint | Método | Descrição |
|---|---|---|
| `/api/scan` | GET | Lista redes Wi-Fi. 202 se ainda escaneando |
| `/api/connect` | GET | Params: `ssid`, `pass`. Retorna JSON com IP/MAC ou erro |
| `/api/status` | GET | `{"connected":true,"ip":"...","mac":"..."}` |
| `/api/restart` | GET | Reinicia o ESP32 (com delay de 3s) |
| `/api/save` | POST | Salva configurações (Google, IA, Bluetooth) |
| `/api/config` | GET | Retorna config atual (sem senhas) |
| `/api/test-drive` | GET | Cria pasta e arquivo de teste no Google Drive |
| `/api/chat` | POST | Chat com IA. Body: `{"prompt":"...","user":"..."}` |
| `/webhook` | POST | Webhook externo para acionar a IA |
| `/whatsapp` | GET | Página de configuração do WhatsApp Bot |
| `/telegram` | GET | Página de configuração do Telegram Bot |
| `/favicon.ico` | GET | Serve logo.ico via proxy do GitHub |

---

## ☁️ Integração Google Drive

### Configurações necessárias (salvas no ESP32)
```json
{
  "google": {
    "client_id": "",
    "client_secret": "",
    "refresh_token": "",
    "folder_id": "",
    "service_email": ""
  }
}
```

### Instruções para o usuário (mostrar na interface)
- Tipo de credencial: **App para computador** (OAuth 2.0)
- URI de redirecionamento: `http://localhost:3000/auth/callback`
- Adicionar email como **Usuário de Teste** no Google Cloud Console
- Habilitar API do Google Drive no projeto

### Fluxo OAuth
- Geração do Refresh Token feita via JavaScript no próprio browser (sem script Python externo)
- A interface abre nova aba para autenticação Google
- Após autenticação, copia o código e cola no campo da interface
- ESP32 troca o código pelo Refresh Token via API do Google
- Salva o Refresh Token no `config.json`

### Estrutura de pastas no Drive
```
[pasta raiz configurada pelo usuário]
└── sistema_opus/
    ├── core/
    ├── plugins/
    │   ├── whatsapp/
    │   ├── telegram/
    │   └── scheduler/
    ├── configs/
    ├── models/
    └── manifest.json
```

> ⚠️ **Anti-duplicidade:** Antes de criar qualquer pasta ou arquivo, verificar se já existe no Drive com a API Files.list. Se existir, reutilizar o ID.

### Teste de integração
Botão "Validar Drive" → cria arquivo `TESTE_CONEXAO.txt` na pasta `sistema_opus`. Se der erro 403, verificar permissões e exibir resposta JSON do Google na interface.

---

## 🤖 Integração com IA (Anthropic Claude)

### Configuração de chaves
- Campo aceita **múltiplas chaves separadas por `;`**
- Sistema de fallback automático: se a primeira chave der erro de cota, tenta a próxima
- Pequeno link "Onde pegar as chaves?" → `https://platform.claude.com/settings/keys`

### Modelo preferido
`claude-3-haiku-20240307` (mais leve para IoT)

### Gerenciamento de contexto
- Histórico mantido em RAM (últimas 6 mensagens por IP/usuário)
- Identificação por IP + username da máquina (`user` no body do request)
- Upload do histórico completo para o Drive a cada interação
- Arquivo salvo como: `historico_192.168.1.x_nomeusuario.json`

### Integração OpenRouter (opcional)
- Checkbox para ativar na interface
- Campo para API key do OpenRouter
- Link: `https://openrouter.ai/`
- Permite usar modelos adicionais (Gemini, GPT, etc.)

---

## 📶 Bluetooth

### Configurações
```json
{
  "bluetooth": {
    "enabled": true,
    "name": "CanalQb-ESP32",
    "pin": "1234"
  }
}
```

### Comportamento
- Bluetooth ativado após primeira configuração de SSID
- Interface permite ativar/desativar, trocar nome e PIN
- Usa `BluetoothSerial` (requer partição `huge_app`)
- Não deve interferir no Wi-Fi

---

## 🛒 Loja de Módulos

### Fonte de dados
`https://raw.githubusercontent.com/canalqb/modulos_ESP/main/market.json`

### Formato do market.json
```json
{
  "modules": [
    {
      "slug": "whatsapp",
      "name": "WhatsApp Bridge v1.5",
      "desc": "Ponte Node.js com persistência na nuvem.",
      "url": "https://raw.githubusercontent.com/canalqb/modulos_ESP/main/whatsapp/plugin.js"
    }
  ]
}
```

### Instalação de módulo
1. ESP32 consulta `market.json` do GitHub
2. Usuário tira/coloca o checkbox do módulo desejado
3. Clica em "Instalar"
4. ESP32 baixa o arquivo do GitHub e salva na pasta `plugins/[slug]/` do Drive
5. Atualiza `manifest.json` no Drive

### Módulos disponíveis
- WhatsApp Bridge v1.5
- Telegram Core Bot
- Agendador de Rotinas (mensagens via chat: "agende para amanhã às 14h...")
- Alexa Link Pro
- Samsung TV Control (UPnP/DLNA)
- Auto-Updater (padrão, sempre instalado)

---

## 🌐 Interface Web (Ambiente GitHub)

### Localização
`https://github.com/canalqb/modulos_ESP/ambiente/index.html`

### Tecnologias
- Bootstrap 5
- Font Awesome 6.5 (apenas `fas`)
- JavaScript vanilla

### Estrutura de menu (sidebar estilo LuCI/OpenWrt)
```
CanalQb - ESP32
├── Visão Geral (dashboard)
├── Redes & Internet
│   ├── Configuração Wi-Fi (scanner + troca de SSID)
│   └── Configurações de Rede (IP fixo/dinâmico, hostname)
├── Acesso à Nuvem (Google Drive)
│   ├── Credenciais OAuth
│   ├── ID da Pasta
│   └── [Botão] Validar Drive
├── Configurações de IA
│   ├── Chaves Anthropic (múltiplas, separadas por ;)
│   └── Integração OpenRouter
├── Bluetooth
│   ├── Ativar/Desativar (toggle)
│   ├── Nome do dispositivo
│   └── PIN (padrão: 1234)
├── 🔒 Módulos (bloqueado até Drive validado)
├── 🔒 Chat (bloqueado até Drive validado)
├── Sobre / Apoiar
│   ├── Link: canalqb.com.br
│   ├── Pix: qrodrigob@gmail.com
│   └── PayPal: qrodrigob@gmail.com
└── ⚠️ Reset de Fábrica
```

### Regras de desbloqueio de menus
- "Módulos" e "Chat" ficam com cadeado 🔒 até o teste do Drive ser bem-sucedido
- Após validar o Drive, os menus são desbloqueados sem recarregar a página

### Tom e linguagem
- Tranquilo, amigável, técnico mas acessível
- Sem termos "industriais" ou excessivamente corporativos
- Sempre fazer referência ao CanalQb de forma natural

### Doação
```html
<div class="card">
  <h6>Apoiar o CanalQb</h6>
  <p>Se este projeto te ajudou, considere apoiar.</p>
  <button onclick="navigator.clipboard.writeText('qrodrigob@gmail.com')">
    <i class="fas fa-key"></i> Copiar Pix
  </button>
  <a href="https://paypal.me/qrodrigob">
    <i class="fas fa-credit-card"></i> PayPal
  </a>
</div>
```

---

## 🔒 Reset de Fábrica

### Localização
Sempre visível no menu (não pode ser removido ou ocultado).

### Comportamento
1. Usuário clica → abre modal customizado (sem `confirm()` nativo)
2. Modal mostra aviso grave com botão "Cancelar" e "Confirmar Reset"
3. Após confirmar: POST para `/api/factory-reset`
4. ESP32 executa:
   - `Preferences prefs; prefs.clear();`
   - Formata LittleFS
   - `ESP.restart()`
5. Dispositivo retorna ao estado de fábrica (Fase 1)

---

## 📁 Estrutura de Arquivos do Projeto

```
ClaudeESP32/
├── src/
│   └── main.cpp                  # Firmware principal
├── modulos_ESP_repo/
│   └── ambiente/
│       ├── index.html            # Interface web (servida via proxy)
│       └── logo.ico              # Favicon
├── opus_bridge_whatsapp/
│   ├── bridge.js                 # Ponte Node.js para WhatsApp
│   └── package.json
├── documentacao/
│   ├── api_reference.md          # Documentação completa da API
│   └── whatsapp_integration.md   # Guia de integração WhatsApp
├── regras/
│   └── master_rules.md           # Regras de design CanalQb
├── platformio.ini
└── protocolo_industrial.md       # Este documento
```

---

## 🖥️ Acesso Externo (sem intranet)

### API REST
```bash
# Enviar mensagem para a IA
curl -X POST http://192.168.0.22/api/chat \
  -H "Content-Type: application/json" \
  -d '{"prompt":"Olá Claude!","user":"meuusuario"}'

# Webhook externo
curl -X POST http://192.168.0.22/webhook \
  -H "Content-Type: application/json" \
  -d '{"prompt":"Automação via webhook","user":"n8n"}'
```

### Terminal SSH (script Python)
`python opus_ssh_terminal.py` — simula um terminal SSH conectando ao ESP32 via API.

---

## ⚙️ platformio.ini

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
board_build.partitions = huge_app.csv
lib_deps =
  ArduinoJson@^7
  ESPAsyncWebServer
  DNSServer
  BluetoothSerial
```

---

## 🔗 Links de Referência

- Blog: https://canalqb.com.br
- GitHub: https://github.com/canalqb/modulos_ESP
- Ambiente web: https://raw.githack.com/canalqb/modulos_ESP/main/ambiente/index.html
- Google Cloud: https://console.cloud.google.com
- Chaves Anthropic: https://platform.claude.com/settings/keys
- OpenRouter: https://openrouter.ai
