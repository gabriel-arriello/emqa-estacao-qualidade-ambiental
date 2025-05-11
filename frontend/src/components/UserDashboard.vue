<template>
  <div class="dashboard">
    <h1>ESTAÇÃO DE QUALIDADE AMBIENTAL</h1>
    
    <div class="grid">
      <div v-for="sensor in sensoresFormatados" :key="sensor.nome" class="col-12 md:col-6 lg:col-4">
        <router-link :to="'/sensor/' + sensor.columName">
          <Card>
            <template #title>{{ sensor.nome }}</template>
            <template #content>
              <div class="text-center">
                <span class="text-4xl font-bold">{{ sensor.valor }}</span>
                <span class="text-xl">{{ sensor.unidade }}</span>
                <Chart 
                  type="line"
                  :data="getSensorChartData(sensor.columName)"
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
        pm25: 'PM 2.5',
        pm10: 'PM 10',
        temperatura: 'Temperatura',
        umidade: 'Umidade',
        uv: 'Luz Ultravioleta',
        ruido: 'Ruído Sonoro',
        co: 'Monóxido de Carbono',
        no2: 'Dióxido de Nitrogênio',
        voc: 'VOCs'
      };
      return names[name] || name;
    },
    getSensorChartData(sensorName) {
      console.log("sensor name: " + sensorName);
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
        console.log(this.historico);
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