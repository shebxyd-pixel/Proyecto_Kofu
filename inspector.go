package main

import (
	"bytes"
	"fmt"
	"io"
	"log"
	"net/http"
	"net/http/httputil"
	"net/url"
)

var portMappings = map[string]string{
	"9010": "4010",
	"9050": "5050",
	"9070": "7070",
}

func inspectAndForward(targetPort string) http.HandlerFunc {
	targetURL, err := url.Parse(fmt.Sprintf("http://127.0.0.1:%s", targetPort))
	if err != nil {
		log.Fatalf("Error configurando la URL destino: %v", err)
	}

	proxy := httputil.NewSingleHostReverseProxy(targetURL)

	return func(w http.ResponseWriter, r *http.Request) {
		fmt.Printf("\n==================== [NUEVA PETICIÓN] ====================\n")
		fmt.Printf("Método: %s | Ruta: %s\n", r.Method, r.URL.Path)
		fmt.Printf("Destino final: %s\n", targetURL.String())

		// Configurar cabeceras CORS de respuesta
		w.Header().Set("Access-Control-Allow-Origin", "*")
		w.Header().Set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
		w.Header().Set("Access-Control-Allow-Headers", "*")

		if r.Method == "OPTIONS" {
			w.WriteHeader(http.StatusOK)
			return
		}

		// Leer e imprimir el Body si existe
		if r.Body != nil {
			bodyBytes, err := io.ReadAll(r.Body)
			if err == nil && len(bodyBytes) > 0 {
				fmt.Println("\n--- BODY (JSON) ---")
				fmt.Println(string(bodyBytes))

				// IMPORTANTE: Restaurar el r.Body para que la proxy lo pueda reenviar
				r.Body = io.NopCloser(bytes.NewBuffer(bodyBytes))
			}
		}

		// Modificar el Host del Request antes de pasarlo al proxy
		r.Host = targetURL.Host
		r.URL.Host = targetURL.Host
		r.URL.Scheme = targetURL.Scheme

		proxy.ServeHTTP(w, r)
	}
}

func main() {
	fmt.Println("🚀 Inspector de Tráfico HTTP iniciado...")

	for listenPort, targetPort := range portMappings {
		go func(lPort, tPort string) {
			mux := http.NewServeMux()
			mux.HandleFunc("/", inspectAndForward(tPort))
			log.Printf("Escuchando en puerto proxy :%s ---> Backend :%s\n", lPort, tPort)
			if err := http.ListenAndServe(":"+lPort, mux); err != nil {
				log.Fatalf("Error en el puerto %s: %v", lPort, err)
			}
		}(listenPort, targetPort)
	}

	select {}
}