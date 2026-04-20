#!/usr/bin/env node

/**
 * CanalQb ESP32 CDN Package Builder
 * Gera pacotes otimizados usando jsDelivr CDN
 */

import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Configuração dos módulos CDN
const cdnModules = [
  {
    name: 'bootstrap',
    version: '5.3.3',
    files: [
      {
        from: 'https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css',
        to: 'css/bootstrap.min.css'
      },
      {
        from: 'https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.bundle.min.js',
        to: 'js/bootstrap.bundle.min.js'
      }
    ]
  },
  {
    name: '@fortawesome/fontawesome-free',
    version: '6.5.0',
    files: [
      {
        from: 'https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.0/css/all.min.css',
        to: 'css/fontawesome.min.css'
      },
      {
        from: 'https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.0/js/all.min.js',
        to: 'js/fontawesome.min.js'
      }
    ]
  },
  {
    name: 'google-fonts',
    files: [
      {
        from: 'https://fonts.googleapis.com/css2?family=Syne:wght@600;700;800&family=DM+Sans:ital,wght@0,300;0,400;0,500;0,600;1,400&family=JetBrains+Mono:wght@400;500&display=swap',
        to: 'css/google-fonts.css'
      }
    ]
  }
];

// Função para baixar arquivo
async function downloadFile(url, outputPath) {
  try {
    const response = await fetch(url);
    if (!response.ok) {
      throw new Error(`Failed to fetch ${url}: ${response.statusText}`);
    }
    
    const buffer = await response.arrayBuffer();
    const uint8Array = new Uint8Array(buffer);
    
    // Garantir que o diretório exista
    const dir = path.dirname(outputPath);
    if (!fs.existsSync(dir)) {
      fs.mkdirSync(dir, { recursive: true });
    }
    
    fs.writeFileSync(outputPath, uint8Array);
    console.log(`✓ Downloaded: ${path.basename(outputPath)}`);
  } catch (error) {
    console.error(`✗ Error downloading ${url}:`, error.message);
  }
}

// Função principal
async function buildCDN() {
  console.log('🚀 Building CanalQb ESP32 CDN Package...\n');
  
  const distDir = 'dist';
  
  // Limpar diretório dist
  if (fs.existsSync(distDir)) {
    fs.rmSync(distDir, { recursive: true });
  }
  
  // Criar estrutura de diretórios
  fs.mkdirSync(path.join(distDir, 'css'), { recursive: true });
  fs.mkdirSync(path.join(distDir, 'js'), { recursive: true });
  fs.mkdirSync(path.join(distDir, 'fonts'), { recursive: true });
  
  // Baixar todos os módulos
  for (const module of cdnModules) {
    console.log(`\n📦 Processing module: ${module.name}`);
    
    for (const file of module.files) {
      await downloadFile(file.from, path.join(distDir, file.to));
    }
  }
  
  // Criar arquivo de manifesto
  const manifest = {
    name: 'canalqb-esp32-cdn',
    version: '1.0.0',
    built: new Date().toISOString(),
    modules: cdnModules.map(m => ({
      name: m.name,
      version: m.version,
      files: m.files.map(f => f.to)
    }))
  };
  
  fs.writeFileSync(
    path.join(distDir, 'manifest.json'),
    JSON.stringify(manifest, null, 2)
  );
  
  // Criar HTML de exemplo
  const exampleHTML = `<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CanalQb ESP32 - CDN Test</title>
    
    <!-- CDN Local Files -->
    <link rel="stylesheet" href="css/bootstrap.min.css">
    <link rel="stylesheet" href="css/fontawesome.min.css">
    <link rel="stylesheet" href="css/google-fonts.css">
    <link rel="stylesheet" href="style.css">
</head>
<body>
    <div class="container">
        <h1>CanalQb ESP32 CDN Package</h1>
        <p>Dependências externas hospedadas localmente via jsDelivr</p>
        
        <div class="alert alert-success">
            ✓ Bootstrap CSS/JS carregados localmente
        </div>
        
        <div class="alert alert-info">
            ✓ Font Awesome ícones carregados localmente  
        </div>
        
        <div class="alert alert-warning">
            ✓ Google Fonts carregadas localmente
        </div>
    </div>
    
    <script src="js/bootstrap.bundle.min.js"></script>
    <script src="js/fontawesome.min.js"></script>
</body>
</html>`;
  
  fs.writeFileSync(path.join(distDir, 'index.html'), exampleHTML);
  
  console.log('\n✅ CDN Package built successfully!');
  console.log(`📁 Output directory: ${distDir}`);
  console.log('\n📋 Generated files:');
  
  function listFiles(dir, prefix = '') {
    const files = fs.readdirSync(dir);
    files.forEach(file => {
      const filePath = path.join(dir, file);
      const stat = fs.statSync(filePath);
      
      if (stat.isDirectory()) {
        console.log(`${prefix}📁 ${file}/`);
        listFiles(filePath, prefix + '  ');
      } else {
        console.log(`${prefix}📄 ${file}`);
      }
    });
  }
  
  listFiles(distDir);
}

// Executar build
buildCDN().catch(console.error);
