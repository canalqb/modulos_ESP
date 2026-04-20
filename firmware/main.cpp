#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <LittleFS.h>

// Configurações do AP
const char* AP_SSID = "CanalQb-ESP32";
const char* AP_PASSWORD = "";
const IPAddress AP_IP(192, 168, 4, 1);

// Globals
WebServer server(80);
DNSServer dnsServer;
Preferences preferences;
bool isAPMode = true;
String connectedSSID = "";
String connectedPassword = "";

// Estrutura para configuração
struct Config {
  String ssid;
  String password;
  String hostname;
} config;

// HTML da página de setup (em PROGMEM para economizar RAM)
const char* setup_html = R"(
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CanalQb ESP32 - Configuração Wi-Fi</title>
    <meta name="author" content="@CanalQb">
    <meta name="copyright" content="@CanalQb 2026">
    
    <style>
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

        *, *::before, *::after {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: var(--bg);
            color: var(--text);
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            line-height: 1.6;
        }

        .container {
            max-width: 480px;
            margin: 0 auto;
            padding: 20px;
            flex: 1;
            width: 100%;
        }

        .header {
            text-align: center;
            margin-bottom: 30px;
            padding: 20px 0;
        }

        .logo {
            font-size: 2.5rem;
            font-weight: 800;
            color: var(--primary);
            margin-bottom: 10px;
            font-family: 'Arial Black', sans-serif;
        }

        .subtitle {
            color: var(--muted);
            font-size: 1rem;
        }

        .card {
            background: var(--card);
            border: 1px solid var(--border);
            border-radius: 16px;
            padding: 24px;
            margin-bottom: 20px;
            box-shadow: 0 4px 20px rgba(0, 0, 0, 0.3);
        }

        .card-title {
            font-size: 1.2rem;
            font-weight: 700;
            margin-bottom: 16px;
            color: var(--text);
            display: flex;
            align-items: center;
            gap: 10px;
        }

        .wifi-list {
            max-height: 400px;
            overflow-y: auto;
            border: 1px solid var(--border);
            border-radius: 12px;
            background: rgba(0, 0, 0, 0.2);
            margin-bottom: 16px;
        }

        .wifi-item {
            padding: 12px 16px;
            border-bottom: 1px solid var(--border);
            cursor: pointer;
            transition: background 0.2s;
            display: flex;
            align-items: center;
            gap: 12px;
        }

        .wifi-item:last-child {
            border-bottom: none;
        }

        .wifi-item:hover {
            background: rgba(0, 98, 255, 0.1);
        }

        .wifi-signal {
            font-size: 1.2rem;
            color: var(--success);
            width: 20px;
            text-align: center;
        }

        .wifi-locked {
            color: var(--warning);
            font-size: 0.9rem;
        }

        .wifi-name {
            flex: 1;
            font-weight: 500;
        }

        .btn {
            background: var(--primary);
            color: white;
            border: none;
            padding: 12px 24px;
            border-radius: 10px;
            font-size: 1rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.2s;
            display: inline-flex;
            align-items: center;
            gap: 8px;
            min-height: 44px;
        }

        .btn:hover {
            background: #0052cc;
            transform: translateY(-1px);
        }

        .btn:disabled {
            opacity: 0.5;
            cursor: not-allowed;
            transform: none;
        }

        .btn-block {
            width: 100%;
            justify-content: center;
        }

        .btn-secondary {
            background: var(--muted);
        }

        .btn-secondary:hover {
            background: #64748b;
        }

        .modal {
            display: none;
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: rgba(0, 0, 0, 0.8);
            z-index: 1000;
            align-items: center;
            justify-content: center;
            padding: 20px;
        }

        .modal.show {
            display: flex;
        }

        .modal-content {
            background: var(--card);
            border: 1px solid var(--border);
            border-radius: 16px;
            padding: 24px;
            max-width: 400px;
            width: 100%;
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.4);
        }

        .modal-header {
            margin-bottom: 20px;
        }

        .modal-title {
            font-size: 1.3rem;
            font-weight: 700;
            color: var(--text);
            margin-bottom: 8px;
        }

        .modal-subtitle {
            color: var(--muted);
            font-size: 0.9rem;
        }

        .form-group {
            margin-bottom: 20px;
        }

        .form-label {
            display: block;
            margin-bottom: 8px;
            font-weight: 600;
            color: var(--text);
        }

        .input-group {
            position: relative;
            display: flex;
            align-items: center;
        }

        .form-input {
            flex: 1;
            background: var(--bg);
            border: 1.5px solid var(--border);
            border-radius: 10px;
            padding: 12px 16px;
            font-size: 1rem;
            color: var(--text);
            outline: none;
            transition: border-color 0.2s;
        }

        .form-input:focus {
            border-color: var(--primary);
        }

        .input-toggle {
            position: absolute;
            right: 12px;
            background: none;
            border: none;
            color: var(--muted);
            cursor: pointer;
            padding: 4px;
            font-size: 1.1rem;
        }

        .input-toggle:hover {
            color: var(--text);
        }

        .modal-footer {
            display: flex;
            gap: 12px;
            margin-top: 24px;
        }

        .terminal {
            background: var(--terminal-bg);
            color: var(--terminal-text);
            font-family: 'Courier New', monospace;
            font-size: 0.85rem;
            padding: 16px;
            border-radius: 10px;
            height: 120px;
            overflow-y: auto;
            border: 1px solid var(--border);
            margin-top: 16px;
        }

        .terminal-line {
            margin-bottom: 4px;
            word-break: break-all;
        }

        .success-modal .modal-title {
            color: var(--success);
        }

        .countdown {
            font-size: 2rem;
            font-weight: 700;
            color: var(--primary);
            text-align: center;
            margin: 20px 0;
        }

        .loading {
            display: inline-block;
            width: 16px;
            height: 16px;
            border: 2px solid var(--muted);
            border-top: 2px solid var(--primary);
            border-radius: 50%;
            animation: spin 1s linear infinite;
        }

        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }

        .error-message {
            background: rgba(239, 68, 68, 0.1);
            border: 1px solid var(--danger);
            color: var(--danger);
            padding: 12px;
            border-radius: 8px;
            margin-bottom: 16px;
            font-size: 0.9rem;
        }

        @media (max-width: 480px) {
            .container {
                padding: 16px;
            }
            
            .card {
                padding: 20px;
            }
            
            .modal-content {
                padding: 20px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <header class="header">
            <div class="logo">🚀 CanalQb</div>
            <div class="subtitle">Configuração Wi-Fi ESP32</div>
        </header>

        <main>
            <div class="card">
                <h2 class="card-title">
                    <span>📡</span>
                    Redes Wi-Fi Disponíveis
                </h2>
                
                <div class="wifi-list" id="wifiList">
                    <div style="padding: 20px; text-align: center; color: var(--muted);">
                        <div class="loading"></div>
                        Escaneando redes...
                    </div>
                </div>
                
                <button class="btn btn-block" onclick="scanNetworks()">
                    <span>🔄</span>
                    Atualizar Redes
                </button>
            </div>

            <div class="card">
                <h2 class="card-title">
                    <span>📟</span>
                    Terminal
                </h2>
                <div class="terminal" id="terminal">
                    <div class="terminal-line">Sistema iniciado. Aguardando comandos...</div>
                </div>
            </div>
        </main>
    </div>

    <!-- Modal de Senha -->
    <div class="modal" id="passwordModal">
        <div class="modal-content">
            <div class="modal-header">
                <h3 class="modal-title" id="modalTitle">Conectar à Rede</h3>
                <div class="modal-subtitle" id="modalSubtitle">Digite a senha para:</div>
            </div>
            
            <div class="form-group">
                <label class="form-label">Senha Wi-Fi</label>
                <div class="input-group">
                    <input type="password" class="form-input" id="passwordInput" placeholder="Digite a senha">
                    <button class="input-toggle" onclick="togglePassword()">
                        <span id="eyeIcon">👁️</span>
                    </button>
                </div>
            </div>
            
            <div class="modal-footer">
                <button class="btn btn-secondary" onclick="closePasswordModal()">
                    Cancelar
                </button>
                <button class="btn" onclick="connectToWifi()">
                    <span>🔗</span>
                    Conectar
                </button>
            </div>
        </div>
    </div>

    <!-- Modal de Sucesso -->
    <div class="modal success-modal" id="successModal">
        <div class="modal-content">
            <div class="modal-header">
                <h3 class="modal-title">✅ Conectado com Sucesso!</h3>
            </div>
            
            <div style="text-align: center; margin: 20px 0;">
                <div style="margin-bottom: 16px;">
                    <strong>IP:</strong> <span id="successIP"></span>
                </div>
                <div style="margin-bottom: 16px;">
                    <strong>MAC:</strong> <span id="successMAC"></span>
                </div>
                <div class="countdown" id="countdown">15</div>
                <div style="color: var(--muted); font-size: 0.9rem;">
                    Redirecionando em segundos...
                </div>
            </div>
        </div>
    </div>

    <script>
        let selectedSSID = '';
        let countdownInterval;

        // Inicia scan automático após 1 segundo
        setTimeout(scanNetworks, 1000);

        function log(message) {
            const terminal = document.getElementById('terminal');
            const line = document.createElement('div');
            line.className = 'terminal-line';
            line.textContent = '[' + new Date().toLocaleTimeString() + '] ' + message;
            terminal.appendChild(line);
            terminal.scrollTop = terminal.scrollHeight;
        }

        async function scanNetworks() {
            log('Iniciando scan de redes Wi-Fi...');
            
            try {
                const response = await fetch('/api/scan');
                
                if (response.status === 202) {
                    log('Ainda escaneando...');
                    setTimeout(scanNetworks, 2000);
                    return;
                }
                
                const networks = await response.json();
                displayNetworks(networks);
                log('Encontradas ' + networks.length + ' redes');
                
            } catch (error) {
                log('Erro ao escanear redes: ' + error.message);
                showError('Falha ao escanear redes. Tente novamente.');
            }
        }

        function displayNetworks(networks) {
            const wifiList = document.getElementById('wifiList');
            
            if (networks.length === 0) {
                wifiList.innerHTML = '<div style="padding: 20px; text-align: center; color: var(--muted);">Nenhuma rede encontrada</div>';
                return;
            }
            
            wifiList.innerHTML = networks.map(network => 
                '<div class="wifi-item" onclick="selectNetwork(\'' + escapeHtml(network.ssid) + '\', \'' + (network.auth || '') + '\')">' +
                    '<div class="wifi-signal">' + getSignalIcon(network.rssi || -50) + '</div>' +
                    '<div class="wifi-name">' + escapeHtml(network.ssid) + '</div>' +
                    (network.auth && network.auth !== 'OPEN' ? '<div class="wifi-locked">🔒</div>' : '') +
                '</div>'
            ).join('');
        }

        function getSignalIcon(rssi) {
            if (rssi > -50) return '📶';
            if (rssi > -60) return '📶';
            if (rssi > -70) return '📱';
            return '📶';
        }

        function escapeHtml(text) {
            const div = document.createElement('div');
            div.textContent = text;
            return div.innerHTML;
        }

        function selectNetwork(ssid, auth) {
            selectedSSID = ssid;
            document.getElementById('modalSubtitle').textContent = 'Digite a senha para: ' + ssid;
            document.getElementById('passwordInput').value = '';
            document.getElementById('passwordModal').classList.add('show');
            document.getElementById('passwordInput').focus();
        }

        function closePasswordModal() {
            document.getElementById('passwordModal').classList.remove('show');
            selectedSSID = '';
        }

        function togglePassword() {
            const input = document.getElementById('passwordInput');
            const icon = document.getElementById('eyeIcon');
            
            if (input.type === 'password') {
                input.type = 'text';
                icon.textContent = '🙈';
            } else {
                input.type = 'password';
                icon.textContent = '👁️';
            }
        }

        async function connectToWifi() {
            const password = document.getElementById('passwordInput').value.trim();
            
            if (!password) {
                showError('Digite a senha da rede');
                return;
            }
            
            closePasswordModal();
            log('Tentando conectar em ' + selectedSSID + '...');
            
            try {
                const response = await fetch('/api/connect?ssid=' + encodeURIComponent(selectedSSID) + '&pass=' + encodeURIComponent(password));
                const data = await response.json();
                
                if (data.error) {
                    log('Erro: ' + data.error);
                    showError('Senha inválida ou falha na conexão. Tente novamente.');
                    // Limpa cache e reinicia scan
                    setTimeout(function() {
                        scanNetworks();
                    }, 2000);
                    return;
                }
                
                // Sucesso
                log('Conectado! IP: ' + data.ip + ', MAC: ' + data.mac);
                showSuccessModal(data.ip, data.mac);
                
            } catch (error) {
                log('Falha na conexão: ' + error.message);
                showError('Falha na conexão. Verifique a senha e tente novamente.');
                setTimeout(scanNetworks, 2000);
            }
        }

        function showSuccessModal(ip, mac) {
            document.getElementById('successIP').textContent = ip;
            document.getElementById('successMAC').textContent = mac;
            document.getElementById('successModal').classList.add('show');
            
            // Inicia countdown de 15 segundos para redirecionamento
            let countdown = 15;
            countdownInterval = setInterval(function() {
                countdown--;
                document.getElementById('countdown').textContent = countdown;
                
                if (countdown <= 0) {
                    clearInterval(countdownInterval);
                    window.location.href = 'http://' + ip;
                }
            }, 1000);
            
            // Após 1 segundo, aciona reinicialização do ESP32
            setTimeout(function() {
                fetch('/api/restart');
            }, 1000);
        }

        function showError(message) {
            const errorDiv = document.createElement('div');
            errorDiv.className = 'error-message';
            errorDiv.textContent = message;
            
            const firstCard = document.querySelector('.card');
            firstCard.insertBefore(errorDiv, firstCard.firstChild);
            
            setTimeout(function() {
                errorDiv.remove();
            }, 5000);
        }

        // Enter no campo de senha aciona a conexão
        document.getElementById('passwordInput').addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                connectToWifi();
            }
        });

        // ESC fecha o modal
        document.addEventListener('keydown', function(e) {
            if (e.key === 'Escape') {
                closePasswordModal();
            }
        });
    </script>
</body>
</html>
)";

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== CanalQb ESP32 Iniciando ===");
  
  // Inicia LittleFS
  if (!LittleFS.begin()) {
    Serial.println("Erro ao iniciar LittleFS. Formatando...");
    LittleFS.format();
    if (!LittleFS.begin()) {
      Serial.println("Falha crítica no LittleFS");
      return;
    }
  }
  
  // Carrega configurações salvas
  loadConfig();
  
  // Inicia modo AP ou STA
  if (config.ssid.length() > 0) {
    Serial.println("Tentando conectar ao Wi-Fi salvo...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.ssid.c_str(), config.password.c_str());
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {
      delay(500);
      Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConectado ao Wi-Fi!");
      isAPMode = false;
      setupSTAMode();
    } else {
      Serial.println("\nFalha na conexão. Iniciando modo AP...");
      WiFi.disconnect();
      setupAPMode();
    }
  } else {
    Serial.println("Nenhum Wi-Fi salvo. Iniciando modo AP...");
    setupAPMode();
  }
}

void setupAPMode() {
  isAPMode = true;
  
  // Configura AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));
  
  // Configura DNS para captive portal
  dnsServer.start(53, "*", AP_IP);
  
  // Configura servidor web
  setupWebServer();
  
  Serial.println("Modo AP iniciado");
  Serial.print("SSID: ");
  Serial.println(AP_SSID);
  Serial.print("IP: ");
  Serial.println(AP_IP);
}

void setupSTAMode() {
  isAPMode = false;
  
  Serial.println("Modo STA iniciado");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  
  // Configura servidor web para modo STA
  setupWebServer();
}

void setupWebServer() {
  // Página principal
  server.on("/", HTTP_GET, handleRoot);
  
  // APIs
  server.on("/api/scan", HTTP_GET, handleScan);
  server.on("/api/connect", HTTP_GET, handleConnect);
  server.on("/api/restart", HTTP_GET, handleRestart);
  server.on("/api/status", HTTP_GET, handleStatus);
  
  // Captive portal handlers
  server.on("/generate_204", HTTP_GET, []() { server.sendHeader("Location", "http://192.168.4.1"); server.send(302, "text/plain", ""); });
  server.on("/connecttest.txt", HTTP_GET, []() { server.send(200, "text/plain", "Microsoft Connect Test"); });
  server.on("/ncsi.txt", HTTP_GET, []() { server.send(200, "text/plain", "Microsoft NCSI"); });
  server.on("/hotspot-detect.html", HTTP_GET, []() { server.sendHeader("Location", "http://192.168.4.1"); server.send(302, "text/plain", ""); });
  server.on("/redirect", HTTP_GET, []() { server.sendHeader("Location", "http://192.168.4.1"); server.send(302, "text/plain", ""); });
  
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("Servidor web iniciado");
}

void handleRoot() {
  if (isAPMode) {
    server.send(200, "text/html; charset=utf-8", setup_html);
  } else {
    // Modo STA - Streaming Proxy do GitHub
    streamFromGitHub();
  }
}

void streamFromGitHub() {
  WiFiClientSecure *client = new WiFiClientSecure();
  client->setInsecure();
  HTTPClient http;
  
  String url = "https://raw.githubusercontent.com/canalqb/modulos_ESP/main/ambiente/index.html";
  http.begin(*client, url);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setTimeout(10000);
  http.addHeader("User-Agent", "CanalQb-ESP32/1.0");
  http.addHeader("Cache-Control", "no-cache");
  
  int code = http.GET();
  
  if (code == HTTP_CODE_OK) {
    server.sendHeader("Content-Type", "text/html; charset=utf-8");
    server.sendHeader("Transfer-Encoding", "chunked");
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    
    WiFiClient *stream = http.getStreamPtr();
    uint8_t buf[4096];
    int len;
    while (http.connected() && (len = stream->readBytes(buf, sizeof(buf))) > 0) {
      server.sendContent_P((char*)buf, len);
    }
    
    server.sendContent("");
  } else {
    Serial.printf("Erro ao carregar do GitHub: %d\n", code);
    // Fallback para página local
    server.send(200, "text/html; charset=utf-8", setup_html);
  }
  
  http.end();
  delete client;
}

void handleScan() {
  if (WiFi.scanComplete() == WIFI_SCAN_RUNNING) {
    server.send(202, "application/json", "[]");
    return;
  }
  
  int n = WiFi.scanNetworks(false, true, false, 300);
  JsonDocument doc;
  JsonArray networks = doc.to<JsonArray>();
  
  for (int i = 0; i < n && i < 10; i++) {
    JsonObject network = networks.add<JsonObject>();
    network["ssid"] = WiFi.SSID(i);
    network["rssi"] = WiFi.RSSI(i);
    network["auth"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "OPEN" : "WPA";
  }
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
  
  // Inicia próximo scan
  WiFi.scanNetworks(true, false, false, 300);
}

void handleConnect() {
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");
  
  if (ssid.length() == 0) {
    server.send(400, "application/json", "{\"error\":\"SSID não informado\"}");
    return;
  }
  
  Serial.printf("Tentando conectar em: %s\n", ssid.c_str());
  
  WiFi.begin(ssid.c_str(), pass.c_str());
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
    delay(500);
    yield();
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    // Salva configuração
    config.ssid = ssid;
    config.password = pass;
    saveConfig();
    
    JsonDocument doc;
    doc["ip"] = WiFi.localIP().toString();
    doc["mac"] = WiFi.macAddress();
    doc["gateway"] = WiFi.gatewayIP().toString();
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
    
    connectedSSID = ssid;
    connectedPassword = pass;
  } else {
    WiFi.disconnect();
    server.send(400, "application/json", "{\"error\":\"Senha inválida ou rede inacessível\"}");
  }
}

void handleRestart() {
  server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Reiniciando...\"}");
  
  delay(1000);
  
  if (isAPMode) {
    WiFi.softAPdisconnect(true);
    delay(1000);
  }
  
  delay(2000);
  ESP.restart();
}

void handleStatus() {
  JsonDocument doc;
  doc["connected"] = (WiFi.status() == WL_CONNECTED);
  doc["ip"] = WiFi.localIP().toString();
  doc["mac"] = WiFi.macAddress();
  doc["ssid"] = WiFi.SSID();
  doc["mode"] = isAPMode ? "AP" : "STA";
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleNotFound() {
  if (isAPMode) {
    server.sendHeader("Location", "http://192.168.4.1");
    server.send(302, "text/plain", "");
  } else {
    // Tenta servir do GitHub
    String path = server.uri();
    
    if (path.endsWith("/")) path += "index.html";
    
    String mime = "text/html";
    if (path.endsWith(".css")) mime = "text/css";
    else if (path.endsWith(".js")) mime = "application/javascript";
    else if (path.endsWith(".png")) mime = "image/png";
    else if (path.endsWith(".jpg")) mime = "image/jpeg";
    else if (path.endsWith(".ico")) mime = "image/x-icon";
    else if (path.endsWith(".svg")) mime = "image/svg+xml";
    
    WiFiClientSecure *client = new WiFiClientSecure();
    client->setInsecure();
    HTTPClient http;
    
    String url = "https://raw.githubusercontent.com/canalqb/modulos_ESP/main/ambiente" + path;
    http.begin(*client, url);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(10000);
    
    int code = http.GET();
    
    if (code == HTTP_CODE_OK) {
      server.sendHeader("Content-Type", mime);
      server.sendHeader("Transfer-Encoding", "chunked");
      server.setContentLength(CONTENT_LENGTH_UNKNOWN);
      server.send(200, mime, "");
      
      WiFiClient *stream = http.getStreamPtr();
      uint8_t buf[4096];
      int len;
      while (http.connected() && (len = stream->readBytes(buf, sizeof(buf))) > 0) {
        server.sendContent_P((char*)buf, len);
      }
      
      server.sendContent("");
    } else {
      server.send(404, "text/plain", "Arquivo não encontrado");
    }
    
    http.end();
    delete client;
  }
}

void loadConfig() {
  preferences.begin("canalqb", false);
  config.ssid = preferences.getString("ssid", "");
  config.password = preferences.getString("password", "");
  config.hostname = preferences.getString("hostname", "canalqb_esp32");
  preferences.end();
}

void saveConfig() {
  preferences.begin("canalqb", false);
  preferences.putString("ssid", config.ssid);
  preferences.putString("password", config.password);
  preferences.putString("hostname", config.hostname);
  preferences.end();
}

void loop() {
  if (isAPMode) {
    dnsServer.processNextRequest();
  }
  
  server.handleClient();
  
  // Gerencia modo AP/STA dinâmico
  static bool apHidden = false;
  if (WiFi.status() == WL_CONNECTED && !apHidden) {
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    apHidden = true;
    Serial.println("AP desativado - modo STA ativo");
  } else if (WiFi.status() != WL_CONNECTED && apHidden) {
    // Perdeu conexão - reativar AP
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    apHidden = false;
    Serial.println("Conexão perdida - AP reativado");
  }
  
  delay(10);
}
