const URL_CHAT = '';
const URL_OFFICE = '';
const URL_RESEARCH = '';
const URL_OLLAMA = '';
const URL_TEMPLATES = '';
 
let allTemplates = { powerpoint: [], word: [] };
let currentMode = 'local';
let activeTasks = 0;
let selectedFile = null;

document.addEventListener('DOMContentLoaded', async () => {
    const badge = document.getElementById('portBadge');
    if (badge) badge.textContent = `:Separados`;
    await Promise.all([loadTemplates(), loadOllamaModels()]);
    setupEventListeners();
    updateTaskCounter();
});

function setupEventListeners() {
    const messageInput = document.getElementById('messageInput');
    const sendBtn = document.getElementById('sendBtn');
    const documentType = document.getElementById('documentType');
    const shutdownBtn = document.getElementById('shutdownBtn');
    const localModeBtn = document.getElementById('localModeBtn');
    const onlineModeBtn = document.getElementById('onlineModeBtn');
    const apiKeyInput = document.getElementById('apiKeyInput');
    const fileInput = document.getElementById('fileInput');
    const removeFileBtn = document.getElementById('removeFileBtn');

    shutdownBtn.addEventListener('click', () => {
        if (confirm('Apagar')) {
            window.close();
        }
    });

    sendBtn.addEventListener('click', sendMessage);

    messageInput.addEventListener('keydown', (e) => {
        if (e.key === 'Enter' && !e.shiftKey) {
            e.preventDefault();
            sendMessage();
        }
    });

    messageInput.addEventListener('input', autoResize);
    documentType.addEventListener('change', updateTemplateDropdown);
    localModeBtn.addEventListener('click', () => switchMode('local'));
    onlineModeBtn.addEventListener('click', () => switchMode('online'));

    const savedApiKey = localStorage.getItem('kofu_api_key');
    if (savedApiKey) apiKeyInput.value = savedApiKey;
    apiKeyInput.addEventListener('input', (e) => {
        localStorage.setItem('kofu_api_key', e.target.value);
    });

    const ollamaModelInput = document.getElementById('ollamaModelInput');
    const savedModel = localStorage.getItem('kofu_ollama_model');
    if (savedModel) ollamaModelInput.value = savedModel;
    ollamaModelInput.addEventListener('input', (e) => {
        localStorage.setItem('kofu_ollama_model', e.target.value);
    });

    fileInput.addEventListener('change', handleFileSelect);
    removeFileBtn.addEventListener('click', removeSelectedFile);
}

function handleFileSelect(e) {
    if (e.target.files && e.target.files[0]) {
        selectedFile = e.target.files[0];
        document.getElementById('fileName').textContent = selectedFile.name;
        document.getElementById('filePreview').style.display = 'block';
    }
}

function removeSelectedFile() {
    selectedFile = null;
    document.getElementById('fileInput').value = '';
    document.getElementById('filePreview').style.display = 'none';
}

function switchMode(mode) {
    currentMode = mode;
    document.getElementById('localModeBtn').classList.toggle('active', mode === 'local');
    document.getElementById('onlineModeBtn').classList.toggle('active', mode === 'online');
    document.getElementById('apiConfig').style.display = mode === 'online' ? 'block' : 'none';
}

function updateTaskCounter() {
    document.getElementById('activeTasks').textContent = activeTasks;
}

function incrementTaskCounter() {
    activeTasks++;
    updateTaskCounter();
}

function decrementTaskCounter() {
    if (activeTasks > 0) activeTasks--;
    updateTaskCounter();
}

async function loadOllamaModels() {
    try {
        const response = await fetch(`${URL_OLLAMA}/ollama/models`);
        if (!response.ok) return;
        const data = await response.json();
        const datalist = document.getElementById('ollamaModelsList');
        datalist.innerHTML = '';
        (data.models || []).forEach(name => {
            const option = document.createElement('option');
            option.value = name;
            datalist.appendChild(option);
        });
    } catch (_) {}
}

async function loadTemplates() {
    try {
        const response = await fetch(`${URL_TEMPLATES}/templates`);
        if (!response.ok) return;
        const data = await response.json();
        allTemplates = data;
        updateTemplateDropdown();
    } catch (_) {}
}

function updateTemplateDropdown() {
    const docType = document.getElementById('documentType').value;
    const templateSelect = document.getElementById('templateSelect');
    templateSelect.innerHTML = '<option value="">Plantilla</option>';

    if (docType && allTemplates[docType]) {
        allTemplates[docType].forEach(template => {
            const option = document.createElement('option');
            option.value = template;
            option.textContent = template;
            templateSelect.appendChild(option);
        });
    }
}

function autoResize() {
    const textarea = document.getElementById('messageInput');
    textarea.style.height = 'auto';
    textarea.style.height = Math.min(textarea.scrollHeight, 150) + 'px';
}

function addMessage(text, role, isError = false, downloadInfo = null) {
    const chatContainer = document.getElementById('chatContainer');

    const welcomeMessage = chatContainer.querySelector('.welcome-message');
    if (welcomeMessage) welcomeMessage.remove();

    const messageDiv = document.createElement('div');
    messageDiv.className = 'message';

    const avatar = document.createElement('div');
    avatar.className = `message-avatar ${role}-avatar`;
    avatar.textContent = role === 'user' ? 'T' : 'K';

    const content = document.createElement('div');
    content.className = 'message-content';

    const roleLabel = document.createElement('div');
    roleLabel.className = 'message-role';
    roleLabel.textContent = role === 'user' ? 'Tú' : 'Kofu';

    const messageText = document.createElement('div');
    messageText.className = 'message-text' + (isError ? ' error-message' : '');
    messageText.textContent = text;

    content.appendChild(roleLabel);
    content.appendChild(messageText);

    if (downloadInfo) {
        const dlBtn = document.createElement('a');
        dlBtn.className = 'download-btn';
        dlBtn.href = downloadInfo.url;
        dlBtn.download = downloadInfo.filename;
        dlBtn.innerHTML = `<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4M7 10l5 5 5-5M12 15V3"/></svg> Descargar ${downloadInfo.filename}`;
        content.appendChild(dlBtn);
    }

    messageDiv.appendChild(avatar);
    messageDiv.appendChild(content);
    chatContainer.appendChild(messageDiv);
    chatContainer.scrollTop = chatContainer.scrollHeight;
}

function showTypingIndicator() {
    const chatContainer = document.getElementById('chatContainer');
    const typingDiv = document.createElement('div');
    typingDiv.className = 'message';
    typingDiv.id = 'typingIndicator';

    const avatar = document.createElement('div');
    avatar.className = 'message-avatar ai-avatar';
    avatar.textContent = 'K';

    const content = document.createElement('div');
    content.className = 'message-content';

    const typingIndicator = document.createElement('div');
    typingIndicator.className = 'typing-indicator';
    typingIndicator.innerHTML = `
        <div class="typing-dot"></div>
        <div class="typing-dot"></div>
        <div class="typing-dot"></div>
    `;

    content.appendChild(typingIndicator);
    typingDiv.appendChild(avatar);
    typingDiv.appendChild(content);
    chatContainer.appendChild(typingDiv);
    chatContainer.scrollTop = chatContainer.scrollHeight;
}

function removeTypingIndicator() {
    const indicator = document.getElementById('typingIndicator');
    if (indicator) indicator.remove();
}

async function sendMessage() {
    const messageInput = document.getElementById('messageInput');
    const message = messageInput.value.trim();

    if (!message && !selectedFile) return;

    if (currentMode === 'online') {
        const apiKey = document.getElementById('apiKeyInput').value.trim();
        if (!apiKey) {
            addMessage('Por favor ingresa una API key para usar el modo Online.', 'ai', true);
            return;
        }
    }

    if (message) {
        addMessage(message, 'user');
    } else if (selectedFile) {
        addMessage(`Archivo subido: ${selectedFile.name}`, 'user');
    }

    messageInput.value = '';
    autoResize();

    const sendBtn = document.getElementById('sendBtn');
    sendBtn.disabled = true;
    incrementTaskCounter();
    showTypingIndicator();

    try {
        const docType = document.getElementById('documentType').value;
        const template = document.getElementById('templateSelect').value;

        if (selectedFile) {
            if (docType) {
                await createDocumentFromFile(selectedFile, docType, template);
            } else {
                await processFile(selectedFile);
            }
        } else if (docType && (message.toLowerCase().includes('crear') || message.toLowerCase().includes('hacer'))) {
            await createDocument(message, docType, template);
        } else if (currentMode === 'online' && isResearchQuery(message)) {
            const result = await researchTopic(message);
            removeTypingIndicator();
            addMessage(result, 'ai');
        } else {
            const result = await chatWithAI(message);
            removeTypingIndicator();
            addMessage(result, 'ai');
        }

        removeSelectedFile();
    } catch (error) {
        removeTypingIndicator();
        const msg = error.message || '';
        const isKnown = msg.includes('seguridad') || msg.includes('bloqueada') ||
            msg.includes('No hay modelo disponible') || msg.includes('418');
        addMessage(isKnown ? msg : 'El servidor no está listo. Verifica que Kofu esté corriendo.', 'ai', true);
    } finally {
        sendBtn.disabled = false;
        decrementTaskCounter();
    }
}

async function processFile(file) {
    const formData = new FormData();
    formData.append('file', file);

    const response = await fetch(`${URL_CHAT}/files/upload`, {
        method: 'POST',
        body: formData,
    });

    if (!response.ok) {
        const data = await response.json().catch(() => ({}));
        throw new Error(data.detail || data.error || 'Error al procesar el archivo');
    }

    const data = await response.json();
    if (data.success) {
        let result = `✅ Archivo "${data.filename}" procesado.\n\n📄 Contenido extraído:\n\n`;
        result += data.text_content.substring(0, 1000);
        if (data.text_content.length > 1000) result += '\n... (contenido truncado)';
        removeTypingIndicator();
        addMessage(result, 'ai');
    } else {
        throw new Error(data.error || 'Error desconocido al procesar archivo');
    }
}

async function createDocumentFromFile(file, docType, template) {
    const isWord = docType === 'word';
    const endpoint = isWord ? '/office/word/download' : '/office/powerpoint/download';
    const filename = isWord ? 'documento_kofu.docx' : 'presentacion_kofu.pptx';
    const topic = file.name.replace(/\.[^.]+$/, '');

    const body = {
        topic,
        filename,
        template: template || undefined,
        style: isWord ? 'professional' : undefined,
        theme: isWord ? undefined : 'professional',
        modo: currentMode,
    };

    const response = await fetch(`${URL_OFFICE}${endpoint}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(body),
    });

    if (!response.ok) {
        const data = await response.json().catch(() => ({}));
        throw new Error(data.detail || data.error || 'Error al crear el documento');
    }

    const blob = await response.blob();
    const url = URL.createObjectURL(blob);
    const backupSaved = response.headers.get('X-Backup-Saved') === 'true';
    const backupPath = response.headers.get('X-Backup-Path') || '';
    const backupName = backupPath.split('/').pop() || filename;

    removeTypingIndicator();
    let msg = `✅ ${isWord ? 'Documento Word' : 'Presentación PowerPoint'} generado desde "${file.name}".`;
    if (backupSaved) msg += `\n📁 Copia guardada en Archivos/${backupName}`;

    addMessage(msg, 'ai', false, { url, filename });
}

function isResearchQuery(message) {
    const keywords = ['investiga', 'busca', 'qué es', 'que es', 'explica', 'informacion sobre', 'información sobre'];
    return keywords.some(k => message.toLowerCase().includes(k));
}

function extractTopic(message) {
    const patterns = [
        /(?:investiga|busca|qué es|que es|explica|informaci[oó]n sobre)[\s]+(.+?)(?:\?|$|,|\.)/i,
        /sobre\s+(.+?)(?:\?|$|,|\.)/i,
    ];
    for (const pattern of patterns) {
        const match = message.match(pattern);
        if (match && match[1]) return match[1].trim();
    }
    return message.trim();
}

async function chatWithAI(message) {
    const model = document.getElementById('ollamaModelInput').value.trim();

    const response = await fetch(`${URL_CHAT}/chat`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
            message,
            show_thinking: false,
            model: model || null,
        }),
    });

    if (!response.ok) {
        const data = await response.json().catch(() => ({}));
        throw new Error(data.detail || data.error || 'Error al comunicarse con el servidor');
    }

    const data = await response.json();
    return data.response;
}

async function researchTopic(message) {
    const topic = extractTopic(message);

    const response = await fetch(`${URL_RESEARCH}/research`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ topic, modo: 'online' }),
    });

    if (!response.ok) {
        const data = await response.json().catch(() => ({}));
        throw new Error(data.detail || data.error || 'Error al buscar información');
    }

    const data = await response.json();
    return data.summary || `No se encontró información sobre "${topic}".`;
}

async function createDocument(topic, docType, template) {
    const isWord = docType === 'word';
    const endpoint = isWord ? '/office/word/download' : '/office/powerpoint/download';
    const filename = isWord ? `documento_${topic.substring(0, 20).replace(/\s+/g, '_')}.docx`
                             : `presentacion_${topic.substring(0, 20).replace(/\s+/g, '_')}.pptx`;

    const body = {
        topic,
        filename,
        template: template || undefined,
        style: isWord ? 'professional' : undefined,
        theme: isWord ? undefined : 'professional',
        modo: currentMode,
    };

    const response = await fetch(`${URL_CHAT}${endpoint}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(body),
    });

    if (!response.ok) {
        const data = await response.json().catch(() => ({}));
        throw new Error(data.detail || data.error || 'Error al crear el documento');
    }

    const blob = await response.blob();
    const url = URL.createObjectURL(blob);
    const backupSaved = response.headers.get('X-Backup-Saved') === 'true';
    const backupPath = response.headers.get('X-Backup-Path') || '';
    const backupName = backupPath.split('/').pop() || filename;

    removeTypingIndicator();
    let msg = `✅ ${isWord ? 'Documento Word' : 'Presentación PowerPoint'} listo.`;
    if (template) msg += `\n📌 Plantilla aplicada: ${template}`;
    if (backupSaved) msg += `\n📁 Copia guardada en Archivos/${backupName}`;

    addMessage(msg, 'ai', false, { url, filename });
}
