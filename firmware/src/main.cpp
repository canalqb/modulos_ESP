/**
 * CanalQb ESP32 Hub - Firmware Principal
 * 
 * Sistema dual-mode:
 * - Fase 1: AP Mode com portal de configuração Wi-Fi
 * - Fase 2: STA Mode com Streaming Proxy do GitHub
 * 
 * Regras Obrigatórias:
 * - NUNCA redirecionar navegador para URL externa
 * - NUNCA usar alert() ou confirm() - usar modais customizados
 * - SEMPRE formatar flash antes de gravar (pio run --target erase)
 * - Streaming Proxy mantém IP local na barra de endereços
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <BluetoothSerial.h>

// Configurações do AP
const char* AP_SSID = "CanalQb-ESP32";
const char* AP_PASSWORD = "";  // Rede aberta
const IPAddress AP_IP(192, 168, 4, 1);

// Portas e configurações
const int DNS_PORT = 53;
const int WEB_PORT = 80;

// Objetos globais
AsyncWebServer server(WEB_PORT);
DNSServer dnsServer;
Preferences prefs;
BluetoothSerial SerialBT;

// Estado do sistema
bool isAPMode = true;
bool shouldRestart = false;
unsigned long restartTime = 0;

// Configurações (carregadas do LittleFS)
struct Config {
  String wifi_ssid = "";
  String wifi_password = "";
  String hostname = "canalqb_esp32";
  bool bluetooth_enabled = false;
  String bt_name = "CanalQb-ESP32";
  String bt_pin = "1234";
} config;

// HTML da página de configuração (PROGMEM)
const char setup_html[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CanalQb ESP32 - Hub de Configuração</title>
    <style>
        :root {
            --primary: #6366f1;
            --primary-hover: #4f46e5;
            --bg-gradient: linear-gradient(135deg, #0f172a 0%, #1e1b4b 100%);
            --card-bg: rgba(255, 255, 255, 0.05);
            --card-border: rgba(255, 255, 255, 0.1);
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
            --success: #22c55e;
            --danger: #ef4444;
        }
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: system-ui, -apple-system, sans-serif;
            background: var(--bg-gradient);
            color: var(--text-main);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 20px;
            backdrop-filter: blur(10px);
        }
        .container {
            background: var(--card-bg);
            backdrop-filter: blur(20px);
            border: 1px solid var(--card-border);
            border-radius: 24px;
            box-shadow: 0 25px 50px -12px rgba(0, 0, 0, 0.5);
            width: 100%;
            max-width: 480px;
            overflow: hidden;
            animation: fadeIn 0.6s ease-out;
        }
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(20px); }
            to { opacity: 1; transform: translateY(0); }
        }
        .header {
            padding: 40px 30px;
            text-align: center;
            background: rgba(255, 255, 255, 0.02);
            border-bottom: 1px solid var(--card-border);
        }
        .header h1 {
            font-size: 28px;
            font-weight: 800;
            background: linear-gradient(to right, #818cf8, #c084fc);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            margin-bottom: 10px;
        }
        .header p {
            color: var(--text-muted);
            font-size: 15px;
            font-weight: 500;
        }
        .content { padding: 30px; }
        .wifi-list {
            background: rgba(0, 0, 0, 0.2);
            border-radius: 16px;
            padding: 10px;
            height: 240px;
            overflow-y: auto;
            margin-bottom: 24px;
            border: 1px solid var(--card-border);
        }
        .wifi-list::-webkit-scrollbar { width: 6px; }
        .wifi-list::-webkit-scrollbar-thumb { background: rgba(255,255,255,0.1); border-radius: 10px; }
        .wifi-item {
            display: flex;
            align-items: center;
            padding: 14px 18px;
            margin-bottom: 8px;
            background: rgba(255, 255, 255, 0.03);
            border-radius: 12px;
            cursor: pointer;
            transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
            border: 1px solid transparent;
        }
        .wifi-item:hover {
            background: rgba(255, 255, 255, 0.08);
            border-color: var(--primary);
            transform: scale(1.02);
        }
        .wifi-icon { width: 24px; color: var(--primary); margin-right: 15px; font-size: 18px; }
        .wifi-name { flex: 1; font-weight: 500; font-size: 15px; }
        .wifi-lock { color: var(--text-muted); font-size: 14px; }
        .terminal {
            background: rgba(0, 0, 0, 0.4);
            color: #4ade80;
            font-family: monospace;
            font-size: 12px;
            padding: 15px;
            height: 100px;
            overflow-y: auto;
            border-radius: 12px;
            margin-bottom: 24px;
            border: 1px solid var(--card-border);
            line-height: 1.5;
        }
        .btn {
            background: var(--primary);
            color: white;
            border: none;
            padding: 14px 28px;
            border-radius: 12px;
            cursor: pointer;
            font-weight: 600;
            font-size: 15px;
            transition: all 0.3s;
            width: 100%;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 10px;
            box-shadow: 0 4px 15px rgba(99, 102, 241, 0.3);
        }
        .btn:hover {
            background: var(--primary-hover);
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(99, 102, 241, 0.4);
        }
        .btn:active { transform: translateY(0); }
        .btn:disabled { opacity: 0.5; cursor: not-allowed; transform: none; }
        .btn-secondary { background: rgba(255,255,255,0.1); color: var(--text-main); box-shadow: none; }
        .btn-secondary:hover { background: rgba(255,255,255,0.15); box-shadow: none; }
        
        .modal {
            display: none;
            position: fixed;
            top: 0; left: 0; width: 100%; height: 100%;
            background: rgba(0,0,0,0.8);
            backdrop-filter: blur(8px);
            z-index: 1000;
            align-items: center;
            justify-content: center;
            padding: 20px;
        }
        .modal.show { display: flex; animation: modalIn 0.3s ease-out; }
        @keyframes modalIn {
            from { opacity: 0; scale: 0.95; }
            to { opacity: 1; scale: 1; }
        }
        .modal-content {
            background: #1e293b;
            padding: 35px;
            border-radius: 24px;
            width: 100%;
            max-width: 400px;
            border: 1px solid var(--card-border);
            box-shadow: 0 25px 50px -12px rgba(0, 0, 0, 0.5);
        }
        .modal-header h3 { font-size: 22px; margin-bottom: 10px; }
        .modal-header p { color: var(--text-muted); font-size: 14px; margin-bottom: 25px; }
        
        .form-group { margin-bottom: 25px; }
        .form-group label { display: block; margin-bottom: 10px; font-size: 13px; font-weight: 600; color: var(--text-muted); text-transform: uppercase; letter-spacing: 0.05em; }
        .password-wrapper { position: relative; }
        .form-group input {
            width: 100%;
            padding: 14px 18px;
            background: rgba(0,0,0,0.2);
            border: 1px solid var(--card-border);
            border-radius: 12px;
            color: white;
            font-size: 16px;
            transition: all 0.2s;
            outline: none;
        }
        .form-group input:focus { border-color: var(--primary); box-shadow: 0 0 0 4px rgba(99, 102, 241, 0.15); }
        .toggle-password {
            position: absolute;
            right: 15px; top: 50%;
            transform: translateY(-50%);
            background: none; border: none;
            color: var(--text-muted); cursor: pointer;
            font-size: 18px; transition: color 0.2s;
        }
        .toggle-password:hover { color: var(--text-main); }
        .modal-buttons { display: flex; gap: 15px; }
        
        .success-info {
            background: rgba(34, 197, 94, 0.1);
            border: 1px solid rgba(34, 197, 94, 0.2);
            padding: 20px;
            border-radius: 16px;
            margin: 25px 0;
            text-align: left;
        }
        .success-info div { margin-bottom: 8px; font-size: 14px; }
        .success-info strong { color: var(--success); font-family: monospace; margin-right: 8px; }
        .countdown { font-size: 32px; font-weight: 800; color: var(--primary); margin: 20px 0; }
        .spinner {
            border: 3px solid rgba(255,255,255,0.05);
            border-top: 3px solid var(--primary);
            border-radius: 50%;
            width: 40px; height: 40px;
            animation: spin 0.8s linear infinite;
            margin: 0 auto 20px;
        }
        @keyframes spin { to { transform: rotate(360deg); } }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>📶 CanalQb ESP32</h1>
            <p>Configuração Profissional de Wi-Fi</p>
        </div>
        <div class="content">
            <div class="wifi-list" id="wifiList">
                <div style="text-align:center; padding:50px;">
                    <div class="spinner"></div>
                    <p style="color:var(--text-muted); font-size:14px;">Escaneando redes disponíveis...</p>
                </div>
            </div>
            <button class="btn" onclick="scanNetworks()" id="scanBtn">
                🔄 Atualizar Redes
            </button>
            <div class="terminal" id="terminal">Sistema Opus iniciado. Aguardando interação...</div>
        </div>
    </div>

    <!-- Modal de Senha -->
    <div class="modal" id="passwordModal">
        <div class="modal-content">
            <div class="modal-header">
                <h3>🛡️ Conectar</h3>
                <p id="selectedSSID"></p>
            </div>
            <div class="form-group">
                <label>Senha da Rede</label>
                <div class="password-wrapper">
                    <input type="password" id="password" placeholder="••••••••">
                    <button type="button" class="toggle-password" onclick="togglePassword()">
                        <span id="eyeIcon">👁️</span>
                    </button>
                </div>
            </div>
            <div class="modal-buttons">
                <button class="btn btn-secondary" onclick="closePasswordModal()">Cancelar</button>
                <button class="btn" onclick="connect()">Conectar</button>
            </div>
        </div>
    </div>

    <!-- Modal de Sucesso -->
    <div class="modal" id="successModal">
        <div class="modal-content" style="text-align:center;">
            <div class="modal-header">
                <h3>✅ Conectado!</h3>
                <p>Configurações salvas permanentemente.</p>
            </div>
            <div class="success-info">
                <div><strong>IP LOCAL:</strong> <span id="connectedIP"></span></div>
                <div><strong>MAC ADDR:</strong> <span id="connectedMAC"></span></div>
            </div>
            <div class="countdown" id="countdown">15</div>
            <p style="color:var(--text-muted); font-size:13px; line-height:1.5;">
                O ESP32 está reiniciando e desligando o modo AP.<br>Redirecionando automaticamente...
            </p>
        </div>
    </div>

    <script>
        // SMART LOADER - CanalQb v2
        (async function() {
            const isSTA = window.location.hostname !== '192.168.4.1' && window.location.hostname !== 'localhost';
            
            if (isSTA) {
                console.log("Modo STA detectado. Tentando carregar Dashboard remoto...");
                document.body.innerHTML = `
                    <div style="display:flex; flex-direction:column; align-items:center; justify-content:center; height:100vh; background:#0f172a; color:white; font-family:sans-serif;">
                        <div class="spinner"></div>
                        <h2 style="margin-top:20px;">Carregando CanalQb Dashboard...</h2>
                        <p style="color:#94a3b8; margin-top:10px;">Sincronizando com modulos_ESP/main</p>
                    </div>
                `;
                
                try {
                    // Usar JSDelivr para evitar problemas de MIME type (raw do github é text/plain)
                    const baseUrl = "https://cdn.jsdelivr.net/gh/canalqb/modulos_ESP@main/ambiente";
                    
                    const link = document.createElement('link');
                    link.rel = 'stylesheet';
                    link.href = `${baseUrl}/style.css`;
                    document.head.appendChild(link);

                    const response = await fetch(`${baseUrl}/index.html`);
                    if (response.ok) {
                        const html = await response.text();
                        const parser = new DOMParser();
                        const doc = parser.parseFromString(html, 'text/html');
                        
                        // Importar estilos e links do HEAD remoto
                        doc.querySelectorAll('link, style').forEach(el => {
                            const newEl = document.createElement(el.tagName);
                            Array.from(el.attributes).forEach(attr => newEl.setAttribute(attr.name, attr.value));
                            newEl.innerHTML = el.innerHTML;
                            document.head.appendChild(newEl);
                        });

                        document.body.innerHTML = doc.body.innerHTML;
                        
                        doc.querySelectorAll('script').forEach(oldScript => {
                            const newScript = document.createElement('script');
                            Array.from(oldScript.attributes).forEach(attr => newScript.setAttribute(attr.name, attr.value));
                            if (oldScript.src) {
                                // Se for script externo, manter a fonte
                            } else {
                                newScript.appendChild(document.createTextNode(oldScript.innerHTML));
                            }
                            document.body.appendChild(newScript);
                        });
                        
                        // ENCERRAR scripts locais para evitar erros de 'null'
                        window.stopLocalScripts = true;
                    }
                } catch (e) {
                    console.error("Falha ao carregar UI remota:", e);
                    document.body.innerHTML += '<p style="color:red; margin-top:10px;">Erro ao conectar com o GitHub. Verifique sua internet.</p>';
                }
            }
        })();

        let currentSSID = '';
        let countdownInterval;

        setTimeout(() => { if(!window.stopLocalScripts) scanNetworks(); }, 1000);

        function log(message) {
            if (window.stopLocalScripts) return;
            const terminal = document.getElementById('terminal');
            if (terminal) {
                terminal.innerHTML += '> ' + message + '\n';
                terminal.scrollTop = terminal.scrollHeight;
            }
        }

        async function scanNetworks() {
            if (window.stopLocalScripts) return;
            const btn = document.getElementById('scanBtn');
            if (!btn) return;
            btn.disabled = true;
            btn.innerHTML = '⌛ Escaneando...';
            log('Iniciando scan de redes 2.4GHz...');
            
            try {
                const response = await fetch('/api/scan');
                if (response.status === 202) {
                    setTimeout(scanNetworks, 2000);
                    return;
                }
                const networks = await response.json();
                displayNetworks(networks);
                log('Encontradas ' + networks.length + ' redes no ambiente.');
            } catch (error) {
                log('Erro: ' + error.message);
            } finally {
                btn.disabled = false;
                btn.innerHTML = '🔄 Atualizar Redes';
            }
        }

        function displayNetworks(networks) {
            const wifiList = document.getElementById('wifiList');
            if (networks.length === 0) {
                wifiList.innerHTML = '<div style="text-align:center; padding:50px; color:var(--text-muted);">Nenhuma rede encontrada</div>';
                return;
            }
            wifiList.innerHTML = networks.slice(0, 10).map(net => `
                <div class="wifi-item" onclick="selectNetwork('${net.ssid}', ${net.auth})">
                    <div class="wifi-icon">📶</div>
                    <div class="wifi-name">${net.ssid}</div>
                    ${net.auth ? '<div class="wifi-lock">🔒</div>' : ''}
                </div>
            `).join('');
        }

        function selectNetwork(ssid, hasAuth) {
            currentSSID = ssid;
            document.getElementById('selectedSSID').textContent = 'Rede selecionada: ' + ssid;
            document.getElementById('password').value = '';
            if (!hasAuth) { connect(); }
            else {
                document.getElementById('passwordModal').classList.add('show');
                document.getElementById('password').focus();
            }
        }

        function togglePassword() {
            const pass = document.getElementById('password');
            const icon = document.getElementById('eyeIcon');
            if (pass.type === 'password') {
                pass.type = 'text';
                icon.textContent = '🙈';
            } else {
                pass.type = 'password';
                icon.textContent = '👁️';
            }
        }

        function closePasswordModal() { document.getElementById('passwordModal').classList.remove('show'); }

        async function connect() {
            const pass = document.getElementById('password').value;
            closePasswordModal();
            log('Iniciando handshake com ' + currentSSID + '...');
            
            try {
                const url = `/api/connect?ssid=${encodeURIComponent(currentSSID)}&pass=${encodeURIComponent(pass)}`;
                const response = await fetch(url);
                const data = await response.json();
                
                if (data.error) {
                    log('Falha: ' + data.error);
                    return;
                }
                
                log('Handshake OK! IP: ' + data.ip);
                showSuccessModal(data.ip, data.mac);
            } catch (error) {
                log('Erro fatal de rede.');
            }
        }

        function showSuccessModal(ip, mac) {
            document.getElementById('connectedIP').textContent = ip;
            document.getElementById('connectedMAC').textContent = mac;
            document.getElementById('successModal').classList.add('show');
            
            let count = 15;
            countdownInterval = setInterval(() => {
                count--;
                document.getElementById('countdown').textContent = count;
                if (count <= 0) {
                    clearInterval(countdownInterval);
                    window.location.href = 'http://' + ip;
                }
            }, 1000);
            
            setTimeout(() => fetch('/api/restart'), 1000);
        }

        const passField = document.getElementById('password');
        if (passField) {
            passField.addEventListener('keypress', e => { if (e.key === 'Enter') connect(); });
        }
        document.addEventListener('keydown', e => { 
            if (window.stopLocalScripts) return;
            if (e.key === 'Escape') closePasswordModal(); 
        });
    </script>
</body>
</html>
)HTML";

// Declarações das funções
void loadConfig();
void saveConfig();
void handleScan(AsyncWebServerRequest *request);
void handleConnect(AsyncWebServerRequest *request);
void handleRestart(AsyncWebServerRequest *request);
void handleRoot(AsyncWebServerRequest *request);
void handleNotFound(AsyncWebServerRequest *request);
void handleCaptivePortal(AsyncWebServerRequest *request);
void handleConfig(AsyncWebServerRequest *request);
void setupAPMode();
void setupSTAMode();
void setupWebServer();

// Funções de configuração
void loadConfig() {
  File file = LittleFS.open("/config.json", "r");
  if (!file) {
    Serial.println("Arquivo de configuração não encontrado, usando padrões");
    return;
  }
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.println("Erro ao ler configuração: " + String(error.c_str()));
    return;
  }
  
  // Carregar configurações
  if (doc["wifi"]["ssid"].is<String>()) {
    config.wifi_ssid = doc["wifi"]["ssid"].as<String>();
  }
  if (doc["wifi"]["password"].is<String>()) {
    config.wifi_password = doc["wifi"]["password"].as<String>();
  }
  if (doc["network"]["hostname"].is<String>()) {
    config.hostname = doc["network"]["hostname"].as<String>();
  }
  if (doc["bluetooth"]["enabled"].is<bool>()) {
    config.bluetooth_enabled = doc["bluetooth"]["enabled"];
  }
  if (doc["bluetooth"]["name"].is<String>()) {
    config.bt_name = doc["bluetooth"]["name"].as<String>();
  }
  if (doc["bluetooth"]["pin"].is<String>()) {
    config.bt_pin = doc["bluetooth"]["pin"].as<String>();
  }
  
  Serial.println("Configuração carregada com sucesso");
}

void saveConfig() {
  JsonDocument doc;
  
  // Estrutura do JSON
  doc["wifi"]["ssid"] = config.wifi_ssid;
  doc["wifi"]["password"] = config.wifi_password;
  doc["network"]["hostname"] = config.hostname;
  doc["bluetooth"]["enabled"] = config.bluetooth_enabled;
  doc["bluetooth"]["name"] = config.bt_name;
  doc["bluetooth"]["pin"] = config.bt_pin;
  
  File file = LittleFS.open("/config.json", "w");
  if (!file) {
    Serial.println("Erro ao criar arquivo de configuração");
    return;
  }
  
  if (serializeJson(doc, file) == 0) {
    Serial.println("Erro ao escrever configuração");
  }
  file.close();
  
  Serial.println("Configuração salva com sucesso");
}

// Handlers da API
void handleScan(AsyncWebServerRequest *request) {
  int n = WiFi.scanComplete();
  
  if (n == WIFI_SCAN_RUNNING) {
    request->send(202, "application/json", "[]");
    return;
  }
  
  if (n == -1) {
    WiFi.scanNetworks(true, false);
    request->send(202, "application/json", "[]");
    return;
  }
  
  JsonDocument doc;
  JsonArray networks = doc.to<JsonArray>();
  
  for (int i = 0; i < n; i++) {
    JsonObject network = networks.add<JsonObject>();
    network["ssid"] = WiFi.SSID(i);
    network["rssi"] = WiFi.RSSI(i);
    network["auth"] = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
  }
  
  WiFi.scanDelete();
  WiFi.scanNetworks(true, false);
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void handleConnect(AsyncWebServerRequest *request) {
  if (!request->hasParam("ssid") || !request->hasParam("pass")) {
    request->send(400, "application/json", "{\"error\":\"Parâmetros ausentes\"}");
    return;
  }
  
  String ssid = request->getParam("ssid")->value();
  String pass = request->getParam("pass")->value();
  
  ssid.trim();
  pass.trim();
  
  Serial.println("Tentando conectar em: " + ssid);
  
  WiFi.begin(ssid.c_str(), pass.c_str());
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
    delay(100);
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    // Salvar configuração
    config.wifi_ssid = ssid;
    config.wifi_password = pass;
    saveConfig();
    
    JsonDocument doc;
    doc["ip"] = WiFi.localIP().toString();
    doc["mac"] = WiFi.macAddress();
    doc["gateway"] = WiFi.gatewayIP().toString();
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
    
    Serial.println("Conectado com sucesso!");
    
  } else {
    WiFi.disconnect();
    request->send(200, "application/json", "{\"error\":\"Senha incorreta ou rede inacessível\"}");
    Serial.println("Falha na conexão");
  }
}

void handleRestart(AsyncWebServerRequest *request) {
  request->send(200, "application/json", "{\"status\":\"ok\"}");
  
  shouldRestart = true;
  restartTime = millis() + 1000; // Aguardar 1 segundo antes de iniciar o processo
  
  Serial.println("Reiniciando ESP32...");
}

void handleConfig(AsyncWebServerRequest *request) {
  JsonDocument doc;
  doc["wifi"]["ssid"] = config.wifi_ssid;
  doc["network"]["hostname"] = config.hostname;
  doc["bluetooth"]["enabled"] = config.bluetooth_enabled;
  doc["bluetooth"]["name"] = config.bt_name;
  doc["bluetooth"]["pin"] = config.bt_pin;
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void handleRoot(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html; charset=utf-8", setup_html);
}

void handleNotFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not Found");
}

// Captive Portal handlers
void handleCaptivePortal(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html; charset=utf-8", setup_html);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== CanalQb ESP32 Hub ===");
  Serial.println("Versão: 1.0");
  
  // Inicializar LittleFS (formatar se falhar)
  if (!LittleFS.begin(true)) {
    Serial.println("Falha crítica no LittleFS");
    return;
  }
  
  // Carregar configuração
  loadConfig();
  
  // Configurar hostname
  WiFi.setHostname(config.hostname.c_str());
  
  // Verificar se temos configuração Wi-Fi
  if (config.wifi_ssid.length() == 0) {
    isAPMode = true;
    setupAPMode();
  } else {
    isAPMode = false;
    setupSTAMode();
  }
  
  // Inicializar Bluetooth se habilitado
  if (config.bluetooth_enabled) {
    SerialBT.begin(config.bt_name.c_str());
    Serial.println("Bluetooth iniciado: " + config.bt_name);
  }
  
  // Iniciar servidor web APENAS UMA VEZ ao final de tudo
  setupWebServer();
  
  Serial.println("Setup concluído");
}

void setupAPMode() {
  Serial.println("Entrando em modo AP...");
  
  // Configurar modo Wi-Fi
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));
  
  // Configurar DNS para Captive Portal
  dnsServer.start(DNS_PORT, "*", AP_IP);
  
  // Configurar modo Wi-Fi
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));
  
  // Configurar DNS para Captive Portal
  dnsServer.start(DNS_PORT, "*", AP_IP);
  
  // Iniciar scan de redes
  WiFi.scanNetworks(true, false);
  
  Serial.println("Modo AP iniciado");
  Serial.print("SSID: ");
  Serial.println(AP_SSID);
  Serial.print("IP: ");
  Serial.println(AP_IP);
}

void setupSTAMode() {
  Serial.println("Tentando conectar ao Wi-Fi...");
  Serial.println("SSID: " + config.wifi_ssid);
  
  // Garantir que o rádio AP esteja desligado
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(config.wifi_ssid.c_str(), config.wifi_password.c_str());
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
    
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
  } else {
    Serial.println("\nFalha na conexão Wi-Fi, entrando em modo AP");
    isAPMode = true;
    setupAPMode();
  }
}

void setupWebServer() {
  // Handlers principais
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/scan", HTTP_GET, handleScan);
  server.on("/api/connect", HTTP_GET, handleConnect);
  server.on("/api/restart", HTTP_GET, handleRestart);
  server.on("/api/config", HTTP_GET, handleConfig);
  
  // Captive Portal handlers
  server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("http://canalqb.com.br");
  });
  
  server.on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Microsoft Connect Test");
  });
  
  server.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Microsoft NCSI");
  });
  
  server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("http://192.168.4.1");
  });
  
  server.on("/redirect", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("http://192.168.4.1");
  });
  
  server.onNotFound([](AsyncWebServerRequest *request) {
    String host = request->host();
    if (host == "canalqb.com.br" || host.indexOf("192.168.4.1") >= 0) {
      handleCaptivePortal(request);
    } else {
      handleNotFound(request);
    }
  });
  
  server.begin();
  Serial.println("Servidor web iniciado");
}

void loop() {
  // Gerenciar DNS no modo AP
  if (isAPMode) {
    dnsServer.processNextRequest();
  }
  
  // Gerenciar reinicialização
  if (shouldRestart && millis() >= restartTime) {
    Serial.println("Reiniciando ESP32...");
    delay(100);
    ESP.restart();
  }

  delay(10);
}
