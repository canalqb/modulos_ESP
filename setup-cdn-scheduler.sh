#!/bin/bash
# Setup CDN Scheduler Script
# Configura agendamento automático para executar o CDN Force Update a cada 90 minutos

echo "=== Setup CDN Scheduler ==="
echo "Configurando agendamento automático para CDN Force Update..."

# Verificar se o script existe
SCRIPT_PATH="$(pwd)/cdn-force-update.sh"
if [ ! -f "$SCRIPT_PATH" ]; then
    echo "Erro: Script $SCRIPT_PATH não encontrado!"
    exit 1
fi

# Tornar script executável
chmod +x "$SCRIPT_PATH"
echo "Script tornado executável: $SCRIPT_PATH"

# Criar arquivo de log
sudo mkdir -p /var/log
sudo touch /var/log/cdn-update.log
sudo chmod 666 /var/log/cdn-update.log
echo "Arquivo de log criado: /var/log/cdn-update.log"

# Configurar crontab
TEMP_CRON=$(mktemp)
echo "Configurando crontab..."

# Adicionar entrada ao crontab (executar a cada 90 minutos)
echo "# CDN Force Update - CanalQB ESP32" > "$TEMP_CRON"
echo "*/90 * * * * $SCRIPT_PATH >> /var/log/cdn-update.log 2>&1" >> "$TEMP_CRON"

# Adicionar crontab existente (se houver)
crontab -l 2>/dev/null | grep -v "cdn-force-update" >> "$TEMP_CRON" 2>/dev/null || true

# Instalar novo crontab
crontab "$TEMP_CRON"
rm "$TEMP_CRON"

echo "Crontab configurado para executar a cada 90 minutos"
echo ""

# Verificar configuração
echo "Verificando configuração:"
crontab -l | grep "cdn-force-update"
echo ""

# Testar execução imediata
echo "Deseja testar execução imediata do script? (s/n)"
read -r response
if [[ "$response" =~ ^[Ss]$ ]]; then
    echo "Executando teste..."
    ./cdn-force-update.sh
fi

echo ""
echo "=== Setup concluído ==="
echo "O CDN Force Update será executado automaticamente a cada 90 minutos"
echo "Logs disponíveis em: /var/log/cdn-update.log"
echo ""
echo "Para verificar logs: tail -f /var/log/cdn-update.log"
echo "Para remover agendamento: crontab -e (remover linha do cdn-force-update)"
