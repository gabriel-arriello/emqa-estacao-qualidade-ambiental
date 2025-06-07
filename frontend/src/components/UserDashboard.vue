<template>
  <div class="dashboard">
    <h1 class="text-center mb-4">ESTAÇÃO DE QUALIDADE AMBIENTAL</h1>

    <!-- Cards principais -->
    <div class="main-data-row">
      <div class="main-data-card" @click="openChart('temperatura')">
        <div class="main-value">{{ dados.temperatura || "--" }} °C</div>
        <div class="main-label">Temperatura</div>
      </div>

      <div class="main-data-card" @click="openChart('umidade')">
        <div class="main-value">{{ dados.umidade || "--" }} %</div>
        <div class="main-label">Umidade</div>
      </div>

      <div class="main-data-card" @click="openChart('iqar')">
        <div class="main-value">{{ iqArAtual || "--" }}</div>
        <div class="main-label">IQAr</div>
      </div>
    </div>

    <!-- Modal para gráficos -->
    <Dialog
      :visible="showChart"
      :modal="true"
      :style="{ width: '80vw' }"
      @update:visible="showChart = false"
    >
      <template #header>
        <h3>{{ chartTitle }}</h3>
      </template>

      <div class="chart-container">
        <Chart
          type="line"
          :data="chartData"
          :options="chartOptions"
          style="height: 60vh"
          ref="modalChart"
        />
      </div>

      <template #footer>
        <Button label="Fechar" @click="showChart = false" />
      </template>
    </Dialog>

    <!-- Tabela de IQAr -->
    <table class="iqar-table">
      <thead>
        <tr>
          <th>Poluente</th>
          <th>Valor</th>
          <th>IQAr</th>
          <th>Classificação</th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="p in poluentes" :key="p.nome">
          <td>{{ p.nome }}</td>
          <td>{{ p.valor }}</td>
          <td>{{ p.iqar }}</td>
          <td>{{ classificaIQAr(p.iqar) }}</td>
        </tr>
      </tbody>
    </table>

    <!-- Sensores secundários -->
    <div class="sensor-rows">
      <div
        class="sensor-row"
        v-for="(row, rowIndex) in linhasDeSensores"
        :key="rowIndex"
      >
        <div
          v-for="sensor in row"
          :key="sensor.columName"
          class="sensor-card-wrapper"
        >
          <Card
            class="cursor-pointer sensor-card"
            @click="$router.push('/sensor/' + sensor.columName)"
          >
            <template #title>
              <div class="sensor-card-title">{{ sensor.nome }}</div>
            </template>
            <template #content>
              <div class="text-center">
                <span class="text-4xl font-bold">{{
                  sensor.valor || "--"
                }}</span>
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
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import Card from "primevue/card";
import Chart from "primevue/chart";
import Button from "primevue/button";
import Dialog from "primevue/dialog";
import axios from "axios";

// Faixas para cálculo do IQAr
const faixas = [
  { ini: 0, fin: 40, cini: 0, cfin: 45 },
  { ini: 41, fin: 80, cini: 46, cfin: 100 },
  { ini: 81, fin: 120, cini: 101, cfin: 150 },
  { ini: 121, fin: 200, cini: 151, cfin: 250 },
  { ini: 201, fin: 400, cini: 251, cfin: 600 },
];

// Função para calcular o IQAr
function calculaIQAr(valor, faixas) {
  for (const faixa of faixas) {
    if (valor >= faixa.cini && valor <= faixa.cfin) {
      return Math.round(
        faixa.ini +
          ((faixa.fin - faixa.ini) / (faixa.cfin - faixa.cini)) *
            (valor - faixa.cini)
      );
    }
  }
  return null;
}

export default {
  components: { Card, Chart, Button, Dialog },
  data() {
    return {
      showChart: false,
      chartSensor: null,
      chartTitle: "",
      chartData: null,
      // Incluir CO2 e VOCs apenas nos gráficos secundários
      ordemSensores: [
        "uv",
        "co",
        "co2",
        "no2",
        "o3",
        "voc",
        "pm1",
        "pm25",
        "pm10",
        "ruido",
      ],
      dados: {},
      historico: [],
      chartOptions: {
        responsive: true,
        maintainAspectRatio: false,
        animation: { duration: 0 },
        plugins: { legend: { display: false } },
        scales: {
          y: { title: { display: true } },
          x: { title: { display: true, text: "Tempo" } },
        },
      },
      pollingInterval: null,
    };
  },
  computed: {
    iqArAtual() {
      // Poluentes considerados no cálculo do IQAr (oficiais)
      const poluentes = ["pm1", "pm10", "pm25", "co", "no2", "o3"];

      // Encontra o maior IQAr entre os poluentes
      const iqars = poluentes
        .map((p) => {
          const valor = this.dados[p] || 0;
          return calculaIQAr(valor, faixas);
        })
        .filter((v) => v !== null);

      return iqars.length ? Math.max(...iqars) : null;
    },
    poluentes() {
      // Apenas poluentes oficiais na tabela
      return [
        { chave: "pm1", nome: "PM1.0", valor: this.dados.pm1 || 0 },
        { chave: "pm10", nome: "PM10.0", valor: this.dados.pm10 || 0 },
        { chave: "pm25", nome: "PM2.5", valor: this.dados.pm25 || 0 },
        { chave: "co", nome: "CO", valor: this.dados.co || 0 },
        { chave: "no2", nome: "NO2", valor: this.dados.no2 || 0 },
        { chave: "o3", nome: "O3", valor: this.dados.o3 || 0 },
      ].map((p) => ({
        ...p,
        iqar: calculaIQAr(p.valor, faixas),
      }));
    },
    sensoresFormatados() {
      return this.ordemSensores
        .map((nome) => {
          return {
            nome: this.formatSensorName(nome),
            columName: nome,
            valor: this.dados[nome],
            unidade: this.getUnidade(nome),
          };
        })
        .filter((sensor) => sensor.valor !== undefined);
    },
    linhasDeSensores() {
      const linhas = [];
      for (let i = 0; i < this.sensoresFormatados.length; i += 3) {
        linhas.push(this.sensoresFormatados.slice(i, i + 3));
      }
      return linhas;
    },
  },
  methods: {
    openChart(sensor) {
      this.chartSensor = sensor;
      this.chartTitle = this.formatSensorName(sensor);
      this.prepareChartData();
      this.showChart = true;
    },
    classificaIQAr(iqar) {
      if (!iqar) return "--";
      if (iqar <= 40) return "Boa";
      if (iqar <= 80) return "Moderada";
      if (iqar <= 120) return "Ruim";
      if (iqar <= 200) return "Muito Ruim";
      return "Péssima";
    },
    formatSensorName(name) {
      const names = {
        pm1: "Material Particulado 1.0 (PM1.0)",
        pm10: "Material Particulado 10.0 (PM10.0)",
        pm25: "Material Particulado 2.5 (PM2.5)",
        temperatura: "Temperatura",
        umidade: "Umidade",
        uv: "Luz Ultravioleta (UV)",
        ruido: "Ruído Sonoro",
        co: "Monóxido de Carbono (CO)",
        no2: "Dióxido de Nitrogênio (NO2)",
        voc: "Compostos Orgânicos Voláteis (VOCs)",
        o3: "Trioxigênio (O3)",
        co2: "Dióxido de Carbono (CO2)",
        iqar: "Índice de Qualidade do Ar (IQAr)",
      };
      return names[name] || name;
    },
    getUnidade(name) {
      const unidades = {
        pm1: " pm1.0/10^-4 m³",
        pm10: " pm10.0/10^-4 m³",
        pm25: " pm2.5/10^-4 m³",
        temperatura: " °C",
        umidade: " %",
        uv: " UV index",
        ruido: " dB",
        co: " ppm",
        no2: " ppm",
        voc: " ppb",
        o3: " ppb",
        co2: " ppm",
      };
      return unidades[name] || "";
    },
    getSensorChartData(sensorName) {
      const labels = this.historico.map((d) =>
        new Date(d.timestamp).toLocaleTimeString()
      );
      const data = this.historico.map((d) => d[sensorName]);

      return {
        labels,
        datasets: [
          {
            data,
            borderColor: "#3498db",
            tension: 0.1,
            borderWidth: 2,
          },
        ],
      };
    },
    prepareChartData() {
      if (this.chartSensor === "iqar") {
        // Calcular IQAr histórico
        const labels = this.historico.map((d) =>
          new Date(d.timestamp).toLocaleTimeString()
        );

        const data = this.historico.map((d) => {
          // Considerar apenas poluentes oficiais para cálculo
          const poluentes = ["pm1", "pm10", "pm25", "co", "no2", "o3"];
          const valores = poluentes.map((p) => d[p] || 0);
          const maxValor = Math.max(...valores);
          return calculaIQAr(maxValor, faixas);
        });

        this.chartData = {
          labels,
          datasets: [
            {
              label: "IQAr",
              data,
              borderColor: "#e74c3c",
              tension: 0.4,
              fill: false,
            },
          ],
        };
      } else {
        // Dados normais de sensor
        const labels = this.historico.map((d) =>
          new Date(d.timestamp).toLocaleTimeString()
        );
        const data = this.historico.map((d) => d[this.chartSensor]);

        this.chartData = {
          labels,
          datasets: [
            {
              label: this.formatSensorName(this.chartSensor),
              data,
              borderColor: "#3498db",
              tension: 0.4,
              fill: false,
            },
          ],
        };
      }

      // Configurar unidade no eixo Y
      this.chartOptions = {
        ...this.chartOptions,
        scales: {
          y: {
            title: {
              display: true,
              text:
                this.chartSensor === "iqar"
                  ? "IQAr"
                  : this.getUnidade(this.chartSensor),
            },
          },
          x: {
            title: {
              display: true,
              text: "Tempo",
            },
          },
        },
      };
    },
    async fetchData() {
      try {
        const [current, history] = await Promise.all([
          axios.get("http://localhost:5000/dados"),
          axios.get("http://localhost:5000/historico"),
        ]);

        this.dados = current.data;
        this.historico = history.data;

        // Se o modal estiver aberto, atualize o gráfico
        if (this.showChart) {
          this.prepareChartData();
          this.$nextTick(() => {
            if (this.$refs.modalChart && this.$refs.modalChart.chart) {
              this.$refs.modalChart.chart.update();
            }
          });
        }
      } catch (error) {
        console.error("Erro ao obter dados:", error);
      }
    },
  },
  mounted() {
    this.fetchData();
    this.pollingInterval = setInterval(this.fetchData, 5000);
  },
  beforeUnmount() {
    clearInterval(this.pollingInterval);
  },
};
</script>

<style scoped>
.dashboard {
  padding: 1rem;
  max-width: 1200px;
  margin: 0 auto;
}

.text-center {
  text-align: center;
}

/* Layout dos cards principais */
.main-data-row {
  display: flex;
  justify-content: space-between;
  gap: 1rem;
  margin-bottom: 2rem;
}

.main-data-card {
  flex: 1;
  padding: 1.5rem;
  background-color: #f8f9fa;
  border-radius: 8px;
  text-align: center;
  cursor: pointer;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
  transition: all 0.3s ease;
}

.main-data-card:hover {
  transform: translateY(-5px);
  box-shadow: 0 4px 8px rgba(0, 0, 0, 0.15);
}

.main-value {
  font-size: 2.5rem;
  font-weight: bold;
  margin-bottom: 0.5rem;
}

.main-label {
  font-size: 1.2rem;
  color: #495057;
}

/* Tabela de IQAr */
.iqar-table {
  width: 100%;
  border-collapse: collapse;
  margin-bottom: 2rem;
  background-color: white;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
  border-radius: 8px;
  overflow: hidden;
}

.iqar-table th,
.iqar-table td {
  padding: 0.75rem 1rem;
  text-align: center;
  border-bottom: 1px solid #dee2e6;
}

.iqar-table th {
  background-color: #e9ecef;
  font-weight: 600;
}

.iqar-table tbody tr:last-child td {
  border-bottom: none;
}

/* Sensores secundários */
.sensor-rows {
  display: flex;
  flex-direction: column;
  gap: 1.5rem;
}

.sensor-row {
  display: flex;
  gap: 1.5rem;
}

.sensor-card-wrapper {
  flex: 1;
}

.sensor-card {
  height: 100%;
}

.sensor-card-title {
  text-align: center;
  font-weight: bold;
  margin-bottom: 1rem;
}

.sensor-chart {
  height: 160px;
  margin-top: 1rem;
}

.chart-container {
  padding: 1rem;
}
</style>
