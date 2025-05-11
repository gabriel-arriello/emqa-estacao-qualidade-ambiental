<template>
    <div class="sensor-card" :class="nivel">
      <h3>{{ nomeFormatado }}</h3>
      <p class="valor">{{ valor }} {{ unidade }}</p>
    </div>
  </template>
  
  <script>
  export default {
    name: "SensorCard",
    props: ["nome", "valor", "unidade"],
    computed: {
      nomeFormatado() {
        return this.nome.toUpperCase();
      },
      nivel() {
        // aqui você pode aplicar lógicas de cor por tipo
        if (this.nome.includes("temperatura")) {
          if (this.valor >= 30) return "alto";
          else if (this.valor >= 20) return "medio";
          else return "baixo";
        }
        // Adicione outras lógicas conforme quiser
        return "";
      }
    }
  };
  </script>
  
  <style scoped>
  .sensor-card {
    padding: 1rem;
    border-radius: 12px;
    box-shadow: 0 2px 8px rgba(0,0,0,0.1);
    margin: 10px;
    text-align: center;
    transition: 0.3s;
  }
  .alto {
    background-color: #f8d7da;
  }
  .medio {
    background-color: #fff3cd;
  }
  .baixo {
    background-color: #d4edda;
  }
  .valor {
    font-size: 1.4rem;
    font-weight: bold;
  }
  </style>