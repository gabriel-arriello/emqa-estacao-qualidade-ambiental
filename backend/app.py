from flask import Flask, request, jsonify
from flask_cors import CORS
import json
import os
from datetime import datetime

app = Flask(__name__)
CORS(app)

DATA_FILE = "data/sensores.json"
HISTORICO_FILE = "data/historico.json"

os.makedirs("data", exist_ok=True)

# Inicializa arquivos vazios se não existirem

# Sempre zera o histórico ao iniciar o servidor
with open(HISTORICO_FILE, "w") as f:
    json.dump([], f, indent=4)

if not os.path.exists(DATA_FILE):
    with open(DATA_FILE, "w") as f:
        json.dump({}, f)

if not os.path.exists(HISTORICO_FILE):
    with open(HISTORICO_FILE, "w") as f:
        json.dump([], f)

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
    app.run(host="0.0.0.0", port=5000, debug=True)
