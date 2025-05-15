<template>
  <div class="dashboard">
    <h1 class="text-center mb-4">ESTAÇÃO DE QUALIDADE AMBIENTAL</h1>

    <div class="sensor-rows">
      <div 
        class="sensor-row"
        v-for="(linha, linhaIndex) in linhasDeSensores"
        :key="linhaIndex"
      >
        <div
          v-for="(sensor, index) in linha"
          :key="sensor.columName"
          :class="[
            'sensor-card-wrapper',
            index === 0 ? 'align-left' : index === 1 ? 'align-center' : 'align-right'
          ]"
        >
          <Card class="cursor-pointer sensor-card" @click="$router.push('/sensor/' + sensor.columName)">
            <template #title>
              <div class="sensor-card-title">{{ sensor.nome }}</div>
            </template>
            <template #content>
              <div class="text-center">
                <span class="text-4xl font-bold">{{ sensor.valor }}</span>
                <span class="text-xl">{{ sensor.unidade }}</span>
                <Chart 
                  type="line"
                  :data="getSensorChartData(sensor.columName)"
                  :options="chartOptions"
                  class="sensor-chart"
                  style="width: 100% !important; height: 160px;"
                />
              </div>
            </template>
          </Card>
        </div>
        <!-- Adiciona lugares vazios para manter alinhamento -->
        <div
          v-for="n in 3 - linha.length"
          :key="'placeholder-' + n + '-' + linhaIndex"
          class="sensor-card-wrapper"
          style="visibility: hidden;"
        >
          <div class="sensor-card"></div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import Card from 'primevue/card';
import Chart from 'primevue/chart';
import axios from 'axios';

export default {
  components: { Card, Chart },
  data() {
    return {
      dados: {},
      historico: [],
      chartData: {}, // <--- aqui guardaremos os dados dos gráficos
      chartOptions: {
        responsive: true,
        maintainAspectRatio: false,
        animation: { duration: 0 }, // <--- desativa animações
        plugins: {
          legend: {
            display: false
          }
        }
      },
      ultimoTimestamp: null,
      pollingInterval: null
    }
  },
  methods: {
    formatDate(dateString) {
      return new Date(dateString).toLocaleString();
    },
    formatSensorName(name) {
      const names = {
        pm1: 'Material Particulado 1.0 (PM1.0)',
        pm10: 'Material Particulado 10.0 (PM10.0)',
        pm25: 'Material Particulado 2.5 (PM2.5)',
        temperatura: 'Temperatura',
        umidade: 'Umidade',
        uv: 'Luz Ultravioleta (UV)',
        ruido: 'Ruído Sonoro',
        co: 'Monóxido de Carbono (CO)',
        no2: 'Dióxido de Nitrogênio (NO2)',
        voc: 'Compostos Orgânicos Voláteis (VOCs) ',
        o3: 'Trioxigênio (O3)',
        co2: 'Dióxido de Carbono (CO2)',
      };
      return names[name] || name;
    },
  getSensorChartData(sensorName) {
    return this.chartData[sensorName] || { labels: [], datasets: [] };
  },
    async fetchData() {
      try {
        const [current, history] = await Promise.all([
          axios.get('http://localhost:5000/dados'),
          axios.get('http://localhost:5000/historico')
        ]);
        this.dados = current.data;
        this.historico = history.data;

        // Atualiza os dados dos gráficos
        Object.keys(current.data).forEach(sensorName => {
          if (sensorName === 'timestamp') return;

          const labels = history.data.map(d => new Date(d.timestamp).toLocaleTimeString());
          const data = history.data.map(d => d[sensorName]);

          // Se já existe, atualiza o conteúdo sem recriar o objeto
          if (this.chartData[sensorName]) {
            this.chartData[sensorName].labels = labels;
            this.chartData[sensorName].datasets[0].data = data;
          } else {
            // Se ainda não existe, cria o objeto
            this.chartData[sensorName] = {
              labels,
              datasets: [{
                data,
                borderColor: '#3498db',
                tension: 0.1,
                borderWidth: 2
              }]
            };
          }
        });
      } catch (error) {
        console.error("Erro ao obter dados:", error);
      }
    }
  },
  mounted() {
    this.fetchData();
    this.pollingInterval = setInterval(this.fetchData, 5000);
  },
  beforeUnmount() {
    clearInterval(this.pollingInterval);
  },
  computed: {
    sensoresFormatados() {
    return Object.entries(this.dados)
      .filter(([nome]) => nome !== 'timestamp')
      .map(([nome, valor]) => ({
        nome: this.formatSensorName(nome),
        columName: nome,
        valor,
        unidade: this.getUnidade(nome)
      }));
    },
    linhasDeSensores() {
      const linhas = [];
      const sensores = this.sensoresFormatados;
      for (let i = 0; i < sensores.length; i += 3) {
        linhas.push(sensores.slice(i, i + 3));
      }
      return linhas;
    },
    getUnidade() {
      return (nome) => {
        const unidades = {
          pm1: ' pm1.0/10^-4 m³',
          pm10: ' pm10.0/10^-4 m³',
          pm25: ' pm2.5/10^-4 m³',
          temperatura: ' °C',
          umidade: ' %',
          uv: ' UV index',
          ruido: ' dB',
          co: ' mg/m³',
          no2: ' µg/m³',
          voc: ' ppb',
          o3: ' ppb',
          co2: ' ppm',
        };
        return unidades[nome] || "";
      };
    }
  }
};
</script>

<style scoped>
.text-center {
  text-align: center;
}

.dashboard {
  padding: 1rem;
}

.sensor-card-title {
  text-align: center;
  font-weight: bold;
}

.sensor-chart {
  width: 100%;
  height: 100px;
  margin-top: 10px;
}

/* CSS novo para layout 3 por linha com alinhamento específico */
.sensor-rows {
  display: flex;
  flex-direction: column;
  gap: 1.5rem;
}

.sensor-row {
  display: flex;
  justify-content: space-between;
  margin: 0 -0.5rem; /* remove o padding lateral interno extra */
}

.sensor-card-wrapper {
  width: 33.33%;
  display: flex;
  padding: 0 0.5rem; /* espaço lateral interno para separação leve entre cards */
  box-sizing: border-box;
}

.align-left {
  justify-content: flex-start;
}

.align-center {
  justify-content: center;
}

.align-right {
  justify-content: flex-end;
}

.sensor-card {
  width: 100%;
}
</style>