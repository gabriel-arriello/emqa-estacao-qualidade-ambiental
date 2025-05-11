  <template>
    <div class="sensor-detail">
      <Button 
        label="Voltar" 
        icon="pi pi-arrow-left" 
        @click="$router.go(-1)" 
        class="mb-4"
      />
      <Card>
        <template #title>{{ nomeSensor }}</template>
        <template #content>
          <div class="flex align-items-center gap-4 mb-4">
            <span class="text-6xl font-bold">{{ valorAtual }}</span>
            <span class="text-2xl">{{ unidade }}</span>
          </div>
          <Chart 
            type="line" 
            :data="chartData" 
            :options="chartOptions" 
            class="h-20rem"
          />
        </template>
      </Card>
    </div>
  </template>
  
  <script>
  import axios from 'axios';
  import Card from 'primevue/card';
  import Chart from 'primevue/chart';
  import Button from 'primevue/button';
  
  export default {
    components: { Card, Chart, Button },
    data() {
      console.log("id: " + this.$route.params.id)
      return {
        sensorId: this.$route.params.id,
        historico: [],
        pollingInterval: null,
      }
    },
    computed: {
      nomeSensor() {
        const nomes = {
          pm25: 'Material Particulado (PM2.5)',
          temperatura: 'Temperatura',
          umidade: 'Umidade',
          // Adicione todos os sensores...
        };
        return nomes[this.sensorId] || this.sensorId;
      },
      unidade() {
        const unidades = {
          pm25: 'µg/m³',
          temperatura: '°C',
          umidade: '%',
          // Adicione todas as unidades...
        };
        return unidades[this.sensorId] || '';
      },
      valorAtual() {
        return this.historico[0]?.[this.sensorId] || '--';
      },
      chartData() {
        return {
          labels: this.historico.map(d => new Date(d.timestamp).toLocaleTimeString()),
          datasets: [{
            label: this.nomeSensor,
            data: this.historico.map(d => d[this.sensorId]),
            borderColor: '#4BC0C0',
            tension: 0.4,
            fill: false
          }]
        }
      },
      chartOptions() {
        return {
          plugins: { legend: { display: false } },
          scales: { 
            y: { title: { display: true, text: this.unidade } },
            x: { title: { display: true, text: 'Horário' } } 
          }
        };
      }
    },
    methods: {
      async fetchData() {
        try {
          const response = await axios.get('http://localhost:5000/historico');
          this.historico = response.data.reverse(); // Mais recentes primeiro
          //console.log("historico: " + this.historico)
        } catch (error) {
          console.error("Erro ao obter histórico:", error);
        }
      }
    },
    mounted() {
      this.fetchData();
      this.pollingInterval = setInterval(this.fetchData, 5000); // Atualiza a cada 5s
    },
    beforeUnmount() {
      clearInterval(this.pollingInterval);
    }
  };
  </script>
  
  <style scoped>
  .sensor-detail {
    padding: 1rem;
    max-width: 1200px;
    margin: 0 auto;
  }
  </style>