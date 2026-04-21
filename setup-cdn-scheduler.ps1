# Setup CDN Scheduler Script (PowerShell)
# Configura agendamento automático para executar o CDN Force Update a cada 90 minutos

Write-Host "=== Setup CDN Scheduler ===" -ForegroundColor Green
Write-Host "Configurando agendamento automático para CDN Force Update..." -ForegroundColor Cyan

# Verificar se o script existe
$scriptPath = Join-Path $PSScriptRoot "cdn-force-update.ps1"
if (-not (Test-Path $scriptPath)) {
    Write-Host "Erro: Script $scriptPath não encontrado!" -ForegroundColor Red
    exit 1
}

Write-Host "Script encontrado: $scriptPath" -ForegroundColor Green

# Criar diretório de log
$logDir = "C:\temp"
if (-not (Test-Path $logDir)) {
    try {
        New-Item -ItemType Directory -Path $logDir -Force | Out-Null
        Write-Host "Diretório de log criado: $logDir" -ForegroundColor Green
    } catch {
        Write-Host "Erro ao criar diretório de log: $($_.Exception.Message)" -ForegroundColor Red
        exit 1
    }
}

$logFile = Join-Path $logDir "cdn-update.log"
Write-Host "Arquivo de log: $logFile" -ForegroundColor Green

# Verificar se o Task Scheduler está disponível
try {
    Get-ScheduledTask -TaskName "CDN Force Update" -ErrorAction SilentlyContinue | Out-Null
} catch {
    Write-Host "Erro: Task Scheduler não está disponível!" -ForegroundColor Red
    exit 1
}

# Remover tarefa existente se houver
try {
    $existingTask = Get-ScheduledTask -TaskName "CDN Force Update" -ErrorAction SilentlyContinue
    if ($existingTask) {
        Unregister-ScheduledTask -TaskName "CDN Force Update" -Confirm:$false
        Write-Host "Tarefa existente removida" -ForegroundColor Yellow
    }
} catch {
    # Tarefa não existe, continuar
}

# Criar action para executar o script PowerShell
$action = New-ScheduledTaskAction -Execute "powershell.exe" -Argument "-NonInteractive -WindowStyle Hidden -File `"$scriptPath`""

# Criar trigger para executar a cada 90 minutos
$trigger = New-ScheduledTaskTrigger -Once -At (Get-Date) -RepetitionInterval (New-TimeSpan -Minutes 90) -RepetitionDuration ([TimeSpan]::MaxValue)

# Criar configurações da tarefa
$settings = New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries -StartWhenAvailable -RunOnlyIfNetworkAvailable

# Registrar tarefa agendada
try {
    Register-ScheduledTask -Action $action -Trigger $trigger -Settings $settings -TaskName "CDN Force Update" -Description "Força atualizações do CDN jsDelivr a cada 90 minutos para CanalQB ESP32" -RunLevel Highest | Out-Null
    Write-Host "Tarefa agendada criada com sucesso!" -ForegroundColor Green
} catch {
    Write-Host "Erro ao criar tarefa agendada: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

# Verificar configuração
Write-Host "Verificando configuração da tarefa:" -ForegroundColor Cyan
try {
    $task = Get-ScheduledTask -TaskName "CDN Force Update"
    Write-Host "Nome: $($task.TaskName)" -ForegroundColor White
    Write-Host "Descrição: $($task.Description)" -ForegroundColor White
    Write-Host "Estado: $($task.State)" -ForegroundColor White
    Write-Host "Próxima execução: $($task.Triggers.StartBoundary)" -ForegroundColor White
    Write-Host "Intervalo: 90 minutos" -ForegroundColor White
} catch {
    Write-Host "Erro ao verificar tarefa: $($_.Exception.Message)" -ForegroundColor Red
}

# Testar execução imediata
Write-Host ""
Write-Host "Deseja testar execução imediata do script? (S/N)" -ForegroundColor Yellow
$response = Read-Host
if ($response -eq "S" -or $response -eq "s") {
    Write-Host "Executando teste..." -ForegroundColor Cyan
    try {
        & $scriptPath
    } catch {
        Write-Host "Erro durante execução do teste: $($_.Exception.Message)" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "=== Setup concluído ===" -ForegroundColor Green
Write-Host "O CDN Force Update será executado automaticamente a cada 90 minutos" -ForegroundColor Cyan
Write-Host "Logs disponíveis em: $logFile" -ForegroundColor Cyan
Write-Host ""
Write-Host "Comandos úteis:" -ForegroundColor Yellow
Write-Host "  Verificar logs: Get-Content $logFile -Tail 10" -ForegroundColor White
Write-Host "  Verificar tarefa: Get-ScheduledTask -TaskName 'CDN Force Update'" -ForegroundColor White
Write-Host "  Executar manualmente: Start-ScheduledTask -TaskName 'CDN Force Update'" -ForegroundColor White
Write-Host "  Remover tarefa: Unregister-ScheduledTask -TaskName 'CDN Force Update' -Confirm:`$false" -ForegroundColor White
