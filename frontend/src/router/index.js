import { createRouter, createWebHistory } from 'vue-router'

// Lazy loading para melhor performance
const UserDashboard = () => import('@/components/UserDashboard.vue')
const SensorDetail = () => import('@/components/SensorDetail.vue')

const routes = [
  {
    path: '/',
    name: 'Dashboard',
    component: UserDashboard,
    meta: {
      title: 'Dashboard - Estação Ambiental'
    }
  },
  { 
    path: '/sensor/:id', 
    name: 'SensorDetail',
    component: SensorDetail,
    props: true,
    meta: {
      title: 'Detalhes do Sensor'
    }
  }
]

const router = createRouter({
  history: createWebHistory(process.env.BASE_URL),
  routes,
  scrollBehavior(to, from, savedPosition) {
    return savedPosition || { top: 0 }
  }
})

// Atualiza o título da página dinamicamente
router.beforeEach((to) => {
  document.title = to.meta.title || 'Estação Ambiental'
})

export default router