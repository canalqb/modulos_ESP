@echo off
chcp 65001 >nul
title CanalQb - Sincronizacao com GitHub
color 0A

echo.
echo  ============================================
echo  =   CanalQb - Sincronizacao com GitHub    =
echo  =   @CanalQb ESP32 Repository             =
echo  ============================================
echo.

REM Define o caminho para o repositorio
set REPO_PATH=%~dp0modulos_ESP_repo

REM Verifica se o diretorio do repositorio existe
if not exist "%REPO_PATH%" (
    echo  [ERRO] Diretorio do repositorio nao encontrado!
    echo  [ERRO] %REPO_PATH%
    pause
    exit /b 1
)

REM Verifica se e um repositorio Git
if not exist "%REPO_PATH%\.git" (
    echo  [ERRO] Este nao e um repositorio Git!
    echo  [ERRO] %REPO_PATH%
    pause
    exit /b 1
)

echo  [INFO] Repositorio encontrado em: %REPO_PATH%
echo  [INFO] Iniciando sincronizacao com GitHub...
echo.

REM Muda para o diretorio do repositorio
cd /d "%REPO_PATH%"

REM Etapa 1: Verificar status atual
echo  [1/5] Verificando status do repositorio...
git status --porcelain
echo.

REM Etapa 2: Adicionar todas as alteracoes
echo  [2/5] Adicionando arquivos modificados/novos/deletados...
git add .
if %errorlevel% neq 0 (
    echo  [ERRO] Falha ao adicionar arquivos!
    pause
    exit /b 1
)
echo  [OK] Arquivos adicionados com sucesso!
echo.

REM Etapa 3: Verificar se ha alteracoes para commit
git diff --cached --quiet
if %errorlevel% equ 0 (
    echo  [INFO] Nenhuma alteracao encontrada para commit.
    echo  [INFO] Repositorio ja esta sincronizado.
    pause
    exit /b 0
)

REM Etapa 4: Fazer commit
echo  [3/5] Criando commit...
set /p commit_msg="Digite a mensagem do commit (ou Enter para usar padrao): "
if "%commit_msg%"=="" (
    set commit_msg=Auto-sync from root: %date% %time%
)

git commit -m "%commit_msg%"
if %errorlevel% neq 0 (
    echo  [ERRO] Falha ao criar commit!
    pause
    exit /b 1
)
echo  [OK] Commit criado com sucesso!
echo.

REM Etapa 5: Push para GitHub
echo  [4/5] Enviando alteracoes para GitHub...
git push https://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@github.com/canalqb/modulos_ESP.git main --force
if %errorlevel% neq 0 (
    echo  [ERRO] Falha no push para GitHub!
    pause
    exit /b 1
)
echo  [OK] Push realizado com sucesso!
echo.

REM Etapa 6: Verificar status final
echo  [5/5] Verificando status final...
git status --porcelain
echo.

echo  ============================================
echo  =   Sincronizacao concluida com sucesso!   =
echo  ============================================
echo.
echo  [INFO] Todos os arquivos foram sincronizados com GitHub.
echo  [INFO] O repositorio local e remoto estao identicos.
echo  [INFO] Arquivos: criados, modificados, deletados
echo.

timeout /t 3 >nul
exit /b 0
