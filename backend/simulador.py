import requests
import random
from time import sleep
from datetime import datetime

def gerar_dados():
    return {
        "timestamp": datetime.now().isoformat(),
        "co": round(random.uniform(0.5, 5.0), 2),
        "no2": round(random.uniform(0.01, 0.5), 2),
        "voc": round(random.uniform(0.1, 2.0), 2),
        "pm25": random.randint(10, 100),
        "pm10": random.randint(20, 150),
        "uv": round(random.uniform(1.0, 10.0), 1),
        "temperatura": round(random.uniform(15.0, 35.0), 1),
        "umidade": random.randint(30, 90),
        "ruido": random.randint(40, 90)
    }

while True:
    dados = gerar_dados()
    try:
        response = requests.post("http://localhost:5000/dados", json=dados)
        print(f"Dados enviados: {response.status_code}")
    except Exception as e:
        print(f"Erro: {e}")
    sleep(60)  # Envia a cada 1 minuto