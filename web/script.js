const API_URL = 'http://localhost:5000/api';

let allTemplates = {
    powerpoint: [],
    word: []
};

document.addEventListener('DOMContentLoaded', () => {
    loadTemplates();
    setupEventListeners();
});

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

function setupEventListeners() {
    const messageInput = document.getElementById('messageInput');
    const sendBtn = document.getElementById('sendBtn');
    const documentType = document.getElementById('documentType');
    const shutdownBtn = document.getElementById('shutdownBtn');
    
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
}

function autoResize() {
    const textarea = document.getElementById('messageInput');
    textarea.style.height = 'auto';
    textarea.style.height = Math.min(textarea.scrollHeight, 150) + 'px';
}

async function sendMessage() {
    const messageInput = document.getElementById('messageInput');
    const message = messageInput.value.trim();
    
    if (!message) return;
    
    addMessage(message, 'user');
    messageInput.value = '';
    autoResize();
    
    const sendBtn = document.getElementById('sendBtn');
    sendBtn.disabled = true;
    
    showTypingIndicator();
    
    try {
        const docType = document.getElementById('documentType').value;
        const template = document.getElementById('templateSelect').value;
        
        let response;
        
        if (docType && (message.toLowerCase().includes('crear') || message.toLowerCase().includes('hacer'))) {
            response = await createDocument(message, docType, template);
        } else {
            response = await chatWithAI(message);
        }
        
        removeTypingIndicator();
        addMessage(response, 'ai');
    } catch (error) {
        removeTypingIndicator();
        addMessage('Lo siento, hubo un error. Por favor intenta de nuevo.', 'ai');
        console.error('Error:', error);
    }
    
    sendBtn.disabled = false;
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
    if (isResearchQuery(message)) {
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
    }
    
    const response = await fetch(`${API_URL}/chat`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ message, show_thinking: false })
    });
    
    const data = await response.json();
    return data.response;
}

async function createDocument(topic, docType, template) {
    const endpoint = docType === 'word' ? '/create-document' : '/create-powerpoint';
    const filename = docType === 'word' ? 'documento_laria.docx' : 'presentacion_laria.pptx';
    
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

function addMessage(text, role) {
    const chatContainer = document.getElementById('chatContainer');
    
    const welcomeMessage = chatContainer.querySelector('.welcome-message');
    if (welcomeMessage) {
        welcomeMessage.remove();
    }
    
    const messageDiv = document.createElement('div');
    messageDiv.className = 'message';
    
    const avatar = document.createElement('div');
    avatar.className = `message-avatar ${role}-avatar`;
    avatar.textContent = role === 'user' ? 'T' : 'L';
    
    const content = document.createElement('div');
    content.className = 'message-content';
    
    const roleLabel = document.createElement('div');
    roleLabel.className = 'message-role';
    roleLabel.textContent = role === 'user' ? 'Tú' : 'LarIA';
    
    const messageText = document.createElement('div');
    messageText.className = 'message-text';
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
    avatar.textContent = 'L';
    
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
