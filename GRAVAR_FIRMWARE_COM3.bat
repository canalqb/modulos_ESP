@echo off
title CanalQb ESP32 - Gravador de Firmware
color 0b

echo ============================================================
echo        CANALQB - GRAVADOR AUTOMATICO DE FIRMWARE
echo ============================================================
echo Porta Detectada: COM3
echo Projeto: firmware
echo.
echo [REGRA OBRIGATORIA] Formatando memoria flash antes da gravacao...
echo.
echo Operacao 1/2: Apagando memoria flash (Erase)...
cd /d "%~dp0firmware"
call pio run --target erase

if %errorlevel% neq 0 (
    echo.
    echo [ERRO] Falha ao apagar a flash. Verifique a conexao e a porta COM3.
    pause
    exit /b %errorlevel%
)

echo.
echo Operacao 2/2: Gravando Firmware (Upload)...
call pio run --target upload

if %errorlevel% neq 0 (
    echo.
    echo [ERRO] Falha na gravacao. Verifique os erros de compilacao acima.
    pause
    exit /b %errorlevel%
)

echo.
echo ============================================================
echo [SUCESSO] Firmware gravado corretamente!
echo Abrindo monitor serial em 3 segundos...
echo ============================================================
timeout /t 3 > nul
call pio device monitor

pause
