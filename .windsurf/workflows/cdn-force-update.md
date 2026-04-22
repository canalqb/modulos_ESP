---
description: Forçar atualizações do CDN jsDelivr a cada 1 hora e 30 minutos
---

# CDN Force Update Workflow

Este workflow força atualizações completas do CDN jsDelivr para garantir que todos os arquivos mais recentes sejam servidos aos usuários.

## Quando Usar

Execute este workflow quando:
- Arquivos não estão atualizando no CDN
- Mudanças recentes não aparecem para usuários
- Cache do CDN está persistente
- Necessário forçar propagação imediata

## Frequência Recomendada

**Automático:** A cada 1 hora e 30 minutos (90 minutos)
**Manual:** Quando necessário para atualizações críticas

## Passos do Workflow

### 1. Verificar Status Atual do GitHub
```bash
# Verificar commits mais recentes
git log --oneline -5

# Verificar arquivos modificados
git status
```

### 2. Forçar Purge do CDN jsDelivr

#### Purge Geral do Repositório
```bash
# Usar curl para forçar purge completo
curl -X POST "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main"
```

#### Purge de Arquivos Específicos
```bash
# Forçar purge do index.html
curl -X POST "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main/ambiente/index.html"

# Forçar purge do style.css
curl -X POST "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main/ambiente/style.css"

# Forçar purge do components.css
curl -X POST "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main/ambiente/components.css"

# Forçar purge dos scripts JavaScript
curl -X POST "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main/ambiente/secure-config.js"
curl -X POST "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main/ambiente/ga4-gtm-integration.js"
```

### 3. Verificar Status do Purge
```bash
# Verificar status do purge geral
curl "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main"

# Verificar status de arquivos específicos
curl "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main/ambiente/index.html"
```

### 4. Testar Carregamento dos Arquivos
```bash
# Testar via CDN
curl -I "https://cdn.jsdelivr.net/gh/canalqb/modulos_ESP@main/ambiente/index.html"

# Verificar conteúdo atualizado
curl "https://cdn.jsdelivr.net/gh/canalqb/modulos_ESP@main/ambiente/index.html" | head -20
```

### 5. Validar Timestamps e Versões
```bash
# Verificar timestamp do CDN
curl -s "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main" | jq '.timestamp'

# Comparar com timestamp do GitHub
git log -1 --format="%ct"
```

## Automatização (Script)

### Script Bash para Execução Automática
```bash
#!/bin/bash
# cdn-force-update.sh

echo "=== CDN Force Update Workflow ==="
echo "Iniciando em: $(date)"

# Lista de arquivos para forçar atualização
FILES=(
    "ambiente/index.html"
    "ambiente/style.css"
    "ambiente/components.css"
    "ambiente/secure-config.js"
    "ambiente/ga4-gtm-integration.js"
    "ambiente/sites-externos.html"
    "ambiente/ads.txt"
)

# 1. Forçar purge geral do repositório
echo "1. Forçando purge geral do repositório..."
curl -s -X POST "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main" > /dev/null

# 2. Forçar purge de cada arquivo
echo "2. Forçando purge de arquivos específicos..."
for file in "${FILES[@]}"; do
    echo "   - Purging: $file"
    curl -s -X POST "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main/$file" > /dev/null
done

# 3. Verificar status
echo "3. Verificando status do purge..."
sleep 5
STATUS=$(curl -s "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main" | jq -r '.status')
echo "   Status: $STATUS"

# 4. Testar carregamento
echo "4. Testando carregamento dos arquivos..."
for file in "${FILES[@]}"; do
    HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" "https://cdn.jsdelivr.net/gh/canalqb/modulos_ESP@main/$file")
    echo "   - $file: HTTP $HTTP_CODE"
done

echo "=== Workflow concluído em: $(date) ==="
```

## Agendamento Automático

### Usando Cron (Linux/Mac)
```bash
# Editar crontab
crontab -e

# Adicionar linha para executar a cada 90 minutos
*/90 * * * * /path/to/cdn-force-update.sh >> /var/log/cdn-update.log 2>&1
```

### Usando Task Scheduler (Windows)
```powershell
# Criar tarefa agendada
$action = New-ScheduledTaskAction -Execute "powershell.exe" -Argument "-File C:\path\to\cdn-force-update.ps1"
$trigger = New-ScheduledTaskTrigger -Once -At (Get-Date) -RepetitionInterval (New-TimeSpan -Minutes 90) -RepetitionDuration ([TimeSpan]::MaxValue)
Register-ScheduledTask -Action $action -Trigger $trigger -TaskName "CDN Force Update" -Description "Força atualizações do CDN jsDelivr a cada 90 minutos"
```

### Script PowerShell para Windows
```powershell
# cdn-force-update.ps1
Write-Host "=== CDN Force Update Workflow ==="
Write-Host "Iniciando em: $(Get-Date)"

$files = @(
    "ambiente/index.html",
    "ambiente/style.css", 
    "ambiente/components.css",
    "ambiente/secure-config.js",
    "ambiente/ga4-gtm-integration.js",
    "ambiente/sites-externos.html",
    "ambiente/ads.txt"
)

# Forçar purge geral
Write-Host "1. Forçando purge geral..."
Invoke-RestMethod -Uri "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main" -Method POST | Out-Null

# Forçar purge de arquivos
Write-Host "2. Forçando purge de arquivos..."
foreach ($file in $files) {
    Write-Host "   - Purging: $file"
    try {
        Invoke-RestMethod -Uri "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main/$file" -Method POST | Out-Null
    } catch {
        Write-Host "   Erro ao purgar $file : $($_.Exception.Message)"
    }
}

# Verificar status
Write-Host "3. Verificando status..."
Start-Sleep -Seconds 5
try {
    $response = Invoke-RestMethod -Uri "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main"
    Write-Host "   Status: $($response.status)"
} catch {
    Write-Host "   Erro ao verificar status"
}

# Testar carregamento
Write-Host "4. Testando carregamento..."
foreach ($file in $files) {
    try {
        $response = Invoke-WebRequest -Uri "https://cdn.jsdelivr.net/gh/canalqb/modulos_ESP@main/$file" -UseBasicParsing
        Write-Host "   - $file : HTTP $($response.StatusCode)"
    } catch {
        Write-Host "   - $file : Erro - $($_.Exception.Message)"
    }
}

Write-Host "=== Workflow concluído em: $(Get-Date) ==="
```

## Monitoramento

### Logs de Execução
```bash
# Verificar logs
tail -f /var/log/cdn-update.log

# Filtrar erros
grep "ERROR\|Erro" /var/log/cdn-update.log
```

### Alertas (Opcional)
```bash
# Adicionar ao script para enviar alertas em caso de falha
if [ $? -ne 0 ]; then
    echo "Falha no CDN update: $(date)" | mail -s "CDN Update Failed" admin@canalqb.com.br
fi
```

## Troubleshooting

### Problemas Comuns
1. **Purge falhando:** Verificar conectividade com jsDelivr
2. **Arquivos não atualizando:** Aumentar tempo de espera entre purges
3. **Rate limiting:** Espaçar requisições em pelo menos 1 segundo

### Soluções
```bash
# Verificar conectividade
ping purge.jsdelivr.net

# Testar manualmente um arquivo
curl -v "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main/ambiente/index.html"

# Limpar cache local se necessário
curl -H "Cache-Control: no-cache" "https://cdn.jsdelivr.net/gh/canalqb/modulos_ESP@main/ambiente/index.html"
```

## Resultados Esperados

Após executar este workflow:
- Todos os arquivos do CDN estarão atualizados
- Cache antigo será limpo globalmente
- Usuários receberão a versão mais recente
- Logs registrarão sucesso das operações

## Frequência de Execução

- **Produção:** A cada 90 minutos (1h 30min)
- **Desenvolvimento:** Após cada commit importante
- **Emergência:** Imediato quando necessário

Este workflow garante que o CDN jsDelivr sempre sirva as versões mais recentes dos arquivos do CanalQB ESP32.
