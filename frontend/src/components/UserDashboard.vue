<template>
  <div class="dashboard">
    <h1 class="text-center mb-4">ESTAÇÃO DE QUALIDADE AMBIENTAL</h1>

    <!-- Cards principais -->
    <div class="main-data-row">
      <div class="main-data-card" @click="openChart('temperatura')">
        <div class="main-value">{{ dados.temperatura }} °C</div>
        <div class="main-label">Temperatura</div>
      </div>

      <div class="main-data-card" @click="openChart('umidade')">
        <div class="main-value">{{ dados.umidade }} %</div>
        <div class="main-label">Umidade</div>
      </div>

      <div class="main-data-card">
        <div class="main-value">{{ iqArAtual }}</div>
        <div class="main-label">IQAr</div>
      </div>
    </div>

    <!-- Modal para gráficos permaneçe apenas para temperatura/umidade -->
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

    <!-- Tabela de Sensores Geral -->
    <table class="iqar-table">
      <thead>
        <tr>
          <th>Poluente</th>
          <th>Valor</th>
          <th>Unidade</th>
          <th>Índice</th>
          <th>Classificação</th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="item in tabelaSensores" :key="item.chave">
          <td>{{ item.nome }}</td>
          <td>{{ item.valor }}</td>
          <td>{{ item.unidade }}</td>
          <td>{{ item.indice }}</td>
          <td>{{ item.classificacao }}</td>
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
            class="sensor-card clickable-card"
            @click="$router.push('/sensor/' + sensor.columName)"
          >
            <template #title>
              <div class="sensor-card-title">{{ sensor.nome }}</div>
            </template>
            <template #content>
              <div class="text-center">
                <span class="text-sm font-medium">Última leitura: </span>
                <span class="text-4xl font-bold">{{
                  sensor.valor
                }}</span>
                <span class="text-xl">{{ sensor.unidade }}</span>
                <Chart
                  type="line"
                  :data="getSensorChartData(sensor.columName)"
                  :options="getChartOptions(sensor.columName)"
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
const niveisIQAr = [
  { ini: 0, fin: 40 },
  { ini: 41, fin: 80 },
  { ini: 81, fin: 120 },
  { ini: 121, fin: 200 },
  { ini: 201, fin: 400 },
];

// Faixas de concentração dos poluentes para cálculo dos índices
const faixasConcentracao = {
  pm10: [
    { cini: 0, cfin: 45 },
    { cini: 46, cfin: 100 },
    { cini: 101, cfin: 150 },
    { cini: 151, cfin: 250 },
    { cini: 251, cfin: 600 },
  ],
  pm25: [
    { cini: 0, cfin: 15 },
    { cini: 16, cfin: 50 },
    { cini: 51, cfin: 75 },
    { cini: 76, cfin: 125 },
    { cini: 126, cfin: 300 },
  ],
  o3: [
    { cini: 0, cfin: 100 },
    { cini: 101, cfin: 130 },
    { cini: 131, cfin: 160 },
    { cini: 161, cfin: 200 },
    { cini: 201, cfin: 800 },
  ],
  co: [
    { cini: 0, cfin: 9 },
    { cini: 10, cfin: 11 },
    { cini: 12, cfin: 13 },
    { cini: 14, cfin: 15 },
    { cini: 16, cfin: 50 },
  ],
  no2: [
    { cini: 0, cfin: 200 },
    { cini: 201, cfin: 240 },
    { cini: 241, cfin: 320 },
    { cini: 321, cfin: 1130 },
    { cini: 1131, cfin: 3750 },
  ],
};

// Função para calcular o IQAr
function calculaIQAr(poluente, valor) {
  const concentracoes = faixasConcentracao[poluente];
  if (!concentracoes) return null; // não aplica a outros sensores

  for (let i = 0; i < concentracoes.length; i++) {
    const { cini, cfin } = concentracoes[i];
    if (valor >= cini && valor <= cfin) {
      const { ini, fin } = niveisIQAr[i];
      // interpolação linear dentro do nível
      return Math.round(ini + ((fin - ini) / (cfin - cini)) * (valor - cini));
    }
  }
  return null;
}

// Funções para classificar índices baseado nas concentrações
function classificaIQAr(iqar) {
  if (!iqar) return "--";
  if (iqar <= 40) return "Boa";
  if (iqar <= 80) return "Moderada";
  if (iqar <= 120) return "Ruim";
  if (iqar <= 200) return "Muito Ruim";
  return "Péssima";
}

function classificaCO2(valor) {
  if (valor <= 1000) return "Boa";
  if (valor <= 2500) return "Moderada";
  if (valor <= 5000) return "Ruim";
  if (valor <= 40000) return "Muito ruim";
  return "Péssima";
}

function classificaVOC(valor) {
  if (valor <= 220) return "Ótima";
  if (valor <= 660) return "Boa";
  if (valor <= 1430) return "Moderada";
  if (valor <= 2200) return "Ruim";
  if (valor <= 3300) return "Muito ruim";
  return "Péssima";
}

function classificaUV(valor) {
  if (valor <= 2) return "Boa";
  if (valor <= 5) return "Moderada";
  if (valor <= 7) return "Ruim";
  if (valor <= 10) return "Muito ruim";
  return "Péssima";
}

function classificaDB(valor) {
  if (valor <= 40) return "Ótima";
  if (valor <= 60) return "Boa";
  if (valor <= 80) return "Moderada";
  if (valor <= 100) return "Ruim";
  return "Muito ruim";
}

export default {
  components: { Card, Chart, Button, Dialog },
  data() {
    return {
      showChart: false,
      chartSensor: null,
      chartTitle: "",
      chartData: null,
      ordemSensores: [
        // "pm1",
        "pm25",
        "pm10",
        "o3",
        "no2",
        "co",
        "co2",
        "voc",
        "uv",
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
          x: { title: { display: true, text: "Tempo (hh-mm-ss)" } },
          y: { title: { display: true } },
        },
      },
      pollingInterval: null,
    };
  },
  computed: {
    tabelaSensores() {
      return this.ordemSensores.map((chave) => {
        const valor = this.dados[chave];
        const unidade = this.getUnidade(chave) || "";
        let indice = "-";
        let classificacao = "-";

        switch (chave) {
          case "co2": {
            indice = "CO2 levels";
            classificacao = classificaCO2(valor);
            break;
          }
          case "voc": {
            indice = "VOC's levels";
            classificacao = classificaVOC(valor);
            break;
          }
          case "uv": {
            indice = "UV levels";
            classificacao = classificaUV(valor)
            break;
          }
          case "ruido": {
            indice = "dB levels";
            classificacao = classificaDB(valor);
            break;
          }
          default: {
            const iqar = calculaIQAr(chave, valor);
            indice = "IQAr";
            classificacao = classificaIQAr(iqar);
          }
        }

        return {
          chave,
          nome: this.formatSensorName(chave),
          valor,
          unidade,
          indice,
          classificacao,
        };
      });
    },
    sensoresFormatados() {
      return this.ordemSensores
        .map((nome) => ({
          nome: this.formatSensorName(nome),
          columName: nome,
          valor: this.dados[nome],
          unidade: this.getUnidade(nome),
        }))
        .filter((sensor) => sensor.valor !== undefined);
    },
    linhasDeSensores() {
      const linhas = [];
      for (let i = 0; i < this.sensoresFormatados.length; i += 3) {
        linhas.push(this.sensoresFormatados.slice(i, i + 3));
      }
      return linhas;
    },
    iqArAtual() {
      // recalcula IQAr atual mesmo sem gráfico
      const valores = this.ordemSensores
        .map((chave) => {
          const iqar = calculaIQAr(chave, this.dados[chave] || 0);
          return iqar;
        })
        .filter((v) => v != null);
      return valores.length ? Math.max(...valores) : null;
    },
  },
  methods: {
    openChart(sensor) {
      this.chartSensor = sensor;
      this.chartTitle = this.formatSensorName(sensor);
      this.prepareChartData();
      this.showChart = true;
    },
    formatSensorName(name) {
      const names = {
        //pm1: "Material Particulado 1.0 (PM1.0)",
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
        //pm1: "µg/m³",
        pm10: " µg/m³",
        pm25: " µg/m³",
        temperatura: " °C",
        umidade: " %",
        uv: " UV index",
        ruido: " dB",
        co: " ppm",
        no2: " µg/m³",
        voc: " ppb",
        o3: " µg/m³",
        co2: " ppm",
      };
      return unidades[name] || "";
    },
    getSensorChartData(sensorName) {
      const maxPoints = 10;
      // Pega só os últimos oito registros
      const recent = this.historico.slice(-maxPoints);

      // Extrai rótulos e valores
      const labels = recent.map((d) =>
        new Date(d.timestamp).toLocaleTimeString()
      );
      const data = recent.map((d) => d[sensorName]);

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

    // Novo método para opções dinâmicas
    getChartOptions(sensorName) {
      const base = {
        responsive: true,
        maintainAspectRatio: false,
        animation: { duration: 0 },
        plugins: { legend: { display: false } },
      };
      return {
        ...base,
        scales: {
          x: {
            title: {
              display: true,
              text: "Tempo (hh-mm-ss)",
            },
          },
          y: {
            title: {
              display: true,
              text: this.getUnidade(sensorName),
              rotation: 90,
            },
          },
        },
      };
    },
    prepareChartData() {
      if (this.chartSensor === "iqar") {
        const labels = this.historico.map((d) =>
          new Date(d.timestamp).toLocaleTimeString()
        );

        const data = this.historico.map((d) => {
          const poluentes = ["pm1", "pm10", "pm25", "co", "no2", "o3"];
          const valores = poluentes.map((p) => d[p] || 0);
          const maxValor = Math.max(...valores);
          return calculaIQAr(maxValor, faixasConcentracao);
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

        this.chartOptions = this.getChartOptions("iqar");
      } else {
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
          axios.get("http://192.168.196.197:5000/dados"),
          axios.get("http://192.168.196.197:5000/historico"),
        ]);

        this.dados = current.data;
        this.historico = history.data;

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
    this.pollingInterval = setInterval(this.fetchData, 10000);
  },
  beforeUnmount() {
    clearInterval(this.pollingInterval);
  },
};
</script>

<style scoped>
/* Layout geral */
.dashboard {
  max-width: 1200px;
  margin: 0 auto;
  padding: 1rem;
}

.text-center {
  text-align: center;
}

/* Cards principais */
.main-data-row {
  display: flex;
  gap: 1rem;
  margin-bottom: 2rem;
}

.main-data-card {
  flex: 1;
  padding: 1.5rem;
  background-color: #f8f9fa;
  border-radius: 8px;
  text-align: center;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
  cursor: pointer;
  transition: transform 0.3s ease, box-shadow 0.3s ease;
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

/* Tabela de Sensores Geral */
.iqar-table {
  width: 100%;
  margin-bottom: 2rem;
  border-collapse: collapse;
  background-color: #fff;
  border-radius: 8px;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
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

/* Cards dos sensores secundários */
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
  background-color: #fff;
  border-radius: 8px;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
  transition: transform 0.3s ease, box-shadow 0.3s ease;
  cursor: pointer;
}

.sensor-card:hover {
  transform: translateY(-5px);
  box-shadow: 0 4px 8px rgba(0, 0, 0, 0.15);
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
