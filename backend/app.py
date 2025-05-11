from flask import Flask, request, jsonify
from flask_cors import CORS
import json
import os
from datetime import datetime  # <-- Novo import adicionado

app = Flask(__name__)
CORS(app)  # Libera acesso ao front-end (Vue)

DATA_FILE = "data/sensores.json"
HISTORICO_FILE = "data/historico.json"  # <-- Nova constante adicionada

@app.route("/dados", methods=["POST"])
def receber_dados():
    try:
        dados = request.get_json()
        dados["timestamp"] = datetime.now().isoformat()  # Garante timestamp atual
        
        # Salva última leitura
        with open(DATA_FILE, "w") as f:
            json.dump(dados, f, indent=4)
        
        # Adiciona ao histórico
        historico = []
        if os.path.exists(HISTORICO_FILE):
            with open(HISTORICO_FILE) as f:
                historico = json.load(f)
        
        historico.append(dados)
        if len(historico) > 100:  # Mantém apenas 100 leituras
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

@app.route("/historico")  # <-- Nova rota adicionada
def get_historico():
    if os.path.exists(HISTORICO_FILE):
        with open(HISTORICO_FILE) as f:
            return jsonify(json.load(f)), 200
    return jsonify([]), 200

@app.route("/ultima-atualizacao")
def get_ultima_atualizacao():
    if os.path.exists(DATA_FILE):
        with open(DATA_FILE) as f:
            dados = json.load(f)
            return jsonify({"timestamp": dados["timestamp"]}), 200
    return jsonify({"timestamp": None}), 404

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)