# PROMPT DE EXECUÇÃO — Firmware ESP32 CanalQb

> Cole este prompt inteiro em qualquer LLM com capacidade de gerar código C++ e HTML.
> Ele é autocontido e descreve tudo que precisa ser implementado.

---

## 🎯 Contexto e Objetivo

Você é um especialista em firmware ESP32 e desenvolvimento web embarcado. Sua tarefa é implementar o **CanalQb ESP32 Hub** — um firmware completo para ESP32 que funciona em dois modos distintos:

- **Fase 1 (Modo AP):** Portal de configuração Wi-Fi offline, embutido no firmware
- **Fase 2 (Modo STA):** Servidor Espelho (Streaming Proxy) que serve a interface web hospedada no GitHub diretamente ao navegador do usuário, sem redirecionar a URL

O projeto usa **PlatformIO + Arduino framework**. O arquivo principal é `src/main.cpp`.

---

## ⚠️ REGRAS ABSOLUTAS — Nunca viole estas regras

1. **NUNCA redirecione o navegador para URL externa.** A função `handleRoot` deve fazer streaming (proxy) do conteúdo do GitHub e entregar ao browser mantendo o IP local na barra de endereços.
2. **NUNCA use `alert()` ou `confirm()` nativos.** Toda interação usa modais HTML customizados.
3. **NUNCA altere funcionalidade que foi descrita como "funcionando".** Adicione código novo de forma isolada.
4. **SEMPRE formate o flash antes de gravar** (`pio run --target erase` antes de `pio run --target upload`).
5. **O modal de sucesso (IP/MAC) NUNCA fecha automaticamente.** Permanece visível até o browser redirecionar.
6. **Todos os ícones na interface usam Font Awesome Solid (`fas`).**
7. **Todos os campos de texto passam por `trim()` antes de salvar** (remove espaços copiados acidentalmente).

---

## 📋 FASE 1 — Modo Access Point

### Condição de entrada
O ESP32 entra em modo AP quando:
- Não há SSID salvo em `/config.json` (LittleFS), OU
- O SSID salvo falha na conexão após 30 segundos

### Configurações do AP
```
SSID:      CanalQb-ESP32
IP:        192.168.4.1
Hostname:  canalqb_esp32
DHCP:      ativo
```

### Captive Portal (handlers obrigatórios)
O servidor DNS interno responde por QUALQUER domínio com o IP `192.168.4.1`.
Implemente estes handlers HTTP para garantir abertura automática no Android e Windows:

```cpp
server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *r){ r->redirect("http://canalqb.com.br"); });
server.on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest *r){ r->send(200, "text/plain", "Microsoft Connect Test"); });
server.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest *r){ r->send(200, "text/plain", "Microsoft NCSI"); });
server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *r){ r->redirect("http://192.168.4.1"); });
server.on("/redirect", HTTP_GET, [](AsyncWebServerRequest *r){ r->redirect("http://192.168.4.1"); });
// onNotFound → se Host for "canalqb.com.br", servir HTML local. Senão, redirect para 192.168.4.1
```

### Interface HTML de setup (embutida em PROGMEM)
A interface deve ser **100% offline**, sem CDN externo. Use CSS inline e JS inline.

**Componentes:**
1. Header com logo CanalQb e título "Configuração Wi-Fi"
2. Lista de SSIDs (div com altura 220px, overflow-y scroll), cada item clicável
3. Modal de senha com: campo input UTF-8, botão "Conectar" + botão "Cancelar", tecla Enter aciona conexão
4. Caixa terminal preta (fundo #000, texto #00ff00, fonte monospace, altura 80px, overflow-y scroll)
5. Modal de sucesso (IP + MAC + countdown + instrução)

**Scanner de redes:**
```javascript
// Inicia automaticamente após 1 segundo do carregamento
setTimeout(scan, 1000);

async function scan() {
  const r = await fetch('/api/scan');
  if (r.status === 202) { setTimeout(scan, 2000); return; } // ainda escaneando
  const nets = await r.json();
  // renderizar lista de redes com ícone de sinal e cadeado se WPA
}
```

**Fluxo de conexão (AJAX — sem recarregar página):**
```javascript
async function connect(ssid, pass) {
  log('Tentando conectar em ' + ssid + '...');
  try {
    const r = await fetch('/api/connect?ssid=' + encodeURIComponent(ssid) + '&pass=' + encodeURIComponent(pass));
    const data = await r.json();
    if (data.error) { log('Erro: ' + data.error); return; }
    // SUCESSO — sequência obrigatória:
    showSuccessModal(data.ip, data.mac); // abre modal e NÃO fecha
    let t = 10;
    const countdown = setInterval(() => {
      updateCountdown(t--);
      if (t < 0) { clearInterval(countdown); window.location.href = 'http://' + data.ip; }
    }, 1000);
    // Após 1 segundo: aciona restart do ESP32
    setTimeout(() => fetch('/api/restart'), 1000);
  } catch(e) {
    log('Falha na conexão. Verifique a senha e tente novamente.');
  }
}
```

**Endpoints da Fase 1:**

```cpp
// GET /api/scan — retorna JSON array ou 202 se escaneando
// Exemplo de resposta: [{"ssid":"MinhaRede","rssi":-45,"auth":"WPA2"},...]

// GET /api/connect?ssid=X&pass=Y
// Sucesso: {"ip":"192.168.0.22","mac":"AA:BB:CC:DD:EE:FF","gateway":"192.168.0.1"}
// Erro:    {"error":"Senha incorreta ou rede inacessível"}
// Timeout: 15 segundos máximo de tentativa

// GET /api/restart
// Aguarda 1s, desliga AP, aguarda 3s, ESP.restart()
```

**Lógica de scan assíncrono:**
```cpp
// No setup():
WiFi.scanNetworks(true, false); // assíncrono, sem redes ocultas

// Em handleScan():
int n = WiFi.scanComplete();
if (n == WIFI_SCAN_RUNNING) { request->send(202, "application/json", "[]"); return; }
// serializar resultados em JSON
// chamar WiFi.scanNetworks(true, false) novamente para próximo ciclo
```

**Lógica de conexão (sem travar o servidor):**
```cpp
// handleConnect():
WiFi.begin(ssid.c_str(), pass.c_str());
unsigned long t = millis();
while (WiFi.status() != WL_CONNECTED && millis() - t < 15000) {
  delay(100);
}
if (WiFi.status() == WL_CONNECTED) {
  // salvar config.json
  // retornar IP, MAC, gateway
} else {
  WiFi.disconnect();
  // retornar erro
}
```

---

## 🌐 FASE 2 — Modo Station (Servidor Espelho)

### Condição de entrada
ESP32 conecta ao Wi-Fi salvo → `WiFi.status() == WL_CONNECTED`

### Gestão do AP após reconexão
```cpp
// No loop():
static bool apHidden = false;
if (WiFi.status() == WL_CONNECTED && !apHidden) {
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  apHidden = true;
} else if (WiFi.status() != WL_CONNECTED && apHidden) {
  // Perdeu conexão — reativar AP
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("CanalQb-ESP32");
  apHidden = false;
}
```

### handleRoot — Streaming Proxy (IMPLEMENTAÇÃO EXATA)

```cpp
void handleRoot(AsyncWebServerRequest *request) {
  if (WiFi.status() != WL_CONNECTED) {
    // Fase 1: servir HTML local
    request->send(200, "text/html; charset=utf-8", local_setup_html);
    return;
  }
  
  // Fase 2: Streaming Proxy do GitHub
  WiFiClientSecure *client = new WiFiClientSecure();
  client->setInsecure();
  HTTPClient http;
  
  String ip = WiFi.localIP().toString();
  String url = "https://raw.githack.com/canalqb/modulos_ESP/main/ambiente/index.html?ip=" + ip;
  
  http.begin(*client, url);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setTimeout(10000);
  http.addHeader("User-Agent", "CanalQb-ESP32/1.0");
  http.addHeader("Cache-Control", "no-cache");
  
  int code = http.GET();
  
  if (code == HTTP_CODE_OK) {
    // Stream pedaço por pedaço
    AsyncResponseStream *response = request->beginResponseStream("text/html; charset=utf-8");
    WiFiClient *stream = http.getStreamPtr();
    uint8_t buf[4096];
    int len;
    while (http.connected() && (len = stream->readBytes(buf, sizeof(buf))) > 0) {
      response->write(buf, len);
    }
    request->send(response);
  } else {
    // Fallback: HTML local
    request->send(200, "text/html; charset=utf-8", local_setup_html);
  }
  
  http.end();
  delete client;
}
```

### onNotFound — Proxy Universal
Qualquer rota não reconhecida deve ser buscada na pasta `/ambiente/` do GitHub:

```cpp
server.onNotFound([](AsyncWebServerRequest *request) {
  String path = request->url();
  
  // Detectar MIME type pela extensão
  String mime = "text/plain";
  if (path.endsWith(".css")) mime = "text/css";
  else if (path.endsWith(".js")) mime = "application/javascript";
  else if (path.endsWith(".png")) mime = "image/png";
  else if (path.endsWith(".jpg")) mime = "image/jpeg";
  else if (path.endsWith(".ico")) mime = "image/x-icon";
  else if (path.endsWith(".svg")) mime = "image/svg+xml";
  
  // Buscar do GitHub
  String url = "https://raw.githack.com/canalqb/modulos_ESP/main/ambiente" + path;
  // ... streaming proxy com mesmo padrão do handleRoot
});
```

---

## 💾 Persistência de Dados

### Arquivo /config.json (LittleFS)
```json
{
  "wifi": {
    "ssid": "",
    "password": ""
  },
  "google": {
    "client_id": "",
    "client_secret": "",
    "refresh_token": "",
    "folder_id": "",
    "service_email": ""
  },
  "anthropic": {
    "keys": ["sk-ant-..."]
  },
  "openrouter": {
    "enabled": false,
    "api_key": ""
  },
  "bluetooth": {
    "enabled": true,
    "name": "CanalQb-ESP32",
    "pin": "1234"
  },
  "network": {
    "hostname": "canalqb_esp32",
    "static_ip": false,
    "ip": "",
    "gateway": "",
    "subnet": "",
    "dns": ""
  }
}
```

### Funções obrigatórias
```cpp
void loadConfig();   // Carrega /config.json no LittleFS
void saveConfig();   // Salva /config.json no LittleFS
String getConfigJson(); // Retorna JSON sem senhas (para API)
```

---

## 📡 APIs Internas (Fase 2)

Todos estes endpoints devem funcionar quando o ESP32 está em modo STA:

```
POST /api/chat      — Chat com IA (body JSON: {"prompt":"...","user":"..."})
POST /webhook       — Webhook externo (mesmo formato do /api/chat)
GET  /api/status    — Status do sistema
POST /api/save      — Salvar configurações
GET  /api/config    — Retornar configurações atuais (sem senhas)
GET  /api/test-drive — Testar conexão com Google Drive
POST /api/factory-reset — Reset total
GET  /api/modules   — Lista de módulos (busca market.json do GitHub)
POST /api/install   — Instalar módulo no Drive
```

### Chat com IA (sistema de fallback de keys)
```cpp
String callAI(String prompt, String history, String user) {
  // Separar keys por ";"
  // Tentar cada key até uma funcionar
  // Se todas falharem, retornar erro claro
  // Salvar histórico no Drive após resposta bem-sucedida
}
```

---

## 🔄 Reset de Fábrica

```cpp
void handleFactoryReset(AsyncWebServerRequest *request) {
  // Limpar NVS
  Preferences prefs;
  prefs.begin("canalqb", false);
  prefs.clear();
  prefs.end();
  
  // Formatar LittleFS
  LittleFS.format();
  
  // Responder ao browser
  request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Reiniciando...\"}");
  
  // Aguardar e reiniciar
  delay(2000);
  ESP.restart();
}
```

---

## 📦 platformio.ini

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
board_build.partitions = huge_app.csv
board_build.filesystem = littlefs
lib_deps =
  bblanchon/ArduinoJson @ ^7
  me-no-dev/ESPAsyncWebServer @ ^1.2.3
  DNSServer
  BluetoothSerial
  ESP32 BLE Arduino
```

---

## 🎨 Estilo da Interface Local (Fase 1)

Use estas variáveis CSS para manter a identidade visual:
```css
:root {
  --primary: #0062ff;
  --bg: #0f172a;
  --card: #1e293b;
  --text: #f8fafc;
  --muted: #94a3b8;
  --success: #22c55e;
  --danger: #ef4444;
  --warning: #f59e0b;
  --border: #334155;
  --terminal-bg: #000000;
  --terminal-text: #00ff00;
}
```

A interface deve:
- Funcionar em mobile (responsiva)
- Ter touch targets mínimos de 44px
- Não ter scroll horizontal
- Usar transições suaves nos modais (fade/slide)
- Mostrar feedback visual imediato em cada ação

---

## ✅ Checklist de Validação

Antes de considerar o firmware completo, verificar:

**Fase 1:**
- [ ] AP sobe com SSID "CanalQb-ESP32" e IP 192.168.4.1
- [ ] Android abre captive portal automaticamente
- [ ] Windows abre captive portal automaticamente
- [ ] Scanner carrega redes sem necessidade de botão (automático após 1s)
- [ ] SSIDs aparecem completos, sem truncamento
- [ ] Clicar em rede abre modal de senha
- [ ] Enter no campo de senha aciona conexão
- [ ] Botão Cancelar fecha o modal
- [ ] Senha errada → mensagem de erro, sem reiniciar, pode tentar novamente
- [ ] Conexão bem-sucedida → modal abre com IP/MAC e NÃO fecha
- [ ] Countdown de 10s visível no modal
- [ ] Após 1s do sucesso → ESP32 agenda restart
- [ ] Browser redireciona para o IP correto
- [ ] ESP32 reinicia 3s após o restart ser acionado

**Fase 2:**
- [ ] Ao acessar IP local, a barra de endereços NÃO muda
- [ ] Conteúdo do GitHub é carregado corretamente
- [ ] CSS e JS do ambiente são servidos via proxy
- [ ] Favicon carregado do GitHub
- [ ] AP não aparece mais nas redes Wi-Fi disponíveis
- [ ] Se Wi-Fi cair, AP reaparece automaticamente
- [ ] Se reconectar, AP desaparece novamente

**APIs:**
- [ ] /api/chat responde com contexto de histórico
- [ ] /webhook aceita POST externo
- [ ] /api/factory-reset apaga tudo e reinicia
- [ ] /api/test-drive cria arquivo de teste no Drive (sem duplicatas)

---

## 🚀 Ordem de Implementação Sugerida

1. Setup do PlatformIO com partição `huge_app`
2. Sistema de arquivos LittleFS e loadConfig/saveConfig
3. Lógica de modo (AP vs STA) no setup()
4. Servidor DNS para captive portal
5. HTML de setup local (PROGMEM) com scanner
6. Endpoints /api/scan e /api/connect
7. Lógica de restart (/api/restart)
8. handleRoot com Streaming Proxy (Fase 2)
9. onNotFound com Proxy Universal
10. Endpoints de chat e webhook
11. Integração Google Drive (autenticação + upload)
12. Bluetooth
13. Reset de fábrica
14. Testes de todos os fluxos

---

## 📎 Referências Importantes

- Ambiente web GitHub: `https://raw.githack.com/canalqb/modulos_ESP/main/ambiente/index.html`
- Loja de módulos: `https://raw.githubusercontent.com/canalqb/modulos_ESP/main/market.json`
- Blog do projeto: `https://canalqb.com.br`
- Chaves Anthropic: `https://platform.claude.com/settings/keys`
- Setup OAuth Google: `https://www.canalqb.com.br/2026/04/google-cloud-console-oauth-20-clientid.html`
- Doação Pix/PayPal: `qrodrigob@gmail.com`

---

> **Nota para o LLM:** Este documento é completo e autocontido.
> Ao gerar o código, siga rigorosamente a ordem da Fase 1 e depois a Fase 2.
> Jamais quebre o que está funcionando ao adicionar novos recursos.
> A função `handleRoot` é o coração do sistema — implemente-a com cuidado total.
