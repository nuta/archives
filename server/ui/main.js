import Vue from "vue";
import VueRouter from "vue-router";
import router from "./router";
import App from "./app";

Vue.use(VueRouter);
window.app = new Vue(Vue.util.extend({ router }, App));
app.$mount("#app");