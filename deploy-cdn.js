#!/usr/bin/env node

/**
 * Deploy para jsDelivr CDN
 * Faz upload dos arquivos para o GitHub e disponibiliza via CDN
 */

import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';
import { execSync } from 'child_process';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Configurações
const config = {
  repo: 'canalqb/modulos_ESP',
  branch: 'main',
  sourceDir: './dist',
  targetDir: 'ambiente',
  token: process.env.GITHUB_TOKEN || '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'
};

// Função para executar comandos
function runCommand(command, description) {
  console.log(`\n🔄 ${description}...`);
  try {
    const result = execSync(command, { stdio: 'inherit' });
    console.log(`✅ ${description} concluído`);
    return result;
  } catch (error) {
    console.error(`❌ Erro em ${description}:`, error.message);
    process.exit(1);
  }
}

// Função principal de deploy
async function deploy() {
  console.log('🚀 Iniciando deploy para jsDelivr CDN...\n');
  
  // Verificar se o diretório dist existe
  if (!fs.existsSync(config.sourceDir)) {
    console.error('❌ Diretório dist não encontrado. Execute "npm run build:cdn" primeiro.');
    process.exit(1);
  }
  
  // Configurar Git se necessário
  if (!fs.existsSync('.git')) {
    console.log('📁 Inicializando repositório Git...');
    runCommand('git init', 'Git init');
    runCommand('git remote add origin https://github.com/canalqb/modulos_ESP.git', 'Add remote');
  }
  
  // Copiar arquivos para o ambiente
  const ambienteDir = path.join(config.sourceDir, config.targetDir);
  if (!fs.existsSync(ambienteDir)) {
    fs.mkdirSync(ambienteDir, { recursive: true });
  }
  
  // Copiar CSS para ambiente
  const cssSource = path.join(__dirname, 'modulos_ESP_repo', 'ambiente', 'style.css');
  const cssTarget = path.join(ambienteDir, 'style.css');
  
  if (fs.existsSync(cssSource)) {
    fs.copyFileSync(cssSource, cssTarget);
    console.log('✅ CSS copiado para ambiente/');
  }
  
  // Copiar outros arquivos do dist
  const distFiles = fs.readdirSync(config.sourceDir);
  for (const file of distFiles) {
    if (file !== config.targetDir) {
      const sourcePath = path.join(config.sourceDir, file);
      const targetPath = path.join(ambienteDir, file);
      
      if (fs.statSync(sourcePath).isDirectory()) {
        if (!fs.existsSync(targetPath)) {
          fs.mkdirSync(targetPath, { recursive: true });
        }
        // Copiar recursivamente
        copyRecursive(sourcePath, targetPath);
      } else {
        fs.copyFileSync(sourcePath, targetPath);
      }
    }
  }
  
  // Configurar credenciais do Git
  runCommand(`git config user.name "CanalQb"`, 'Config user name');
  runCommand(`git config user.email "qrodrigob@gmail.com"`, 'Config user email');
  
  // Adicionar todos os arquivos
  runCommand(`git add .`, 'Add files');
  
  // Commit
  const timestamp = new Date().toISOString().replace(/[:.]/, '-').split('.')[0];
  runCommand(`git commit -m "Update CDN package - ${timestamp}"`, 'Commit changes');
  
  // Push para o GitHub
  console.log('\n📤 Enviando para GitHub...');
  const pushCommand = `git push https://${config.token}@github.com/canalqb/modulos_ESP.git ${config.branch}`;
  runCommand(pushCommand, 'Push to GitHub');
  
  // URLs CDN
  console.log('\n🌐 URLs CDN disponíveis:');
  console.log(`CSS: https://cdn.jsdelivr.net/gh/canalqb/modulos_ESP@${config.branch}/ambiente/style.css`);
  console.log(`Bootstrap CSS: https://cdn.jsdelivr.net/gh/canalqb/modulos_ESP@${config.branch}/ambiente/css/bootstrap.min.css`);
  console.log(`Bootstrap JS: https://cdn.jsdelivr.net/gh/canalqb/modulos_ESP@${config.branch}/ambiente/js/bootstrap.bundle.min.js`);
  console.log(`Font Awesome CSS: https://cdn.jsdelivr.net/gh/canalqb/modulos_ESP@${config.branch}/ambiente/css/fontawesome.min.css`);
  console.log(`Font Awesome JS: https://cdn.jsdelivr.net/gh/canalqb/modulos_ESP@${config.branch}/ambiente/js/fontawesome.min.js`);
  
  console.log('\n✅ Deploy concluído com sucesso!');
  console.log('📦 Pacote disponível via jsDelivr CDN');
}

// Função para copiar diretórios recursivamente
function copyRecursive(source, target) {
  const files = fs.readdirSync(source);
  for (const file of files) {
    const sourcePath = path.join(source, file);
    const targetPath = path.join(target, file);
    
    if (fs.statSync(sourcePath).isDirectory()) {
      if (!fs.existsSync(targetPath)) {
        fs.mkdirSync(targetPath, { recursive: true });
      }
      copyRecursive(sourcePath, targetPath);
    } else {
      fs.copyFileSync(sourcePath, targetPath);
    }
  }
}

// Executar deploy
deploy().catch(console.error);
