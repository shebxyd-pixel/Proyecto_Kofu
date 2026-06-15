import requests
import json
import os
from dotenv import load_dotenv

load_dotenv()


class ExternalAIAPI:
    def __init__(self):
        self.google_api_key = os.getenv("GOOGLE_API_KEY")
        self.google_cx = os.getenv("GOOGLE_CX")
        self.openai_api_key = os.getenv("OPENAI_API_KEY")
        
    def search_google(self, query, num_results=5):
        if not self.google_api_key or not self.google_cx:
            return None
            
        url = "https://www.googleapis.com/customsearch/v1"
        params = {
            "key": self.google_api_key,
            "cx": self.google_cx,
            "q": query,
            "num": num_results
        }
        
        try:
            response = requests.get(url, params=params)
            response.raise_for_status()
            data = response.json()
            
            results = []
            if "items" in data:
                for item in data["items"]:
                    results.append({
                        "title": item.get("title"),
                        "link": item.get("link"),
                        "snippet": item.get("snippet")
                    })
            return results
        except Exception as e:
            print(f"Error en Google Search: {e}")
            return None
    
    def query_openai(self, prompt, model="gpt-3.5-turbo", max_tokens=500):
        if not self.openai_api_key:
            return None
            
        url = "https://api.openai.com/v1/chat/completions"
        headers = {
            "Content-Type": "application/json",
            "Authorization": f"Bearer {self.openai_api_key}"
        }
        
        payload = {
            "model": model,
            "messages": [
                {"role": "user", "content": prompt}
            ],
            "max_tokens": max_tokens,
            "temperature": 0.7
        }
        
        try:
            response = requests.post(url, headers=headers, data=json.dumps(payload))
            response.raise_for_status()
            data = response.json()
            
            return data["choices"][0]["message"]["content"].strip()
        except Exception as e:
            print(f"Error en OpenAI: {e}")
            return None
    
    def get_response(self, query, use_google=True, use_openai=True):
        results = {}
        
        if use_google:
            google_results = self.search_google(query)
            if google_results:
                results["google"] = google_results
        
        if use_openai:
            openai_response = self.query_openai(query)
            if openai_response:
                results["openai"] = openai_response
        
        return results


if __name__ == "__main__":
    ai = ExternalAIAPI()
    response = ai.get_response("Que es el pentesting?")
    print(json.dumps(response, indent=2, ensure_ascii=False))
