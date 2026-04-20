# CanalQb ESP32 CDN Package

## 📦 Descrição

Pacote de dependências externas otimizado para o CanalQb ESP32 Hub, hospedado via jsDelivr CDN para carregamento rápido e offline.

## 🚀 Funcionalidades

- **Bootstrap 5.3.3** - Framework CSS completo
- **Font Awesome 6.5.0** - Biblioteca de ícones completa  
- **Google Fonts** - Fontes personalizadas (Syne, DM Sans, JetBrains Mono)
- **CSS Customizado** - Estilos exclusivos do CanalQb
- **Carregamento Local** - Todas as dependências hospedadas localmente

## 📁 Estrutura de Arquivos

```
dist/
├── css/
│   ├── bootstrap.min.css      # Bootstrap 5.3.3
│   ├── fontawesome.min.css    # Font Awesome 6.5.0
│   └── google-fonts.css      # Google Fonts
├── js/
│   ├── bootstrap.bundle.min.js # Bootstrap JS
│   └── fontawesome.min.js    # Font Awesome JS
├── fonts/                  # Arquivos de fontes (quando necessário)
├── index.html               # Página de teste
├── manifest.json            # Manifesto do pacote
└── README.md               # Este arquivo
```

## 🌐 URLs CDN

O pacote usa as seguintes URLs CDN otimizadas:

### Bootstrap
- CSS: `https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css`
- JS: `https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js`

### Font Awesome
- CSS: `https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.0/css/all.min.css`
- JS: `https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.0/js/all.min.js`

### Google Fonts
- Fonts: `https://fonts.googleapis.com/css2?family=Syne:wght@600;700;800&family=DM+Sans:ital,wght@0,300;0,400;0,500;0,600;1,400&family=JetBrains+Mono:wght@400;500&display=swap`

## 🔧 Como Usar

### 1. No ESP32 (Streaming Proxy)

```cpp
// No firmware do ESP32, sirva os arquivos localmente
server.serveStatic("/css/bootstrap.min.css", LittleFS, "/css/bootstrap.min.css");
server.serveStatic("/js/bootstrap.bundle.min.js", LittleFS, "/js/bootstrap.bundle.min.js");
server.serveStatic("/css/fontawesome.min.css", LittleFS, "/css/fontawesome.min.css");
server.serveStatic("/js/fontawesome.min.js", LittleFS, "/js/fontawesome.min.js");
```

### 2. Via jsDelivr (Direto)

```html
<!-- Bootstrap -->
<link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js"></script>

<!-- Font Awesome -->
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.0/css/all.min.css">
<script src="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.0/js/all.min.js"></script>

<!-- Google Fonts -->
<link href="https://fonts.googleapis.com/css2?family=Syne:wght@600;700;800&family=DM+Sans:ital,wght@0,300;0,400;0,500;0,600;1,400&family=JetBrains+Mono:wght@400;500&display=swap" rel="stylesheet">
```

### 3. Combinação Local + CDN

```html
<!-- Prioridade: Local, Fallback: CDN -->
<link rel="stylesheet" href="css/bootstrap.min.css" onerror="this.href='https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css'">
<link rel="stylesheet" href="css/fontawesome.min.css" onerror="this.href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.0/css/all.min.css'">
```

## 📊 Manifesto do Pacote

O arquivo `manifest.json` contém:

```json
{
  "name": "canalqb-esp32-cdn",
  "version": "1.0.0",
  "built": "2026-04-20T...",
  "modules": [
    {
      "name": "bootstrap",
      "version": "5.3.3",
      "files": ["css/bootstrap.min.css", "js/bootstrap.bundle.min.js"]
    },
    {
      "name": "@fortawesome/fontawesome-free", 
      "version": "6.5.0",
      "files": ["css/fontawesome.min.css", "js/fontawesome.min.js"]
    },
    {
      "name": "google-fonts",
      "files": ["css/google-fonts.css"]
    }
  ]
}
```

## 🔄 Build Process

O pacote é gerado automaticamente usando:

```bash
# Instalar dependências
npm install

# Gerar pacote CDN
npm run build:cdn

# Ou executar manualmente
node build-cdn.js
```

## 🎯 Benefícios

1. **Performance** - Carregamento mais rápido sem múltiplas requisições
2. **Offline** - Funciona mesmo sem internet após primeiro carregamento
3. **Cache** - Melhor aproveitamento de cache do navegador
4. **Otimização** - Arquivos minificados e comprimidos
5. **Controle** - Versões fixas evitam quebras de compatibilidade

## 📝 Versões

- **Bootstrap**: 5.3.3 (última estável)
- **Font Awesome**: 6.5.0 (última estável)
- **Build**: CanalQb ESP32 v1.0.0

---

@CanalQb 2026 - Todos os direitos reservados
