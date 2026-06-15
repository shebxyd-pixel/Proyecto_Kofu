import requests
from bs4 import BeautifulSoup
import urllib.parse
import os
import re


class WebResearcher:
    def __init__(self, google_api_key=None, google_cx=None):
        self.headers = {
            'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36'
        }
        self.google_api_key = google_api_key or os.getenv('GOOGLE_API_KEY')
        self.google_cx = google_cx or os.getenv('GOOGLE_CX')

    def search_web(self, query, num_results=5, engine='duckduckgo'):
        results = []
        
        if engine == 'google' and self.google_api_key and self.google_cx:
            results = self._search_google(query, num_results)
        
        if not results:
            results = self._search_duckduckgo(query, num_results)
            
        if not results:
            results = self._search_bing(query, num_results)
            
        return results

    def _search_google(self, query, num_results):
        url = "https://www.googleapis.com/customsearch/v1"
        params = {
            'key': self.google_api_key,
            'cx': self.google_cx,
            'q': query,
            'num': min(num_results, 10)
        }
        
        try:
            response = requests.get(url, params=params, timeout=10)
            response.raise_for_status()
            data = response.json()
            
            results = []
            for item in data.get('items', []):
                results.append({
                    'title': item.get('title', ''),
                    'url': item.get('link', ''),
                    'content': item.get('snippet', ''),
                    'source': 'Google'
                })
            
            return results
        except Exception as e:
            print(f"Error en búsqueda de Google: {e}")
            return []

    def _search_duckduckgo(self, query, num_results):
        try:
            search_url = f"https://html.duckduckgo.com/html/?q={urllib.parse.quote(query)}"
            response = requests.get(search_url, headers=self.headers, timeout=10)
            response.raise_for_status()
            
            soup = BeautifulSoup(response.text, 'html.parser')
            results = []
            
            result_divs = soup.find_all('div', class_='result')
            
            for i, result in enumerate(result_divs[:num_results]):
                title_elem = result.find('a', class_='result__a')
                snippet_elem = result.find('a', class_='result__snippet')
                
                if title_elem:
                    title = title_elem.get_text(strip=True)
                    url = title_elem.get('href', '')
                    
                    content = ''
                    if snippet_elem:
                        content = snippet_elem.get_text(strip=True)
                    
                    if url:
                        results.append({
                            'title': title,
                            'url': url,
                            'content': content,
                            'source': 'DuckDuckGo'
                        })
            
            return results
        except Exception as e:
            print(f"Error en la búsqueda de DuckDuckGo: {e}")
            return []

    def _search_bing(self, query, num_results):
        try:
            search_url = f"https://www.bing.com/search?q={urllib.parse.quote(query)}"
            response = requests.get(search_url, headers=self.headers, timeout=10)
            response.raise_for_status()
            
            soup = BeautifulSoup(response.text, 'html.parser')
            results = []
            
            result_items = soup.find_all('li', class_='b_algo')
            
            for i, item in enumerate(result_items[:num_results]):
                title_elem = item.find('h2')
                link_elem = item.find('a')
                snippet_elem = item.find('p')
                
                if title_elem and link_elem:
                    title = title_elem.get_text(strip=True)
                    url = link_elem.get('href', '')
                    
                    content = ''
                    if snippet_elem:
                        content = snippet_elem.get_text(strip=True)
                    
                    if url and url.startswith('http'):
                        results.append({
                            'title': title,
                            'url': url,
                            'content': content,
                            'source': 'Bing'
                        })
            
            return results
        except Exception as e:
            print(f"Error en la búsqueda de Bing: {e}")
            return []

    def extract_content(self, url):
        try:
            response = requests.get(url, headers=self.headers, timeout=8)
            response.raise_for_status()
            soup = BeautifulSoup(response.text, 'html.parser')
            
            paragraphs = soup.find_all('p')
            content = ' '.join([p.get_text(strip=True) for p in paragraphs[:8]])
            
            return content[:400]
        except Exception as e:
            print(f"Error al extraer contenido de {url}: {e}")
            return ""

    def generate_summary(self, topic, results):
        if not results:
            return f"No se encontró información sobre: {topic}\n\nSugerencias:\n• Intenta con términos más específicos\n• Verifica tu conexión a internet"
        
        summary = f"📚 Resumen sobre: {topic}\n\n"
        
        for i, result in enumerate(results, 1):
            summary += f"🔹 Fuente {i} ({result.get('source', 'Desconocida')})\n"
            summary += f"   📝 {result['title']}\n"
            if result['content']:
                summary += f"   ℹ️ {result['content'][:180]}...\n"
            summary += f"   🔗 {result['url']}\n\n"
        
        return summary.strip()
