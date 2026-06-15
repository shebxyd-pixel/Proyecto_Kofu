import json
import asyncio
from typing import Dict, Any, Optional


class MCPServer:
    def __init__(self):
        self.tools = {
            "create_powerpoint": self.mcp_create_powerpoint,
            "create_word_document": self.mcp_create_word_document,
            "research_topic": self.mcp_research_topic,
            "get_office_tips": self.mcp_get_office_tips
        }
        self.assistant = None

    def set_assistant(self, assistant):
        self.assistant = assistant

    async def handle_request(self, request: Dict[str, Any]) -> Dict[str, Any]:
        method = request.get("method")
        params = request.get("params", {})

        if method == "initialize":
            return self.initialize()
        elif method == "tools/list":
            return self.list_tools()
        elif method == "tools/call":
            return await self.call_tool(params)
        else:
            return {
                "jsonrpc": "2.0",
                "id": request.get("id"),
                "error": {"code": -32601, "message": "Method not found"}
            }

    def initialize(self) -> Dict[str, Any]:
        return {
            "jsonrpc": "2.0",
            "id": None,
            "result": {
                "protocolVersion": "2024-11-05",
                "capabilities": {
                    "tools": {}
                },
                "serverInfo": {
                    "name": "Office AI Assistant",
                    "version": "1.0.0"
                }
            }
        }

    def list_tools(self) -> Dict[str, Any]:
        return {
            "jsonrpc": "2.0",
            "id": None,
            "result": {
                "tools": [
                    {
                        "name": "create_powerpoint",
                        "description": "Crea una presentación de PowerPoint sobre un tema específico",
                        "inputSchema": {
                            "type": "object",
                            "properties": {
                                "topic": {"type": "string", "description": "Tema de la presentación"},
                                "output_path": {"type": "string", "description": "Ruta de salida del archivo"},
                                "theme": {"type": "string", "description": "Tema de color (professional/modern/vibrant)"}
                            },
                            "required": ["topic", "output_path"]
                        }
                    },
                    {
                        "name": "create_word_document",
                        "description": "Crea un documento de Word sobre un tema específico",
                        "inputSchema": {
                            "type": "object",
                            "properties": {
                                "topic": {"type": "string", "description": "Tema del documento"},
                                "output_path": {"type": "string", "description": "Ruta de salida del archivo"},
                                "style": {"type": "string", "description": "Estilo (professional/modern)"}
                            },
                            "required": ["topic", "output_path"]
                        }
                    },
                    {
                        "name": "research_topic",
                        "description": "Investiga un tema en internet",
                        "inputSchema": {
                            "type": "object",
                            "properties": {
                                "topic": {"type": "string", "description": "Tema a investigar"}
                            },
                            "required": ["topic"]
                        }
                    },
                    {
                        "name": "get_office_tips",
                        "description": "Obtén consejos sobre PowerPoint o Word",
                        "inputSchema": {
                            "type": "object",
                            "properties": {
                                "software": {"type": "string", "description": "powerpoint o word"}
                            },
                            "required": ["software"]
                        }
                    }
                ]
            }
        }

    async def call_tool(self, params: Dict[str, Any]) -> Dict[str, Any]:
        tool_name = params.get("name")
        arguments = params.get("arguments", {})

        if tool_name in self.tools:
            try:
                result = await self.tools[tool_name](**arguments)
                return {
                    "jsonrpc": "2.0",
                    "id": None,
                    "result": {
                        "content": [
                            {
                                "type": "text",
                                "text": str(result)
                            }
                        ]
                    }
                }
            except Exception as e:
                return {
                    "jsonrpc": "2.0",
                    "id": None,
                    "error": {"code": -32603, "message": str(e)}
                }
        else:
            return {
                "jsonrpc": "2.0",
                "id": None,
                "error": {"code": -32601, "message": "Tool not found"}
            }

    async def mcp_create_powerpoint(self, topic: str, output_path: str, theme: str = "professional") -> str:
        if self.assistant:
            path = self.assistant.create_presentation(topic, output_path, theme)
            return f"Presentación creada exitosamente: {path}"
        return "Asistente no inicializado"

    async def mcp_create_word_document(self, topic: str, output_path: str, style: str = "professional") -> str:
        if self.assistant:
            path = self.assistant.create_document(topic, output_path, style)
            return f"Documento creado exitosamente: {path}"
        return "Asistente no inicializado"

    async def mcp_research_topic(self, topic: str) -> str:
        if self.assistant:
            return self.assistant.research_topic(topic)
        return "Asistente no inicializado"

    async def mcp_get_office_tips(self, software: str) -> str:
        if self.assistant:
            tips = self.assistant.get_office_tips(software)
            return "\n".join([f"- {tip}" for tip in tips])
        return "Asistente no inicializado"
