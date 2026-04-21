# Tokens de Acesso - CanalQB ESP32
**Data:** 21/04/2026  
**Projeto:** CanalQB ESP32  
**Finalidade:** Documentação offline para referência futura  

---

## **AVISO DE SEGURANÇA**  
**IMPORTANTE:** Este documento contém informações sensíveis e deve ser mantido offline e seguro.  
- **NÃO COMPARTILHAR** em repositórios públicos  
- **NÃO EXPOR** em logs ou documentações online  
- **ACESSO RESTRITO** apenas para desenvolvedores autorizados  
- **ARMAZENAMENTO OFFLINE** em ambiente seguro  

---

## **GitHub Personal Access Tokens**

### **Token Principal - CanalQB ESP32**
- **Tipo:** Personal Access Token (Classic)
- **Acesso:** Full
- **Repositório:** canalqb/modulos_ESP
- **Token:** [ARMAZENADO OFFLINE - VER DOCUMENTO FÍSICO]
- **Propósito:** Automação de push, CDN purge, GitHub Actions
- **Uso:** Push forçado para branch main, configuração de workflows

### **Token Alternativo - GitHub Actions**
- **Tipo:** Personal Access Token (Classic)  
- **Acesso:** Full
- **Repositório:** canalqb/modulos_ESP
- **Token:** [ARMAZENADO OFFLINE - VER DOCUMENTO FÍSICO]
- **Propósito:** Push automatizado via GitHub Actions
- **Uso:** Deploy automático, sincronização com CDN

---

## **Google Cloud Platform - Service Account**

### **Service Account - Analytics**
- **Email:** `llm-533@bloggerwindsurf.iam.gserviceaccount.com`
- **Projeto GCP:** `bloggerwindsurf`
- **Arquivo JSON:** `bloggerwindsurf-0de5154ca681.json`
- **Property ID GA4:** `392122309`
- **Propósito:** Integração Google Analytics 4
- **APIs Necessárias:**
  - `analyticsdata.googleapis.com`
  - `analyticsadmin.googleapis.com`
- **Permissões GA4:** `Viewer` ou `Editor`

---

## **Uso Padrão - Comandos**

### **GitHub Push (Token Principal)**
```bash
git push https://[TOKEN_PRINCIPAL]@github.com/canalqb/modulos_ESP.git main --force
```

### **GitHub Push (Token Actions)**
```bash
git push https://[TOKEN_ACTIONS]@github.com/canalqb/modulos_ESP.git main --force
```

### **GCP CLI - Ativar APIs**
```bash
gcloud services enable analyticsdata.googleapis.com --project=bloggerwindsurf
gcloud services enable analyticsadmin.googleapis.com --project=bloggerwindsurf
```

### **GCP CLI - Autenticar Service Account**
```bash
gcloud auth activate-service-account --key-file=bloggerwindsurf-0de5154ca681.json
```

---

## **CDN jsDelivr - URLs de Referência**

### **Purge do Repositório**
```bash
curl -X GET "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main"
```

### **Purge de Arquivos Específicos**
```bash
curl -X GET "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main/style.css"
curl -X GET "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main/components.css"
curl -X GET "https://purge.jsdelivr.net/gh/canalqb/modulos_ESP@main/index.html"
```

### **URLs CDN para Uso**
```html
<!-- CSS -->
<link rel="stylesheet" href="https://cdn.jsdelivr.net/gh/canalqb/modulos_ESP@main/style.css">
<link rel="stylesheet" href="https://cdn.jsdelivr.net/gh/canalqb/modulos_ESP@main/components.css">

<!-- JavaScript -->
<script src="https://cdn.jsdelivr.net/gh/canalqb/modulos_ESP@main/secure-config.js"></script>
<script src="https://cdn.jsdelivr.net/gh/canalqb/modulos_ESP@main/ga4-gtm-integration.js"></script>
```

---

## **Configurações de Sistema**

### **GitHub Actions Workflows**
- **Arquivo:** `.github/workflows/cdn-force-update.yml`
- **Agendamento:** A cada 30 minutos
- **Função:** Purge automático do CDN
- **Trigger:** Schedule e manual

### **Google Analytics 4**
- **Property ID:** `392122309`
- **Domínio:** `canalqb.com.br`
- **Integração:** ESP32 + Dashboard Web
- **Conformidade:** LGPD/GDPR/CCPA

---

## **Procedimentos de Emergência**

### **Se Token for Comprometido**
1. **Revogar Imediatamente** no GitHub
2. **Gerar Novo Token**
3. **Atualizar Documento**
4. **Atualizar Scripts/Workflows**
5. **Fazer Push com Novo Token**

### **Se Service Account for Comprometida**
1. **Desativar** no Google Cloud Console
2. **Criar Nova Service Account**
3. **Atualizar Permissões GA4**
4. **Atualizar Arquivo JSON**
5. **Testar Integração**

---

## **Manutenção**

### **Verificação Mensal**
- [ ] Validar tokens ainda funcionam
- [ ] Verificar permissões GA4
- [ ] Testar integração CDN
- [ ] Revisar logs de acesso

### **Atualização Trimestral**
- [ ] Rotacionar tokens (se necessário)
- [ ] Revisar permissões mínimas
- [ ] Atualizar documentação
- [ ] Backup das configurações

---

## **Contato e Suporte**

### **Para Problemas com Tokens**
- **GitHub:** Verificar configurações de acesso
- **GCP:** Console do Google Cloud
- **CDN:** Documentação jsDelivr

### **Para Problemas de Integração**
- **ESP32:** Verificar configuração de rede
- **GA4:** Validar permissões e APIs
- **GitHub Actions:** Revisar logs de execução

---

## **Histórico de Alterações**

| Data | Alteração | Responsável |
|------|-----------|-------------|
| 21/04/2026 | Criação do documento | Sistema |
| | | |

---

**Última Atualização:** 21/04/2026  
**Próxima Revisão:** 21/07/2026  
**Status:** Ativo e Verificado
