from flask import Flask, request, jsonify
from flask_cors import CORS
import json
import os
import random
import threading
import time
from datetime import datetime

app = Flask(__name__)
CORS(app)

DATA_FILE = "data/sensores.json"
HISTORICO_FILE = "data/historico.json"

os.makedirs("data", exist_ok=True)

# Inicializa arquivos vazios se não existirem
if not os.path.exists(DATA_FILE):
    with open(DATA_FILE, "w") as f:
        json.dump({}, f)

# Sempre zera o histórico ao iniciar o servidor
with open(HISTORICO_FILE, "w") as f:
    json.dump([], f)

def gerar_dados_aleatorios():
    """Gera dados de sensores aleatórios no formato especificado"""
    return {
        "co": round(random.uniform(0.1, 5.0), 2),
        "no2": round(random.uniform(0.05, 1.0), 2),
        "voc": random.randint(500, 2000),
        "o3": round(random.uniform(20.0, 100.0), 2),
        "co2": random.randint(300, 1000),
        # "pm1": random.randint(0, 50),
        "pm25": random.randint(0, 100),
        "pm10": random.randint(0, 150),
        "uv": random.randint(0, 12),
        "temperatura": round(random.uniform(15.0, 35.0), 1),
        "umidade": round(random.uniform(30.0, 90.0), 1),
        "ruido": round(random.uniform(30.0, 100.0), 2),
    }

def enviar_dados_automaticamente():
    """Envia dados aleatórios periodicamente para o backend"""
    while True:
        time.sleep(10)  # Intervalo de 10 segundos
        
        # Simula uma requisição POST com dados aleatórios
        with app.test_client() as client:
            dados = gerar_dados_aleatorios()
            response = client.post(
                "/dados",
                json=dados,
                content_type='application/json'
            )
            print(f"Dados enviados: {dados} | Status: {response.status_code}")

@app.route("/dados", methods=["POST"])
def receber_dados():
    try:
        dados = request.get_json()
        print("Dados recebidos:", dados)
        dados["timestamp"] = datetime.now().isoformat()
        
        # Salva dados atuais
        with open(DATA_FILE, "w") as f:
            json.dump(dados, f, indent=4)

        # Atualiza histórico
        with open(HISTORICO_FILE, "r+") as f:
            historico = json.load(f)
            historico.append(dados)
            
            # Mantém apenas últimos 100 registros
            if len(historico) > 100:
                historico = historico[-100:]
            
            f.seek(0)
            f.truncate()
            json.dump(historico, f, indent=4)

        return jsonify({"status": "ok"}), 200
    except Exception as e:
        return jsonify({"erro": str(e)}), 500

@app.route("/dados", methods=["GET"])
def enviar_dados():
    try:
        with open(DATA_FILE) as f:
            dados = json.load(f)
        return jsonify(dados), 200
    except:
        return jsonify({"erro": "dados não disponíveis"}), 404

@app.route("/historico")
def get_historico():
    try:
        with open(HISTORICO_FILE) as f:
            return jsonify(json.load(f)), 200
    except:
        return jsonify([]), 200

if __name__ == "__main__":
    # Inicia thread para envio automático de dados
    thread = threading.Thread(target=enviar_dados_automaticamente)
    thread.daemon = True  # Encerra com o processo principal
    thread.start()
    
    app.run(host="0.0.0.0", port=5000, debug=True)