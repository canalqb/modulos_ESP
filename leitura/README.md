# Pasta de Leitura - CanalQB ESP32

## Estrutura de Documentação

Esta pasta contém toda a documentação e conhecimento do projeto CanalQB ESP32, organizado seguindo as diretrizes do `master_rules.md`.

## Arquivos de Documentação

### Documentação Principal
- **ESP32_CanalQb_Documento_Tecnico.md** - Documentação técnica completa do projeto
- **ESP32_CanalQb_Prompt_LLM.md** - Prompts e configurações para LLMs
- **ACOMPANHAMENTO_PROJETO.md** - Acompanhamento e progresso do projeto
- **ultima_conversa_ga4.md** - Registro da última conversa com GA4

### Documentação do Repositório
- **modulos_ESP_repo/docs/README.md** - Visão geral do repositório
- **modulos_ESP_repo/docs/conhecimento-compartilhado.md** - Histórico de interações e soluções

### Documentação de Firmware
- **firmware/README.md** - Documentação do firmware ESP32

### Documentação de Distribuição
- **dist/README.md** - Documentação dos arquivos de distribuição
- **dist/ambiente/README.md** - Documentação do ambiente distribuído

## Regras de Organização

### Arquivos Mantidos em `regras/`
Os seguintes arquivos NÃO foram movidos pois pertencem à pasta `regras/` que é imutável:
- `regras/master_rules.md` - Regra mestre do sistema
- `regras/log_solucoes.md` - Log de soluções
- `regras/arquivos_ausentes_readme.md` - Arquivos ausentes
- `regras/analise_arquivos_vs_readme.md` - Análise de arquivos
- Todos os arquivos em `regras/templates_php/`
- Todos os arquivos em `regras/prompts_php/`
- Todos os arquivos em `regras/prompt_de_llms/`

### Arquivos Ignorados
Os seguintes arquivos foram ignorados por serem dependências de terceiros:
- Todos os arquivos em `node_modules/`

## Estrutura Final

```
leitura/
README.md (este arquivo)
ESP32_CanalQb_Documento_Tecnico.md
ESP32_CanalQb_Prompt_LLM.md
ACOMPANHAMENTO_PROJETO.md
ultima_conversa_ga4.md

modulos_ESP_repo/
docs/README.md
docs/conhecimento-compartilhado.md

firmware/README.md

dist/README.md
dist/ambiente/README.md
```

## Observações

- Esta organização segue a Regra 17 do `master_rules.md`
- Todos os arquivos são agora acessíveis centralmente
- A estrutura facilita o onboarding de novos desenvolvedores
- Mantém-se a integridade da pasta `regras/` conforme especificado
