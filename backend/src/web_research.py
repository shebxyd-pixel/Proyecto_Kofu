import os
import urllib.parse
from typing import Dict, List, Optional

import requests


class WebResearcher:
    HEADERS = {"User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) Chrome/120.0.0.0 Safari/537.36"}

    def __init__(self, google_api_key: Optional[str] = None, google_cx: Optional[str] = None):
        self.google_api_key = google_api_key or os.getenv("GOOGLE_API_KEY")
        self.google_cx = google_cx or os.getenv("GOOGLE_CX")

    def search_web(self, query: str, num_results: int = 5) -> List[Dict[str, str]]:
        for buscador in (self._search_google, self._search_duckduckgo, self._search_bing):
            try:
                results = buscador(query, num_results)
                if results:
                    return results
            except Exception:
                continue
        return []

    def _search_google(self, query: str, num_results: int) -> List[Dict[str, str]]:
        if not (self.google_api_key and self.google_cx):
            return []
        params = {"key": self.google_api_key, "cx": self.google_cx, "q": query, "num": min(num_results, 10)}
        resp = requests.get("https://www.googleapis.com/customsearch/v1", params=params, timeout=10)
        resp.raise_for_status()
        return [
            {"title": i.get("title", ""), "url": i.get("link", ""), "content": i.get("snippet", ""), "source": "Google"}
            for i in resp.json().get("items", [])
        ]

    def _search_duckduckgo(self, query: str, num_results: int) -> List[Dict[str, str]]:
        from bs4 import BeautifulSoup
        url = f"https://html.duckduckgo.com/html/?q={urllib.parse.quote(query)}"
        resp = requests.get(url, headers=self.HEADERS, timeout=10)
        resp.raise_for_status()
        soup = BeautifulSoup(resp.text, "html.parser")
        results = []
        for div in soup.find_all("div", class_="result")[:num_results]:
            title_elem = div.find("a", class_="result__a")
            if not title_elem:
                continue
            snippet_elem = div.find("a", class_="result__snippet")
            results.append({
                "title": title_elem.get_text(strip=True),
                "url": title_elem.get("href", ""),
                "content": snippet_elem.get_text(strip=True) if snippet_elem else "",
                "source": "DuckDuckGo",
            })
        return results

    def _search_bing(self, query: str, num_results: int) -> List[Dict[str, str]]:
        from bs4 import BeautifulSoup
        url = f"https://www.bing.com/search?q={urllib.parse.quote(query)}"
        resp = requests.get(url, headers=self.HEADERS, timeout=10)
        resp.raise_for_status()
        soup = BeautifulSoup(resp.text, "html.parser")
        results = []
        for item in soup.find_all("li", class_="b_algo")[:num_results]:
            title_elem, link_elem = item.find("h2"), item.find("a")
            if not (title_elem and link_elem):
                continue
            snippet_elem = item.find("p")
            href = link_elem.get("href", "")
            if href.startswith("http"):
                results.append({
                    "title": title_elem.get_text(strip=True), "url": href,
                    "content": snippet_elem.get_text(strip=True) if snippet_elem else "", "source": "Bing",
                })
        return results

    def generate_summary(self, topic: str, results: List[Dict[str, str]]) -> str:
        if not results:
            return f"No se encontró información sobre: {topic}"
        summary = f"📚 Resumen sobre: {topic}\n\n"
        for i, r in enumerate(results, 1):
            summary += f"🔹 Fuente {i} ({r.get('source', 'Desconocida')})\n   📝 {r['title']}\n"
            if r["content"]:
                summary += f"   ℹ️ {r['content'][:180]}...\n"
            summary += f"   🔗 {r['url']}\n\n"
        return summary.strip()
