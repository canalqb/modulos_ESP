#!/bin/bash
# CDN Force Update Script for CanalQB ESP32
# Executa a cada 90 minutos para forçar atualizações do CDN jsDelivr

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
GENERAL_PURGE_RESPONSE=$(curl -s -X POST "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main")
echo "   Resposta: $GENERAL_PURGE_RESPONSE"

# 2. Forçar purge de cada arquivo
echo "2. Forçando purge de arquivos específicos..."
for file in "${FILES[@]}"; do
    echo "   - Purging: $file"
    PURGE_RESPONSE=$(curl -s -X POST "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main/$file")
    if [ $? -eq 0 ]; then
        echo "     Sucesso: $(echo $PURGE_RESPONSE | jq -r '.id // "completed"' 2>/dev/null || echo "completed")"
    else
        echo "     Erro ao purgar $file"
    fi
    sleep 1  # Evitar rate limiting
done

# 3. Verificar status do purge
echo "3. Verificando status do purge..."
sleep 5
STATUS_RESPONSE=$(curl -s "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main")
if command -v jq &> /dev/null; then
    STATUS=$(echo "$STATUS_RESPONSE" | jq -r '.status // "unknown"' 2>/dev/null)
    TIMESTAMP=$(echo "$STATUS_RESPONSE" | jq -r '.timestamp // "unknown"' 2>/dev/null)
    echo "   Status: $STATUS"
    echo "   Timestamp: $TIMESTAMP"
else
    echo "   Status: $(echo "$STATUS_RESPONSE" | grep -o '"status":"[^"]*"' | cut -d'"' -f4 || echo "unknown")"
fi

# 4. Testar carregamento dos arquivos
echo "4. Testando carregamento dos arquivos..."
SUCCESS_COUNT=0
TOTAL_COUNT=${#FILES[@]}

for file in "${FILES[@]}"; do
    HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" "https://cdn.jsdelivr.net/gh/canalqb/modulos_ESP@main/$file")
    if [ "$HTTP_CODE" = "200" ]; then
        echo "   - $file: HTTP $HTTP_CODE [OK]"
        SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
    else
        echo "   - $file: HTTP $HTTP_CODE [ERROR]"
    fi
    sleep 0.5
done

# 5. Resumo
echo "5. Resumo da execução:"
echo "   Arquivos testados: $TOTAL_COUNT"
echo "   Arquivos OK: $SUCCESS_COUNT"
echo "   Taxa de sucesso: $((SUCCESS_COUNT * 100 / TOTAL_COUNT))%"

if [ $SUCCESS_COUNT -eq $TOTAL_COUNT ]; then
    echo "   Status: SUCESSO - Todos os arquivos atualizados"
    EXIT_CODE=0
else
    echo "   Status: PARCIAL - Alguns arquivos podem não estar atualizados"
    EXIT_CODE=1
fi

echo "=== Workflow concluído em: $(date) ==="
echo ""

# Log para arquivo de log
LOG_FILE="/var/log/cdn-update.log"
echo "$(date): CDN Update - Success: $SUCCESS_COUNT/$TOTAL_COUNT" >> "$LOG_FILE" 2>/dev/null

exit $EXIT_CODE
