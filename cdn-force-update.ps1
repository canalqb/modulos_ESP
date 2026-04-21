# CDN Force Update Script for CanalQB ESP32 (PowerShell)
# Executa a cada 90 minutos para forçar atualizações do CDN jsDelivr

Write-Host "=== CDN Force Update Workflow ===" -ForegroundColor Green
Write-Host "Iniciando em: $(Get-Date)" -ForegroundColor Cyan

# Lista de arquivos para forçar atualização
$files = @(
    "ambiente/index.html",
    "ambiente/style.css", 
    "ambiente/components.css",
    "ambiente/secure-config.js",
    "ambiente/ga4-gtm-integration.js",
    "ambiente/sites-externos.html",
    "ambiente/ads.txt"
)

# 1. Forçar purge geral do repositório
Write-Host "1. Forçando purge geral do repositório..." -ForegroundColor Yellow
try {
    $generalResponse = Invoke-RestMethod -Uri "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main" -Method POST
    Write-Host "   Resposta: $($generalResponse.id)" -ForegroundColor Green
} catch {
    Write-Host "   Erro ao fazer purge geral: $($_.Exception.Message)" -ForegroundColor Red
}

# 2. Forçar purge de cada arquivo
Write-Host "2. Forçando purge de arquivos específicos..." -ForegroundColor Yellow
foreach ($file in $files) {
    Write-Host "   - Purging: $file" -ForegroundColor White
    try {
        $purgeResponse = Invoke-RestMethod -Uri "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main/$file" -Method POST
        Write-Host "     Sucesso: $($purgeResponse.id)" -ForegroundColor Green
    } catch {
        Write-Host "     Erro ao purgar $file : $($_.Exception.Message)" -ForegroundColor Red
    }
    Start-Sleep -Seconds 1  # Evitar rate limiting
}

# 3. Verificar status do purge
Write-Host "3. Verificando status do purge..." -ForegroundColor Yellow
Start-Sleep -Seconds 5
try {
    $statusResponse = Invoke-RestMethod -Uri "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main"
    Write-Host "   Status: $($statusResponse.status)" -ForegroundColor Green
    Write-Host "   Timestamp: $($statusResponse.timestamp)" -ForegroundColor Green
} catch {
    Write-Host "   Erro ao verificar status: $($_.Exception.Message)" -ForegroundColor Red
}

# 4. Testar carregamento dos arquivos
Write-Host "4. Testando carregamento dos arquivos..." -ForegroundColor Yellow
$successCount = 0
$totalCount = $files.Count

foreach ($file in $files) {
    try {
        $response = Invoke-WebRequest -Uri "https://cdn.jsdelivr.net/gh/canalqb/modulos_ESP@main/$file" -UseBasicParsing
        if ($response.StatusCode -eq 200) {
            Write-Host "   - $file : HTTP $($response.StatusCode) [OK]" -ForegroundColor Green
            $successCount++
        } else {
            Write-Host "   - $file : HTTP $($response.StatusCode) [ERROR]" -ForegroundColor Red
        }
    } catch {
        Write-Host "   - $file : Erro - $($_.Exception.Message)" -ForegroundColor Red
    }
    Start-Sleep -Milliseconds 500
}

# 5. Resumo
Write-Host "5. Resumo da execução:" -ForegroundColor Cyan
Write-Host "   Arquivos testados: $totalCount" -ForegroundColor White
Write-Host "   Arquivos OK: $successCount" -ForegroundColor White
$successRate = [math]::Round(($successCount / $totalCount) * 100, 2)
Write-Host "   Taxa de sucesso: $successRate%" -ForegroundColor White

if ($successCount -eq $totalCount) {
    Write-Host "   Status: SUCESSO - Todos os arquivos atualizados" -ForegroundColor Green
    $exitCode = 0
} else {
    Write-Host "   Status: PARCIAL - Alguns arquivos podem não estar atualizados" -ForegroundColor Yellow
    $exitCode = 1
}

Write-Host "=== Workflow concluído em: $(Get-Date) ===" -ForegroundColor Green
Write-Host ""

# Log para arquivo
$logFile = "C:\temp\cdn-update.log"
$logEntry = "$(Get-Date): CDN Update - Success: $successCount/$totalCount"
try {
    Add-Content -Path $logFile -Value $logEntry -ErrorAction SilentlyContinue
} catch {
    Write-Host "   Aviso: Não foi possível escrever no log: $($_.Exception.Message)" -ForegroundColor Yellow
}

exit $exitCode
