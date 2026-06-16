const API_URL = 'http://localhost:5000/api';

let allTemplates = {
    powerpoint: [],
    word: []
};

let currentMode = 'local';
let activeTasks = 0;

document.addEventListener('DOMContentLoaded', () => {
    loadTemplates();
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

    shutdownBtn.addEventListener('click', () => {
        if (confirm('¿Tu pagas el servidor o porque me quieres apagar?')) {
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

    // Load saved API key from localStorage
    const savedApiKey = localStorage.getItem('kofu_api_key');
    if (savedApiKey) {
        apiKeyInput.value = savedApiKey;
    }

    apiKeyInput.addEventListener('input', (e) => {
        localStorage.setItem('kofu_api_key', e.target.value);
    });
}

function switchMode(mode) {
    currentMode = mode;
    const localModeBtn = document.getElementById('localModeBtn');
    const onlineModeBtn = document.getElementById('onlineModeBtn');
    const apiConfig = document.getElementById('apiConfig');

    localModeBtn.classList.toggle('active', mode === 'local');
    onlineModeBtn.classList.toggle('active', mode === 'online');

    if (mode === 'online') {
        apiConfig.style.display = 'block';
    } else {
        apiConfig.style.display = 'none';
    }
}

function updateTaskCounter() {
    const counter = document.getElementById('activeTasks');
    counter.textContent = activeTasks;
}

function incrementTaskCounter() {
    activeTasks++;
    updateTaskCounter();
}

function decrementTaskCounter() {
    activeTasks--;
    if (activeTasks < 0) activeTasks = 0;
    updateTaskCounter();
}

async function loadTemplates() {
    try {
        const response = await fetch(`${API_URL}/templates`);
        const data = await response.json();
        allTemplates = data;
        updateTemplateDropdown();
    } catch (error) {
        console.error('Error cargando plantillas:', error);
    }
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

function addMessage(text, role, isError = false) {
    const chatContainer = document.getElementById('chatContainer');

    const welcomeMessage = chatContainer.querySelector('.welcome-message');
    if (welcomeMessage) {
        welcomeMessage.remove();
    }

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
    messageText.className = 'message-text';
    if (isError) {
        messageText.classList.add('error-message');
    }
    messageText.textContent = text;

    content.appendChild(roleLabel);
    content.appendChild(messageText);

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
    const typingIndicator = document.getElementById('typingIndicator');
    if (typingIndicator) {
        typingIndicator.remove();
    }
}

async function sendMessage() {
    const messageInput = document.getElementById('messageInput');
    const message = messageInput.value.trim();

    if (!message) return;

    // Check for Online mode requirements
    if (currentMode === 'online') {
        const apiKey = document.getElementById('apiKeyInput').value.trim();
        if (!apiKey) {
            addMessage('Por favor ingresa una API key para usar el modo Online.', 'ai', true);
            return;
        }
    }

    addMessage(message, 'user');
    messageInput.value = '';
    autoResize();

    const sendBtn = document.getElementById('sendBtn');
    sendBtn.disabled = true;

    incrementTaskCounter();
    showTypingIndicator();

    try {
        const docType = document.getElementById('documentType').value;
        const template = document.getElementById('templateSelect').value;

        let response;

        if (docType && (message.toLowerCase().includes('crear') || message.toLowerCase().includes('hacer'))) {
            response = await createDocument(message, docType, template);
        } else if (currentMode === 'online' && isResearchQuery(message)) {
            response = await researchTopic(message);
        } else {
            response = await chatWithAI(message);
        }

        removeTypingIndicator();
        addMessage(response, 'ai');
    } catch (error) {
        removeTypingIndicator();
        addMessage('La ia no esta lista para usar, Intenta de nuevo mas tarde o vea el manual de errores.', 'ai', true);
        console.error('Error:', error);
    } finally {
        sendBtn.disabled = false;
        decrementTaskCounter();
    }
}

function isResearchQuery(message) {
    const keywords = ['investiga', 'busca', 'qué es', 'que es', 'explica', 'informacion sobre', 'información sobre', 'sobre qué es', 'sobre que es', 'buscar', 'investigar'];
    return keywords.some(keyword => message.toLowerCase().includes(keyword));
}

function extractTopic(message) {
    const patterns = [
        /(?:investiga|busca|qué es|que es|explica|informaci[oó]n sobre|sobre qué es|sobre que es)\s+(.+?)(?:\?|$|,|\.)/i,
        /sobre\s+(.+?)(?:\?|$|,|\.)/i
    ];

    for (const pattern of patterns) {
        const match = message.match(pattern);
        if (match && match[1]) {
            return match[1].trim();
        }
    }
    return message.trim();
}

async function chatWithAI(message) {
    const response = await fetch(`${API_URL}/chat`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ message, show_thinking: false })
    });

    const data = await response.json();
    return data.response;
}

async function researchTopic(message) {
    const topic = extractTopic(message);

    try {
        const searchResponse = await fetch(`${API_URL}/search`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ query: topic, num_results: 5 })
        });

        const searchData = await searchResponse.json();

        if (searchData.results && searchData.results.length > 0) {
            let summary = `📚 Resultados de la búsqueda sobre: "${topic}"\n\n`;

            searchData.results.forEach((result, index) => {
                summary += `🔹 ${index + 1}. ${result.title}\n`;
                if (result.content) {
                    summary += `   ℹ️ ${result.content.substring(0, 200)}...\n`;
                }
                summary += `   🔗 ${result.url}\n`;
                if (result.source) {
                    summary += `   📍 Fuente: ${result.source}\n`;
                }
                summary += '\n';
            });

            return summary;
        }
    } catch (error) {
        console.error('Error en la búsqueda:', error);
    }

    return `No se encontró información sobre "${topic}".`;
}

async function createDocument(topic, docType, template) {
    const endpoint = docType === 'word' ? '/create-document' : '/create-powerpoint';
    const filename = docType === 'word' ? 'documento_kofu.docx' : 'presentacion_kofu.pptx';

    const response = await fetch(`${API_URL}${endpoint}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ 
            topic: topic, 
            template: template,
            filename: filename
        })
    });

    const data = await response.json();

    if (data.success) {
        return `He creado tu ${docType === 'word' ? 'documento de Word' : 'presentación de PowerPoint'} con éxito! 🎉\n\nArchivo guardado en: ${data.file_path}\n\nPlantilla usada: ${template || 'Ninguna (predeterminada)'}`;
    } else {
        return 'Hubo un error al crear el documento. Por favor intenta de nuevo.';
    }
}
