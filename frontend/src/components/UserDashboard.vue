<template>
  <div class="dashboard">
    <h1>EST. AMBIENTAL</h1>
    
    <div class="grid">
      <div v-for="sensor in sensoresFormatados" :key="sensor.nome" class="col-12 md:col-6 lg:col-4">
        <router-link :to="'/sensor/' + sensor.nome">
          <Card>
            <template #title>{{ sensor.nome }}</template>
            <template #content>
              <div class="text-center">
                <span class="text-4xl font-bold">{{ sensor.valor }}</span>
                <span class="text-xl">{{ sensor.unidade }}</span>
                <Chart 
                  type="line"
                  :data="getSensorChartData(sensor.nome)"
                  :options="chartOptions"
                  class="sensor-chart"
                />
              </div>
            </template>
          </Card>
        </router-link>
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
      chartOptions: {
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
        pm25: 'PMA.5',
        temperatura: 'Temperatura',
        umidade: 'Umidade',
        ruido: 'Nível Sonoro',
        co: 'Monóxido de Carbono',
        no2: 'Dióxido de Nitrogênio',
        voc: 'COVs'
      };
      return names[name] || name;
    },
    getSensorChartData(sensorName) {
      return {
        labels: this.historico.map(d => new Date(d.timestamp).toLocaleTimeString()),
        datasets: [{
          data: this.historico.map(d => d[sensorName]),
          borderColor: '#3498db',
          tension: 0.1,
          borderWidth: 2
        }]
      };
    },
    async fetchData() {
      try {
        const [current, history] = await Promise.all([
          axios.get('http://localhost:5000/dados'),
          axios.get('http://localhost:5000/historico')
        ]);
        this.dados = current.data;
        this.historico = history.data;
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
          valor,
          unidade: this.getUnidade(nome)
        }));
    },
    getUnidade() {
      return (nome) => {
        const unidades = {
          temperatura: "°C",
          umidade: "%",
          co: "ppm",
          no2: "ppm",
          voc: "ppm",
          pm25: "µg/m³",
          pm10: "µg/m³",
          ruido: "dB"
        };
        return unidades[nome] || "";
      };
    }
  }
};
</script>

<style scoped>
.dashboard {
  padding: 1rem;
}

.sensor-chart {
  height: 100px;
  margin-top: 10px;
}
</style>