from flask import Flask, request, jsonify
from flask_cors import CORS
import json
import os
from datetime import datetime
import threading
import time
import random  # Para gerar valores aleatórios

app = Flask(__name__)
CORS(app)

DATA_FILE = "data/sensores.json"
HISTORICO_FILE = "data/historico.json"

# Garante que o diretório exista
os.makedirs("data", exist_ok=True)

# Sempre zera o histórico ao iniciar o servidor
with open(HISTORICO_FILE, "w") as f:
    json.dump([], f, indent=4)


@app.route("/dados", methods=["POST"])
def receber_dados():
    try:
        dados = request.get_json()
        dados["timestamp"] = datetime.now().isoformat()

        with open(DATA_FILE, "w") as f:
            json.dump(dados, f, indent=4)

        with open(HISTORICO_FILE) as f:
            historico = json.load(f)

        historico.append(dados)
        if len(historico) > 100:
            historico = historico[-100:]

        with open(HISTORICO_FILE, "w") as f:
            json.dump(historico, f, indent=4)

        return jsonify({"status": "ok"}), 200
    except Exception as e:
        return jsonify({"erro": str(e)}), 500

@app.route("/dados", methods=["GET"])
def enviar_dados():
    if os.path.exists(DATA_FILE):
        with open(DATA_FILE) as f:
            dados = json.load(f)
        return jsonify(dados), 200
    else:
        return jsonify({"erro": "arquivo de dados não encontrado"}), 404

@app.route("/historico")
def get_historico():
    if os.path.exists(HISTORICO_FILE):
        with open(HISTORICO_FILE) as f:
            return jsonify(json.load(f)), 200
    return jsonify([]), 200

# Função para gerar valores aleatórios e salvar no histórico
def gerar_dados_automaticamente():
    while True:
        try:
            leitura = {
                "timestamp": datetime.now().isoformat(),
                "co": round(random.uniform(0, 10), 2),
                "no2": round(random.uniform(0, 1), 2),
                "voc": round(random.uniform(0, 10), 2),
                "o3": round(random.uniform(0, 10), 2),
                "co2": round(random.uniform(0, 10), 2),
                "pm1": random.randint(0, 100),
                "pm10": random.randint(0, 100),
                "pm25": random.randint(0, 100),
                "uv": round(random.uniform(0, 15), 1),
                "temperatura": round(random.uniform(10, 40), 1),
                "umidade": round(random.uniform(10, 100), 1),
                "ruido": random.randint(30, 100)
            }

            with open(DATA_FILE, "w") as f:
                json.dump(leitura, f, indent=4)

            with open(HISTORICO_FILE) as f:
                historico = json.load(f)

            historico.append(leitura)
            if len(historico) > 100:
                historico = historico[-100:]

            with open(HISTORICO_FILE, "w") as f:
                json.dump(historico, f, indent=4)

        except Exception as e:
            print("Erro ao gerar dados:", e)

        time.sleep(5)


# Inicia a thread em segundo plano
threading.Thread(target=gerar_dados_automaticamente, daemon=True).start()

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
