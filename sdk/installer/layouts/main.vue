<template>
<div class="main-wrapper">
    <slot></slot>
</div>
</template>

<script>
export default {

}
</script>

<style lang="scss" scoped>
header {
  background: linear-gradient(to left, #7474bf, #348ac7);

  h1 {
    text-align: center;
    font-weight: 200;
    font-size: 25px;
    padding: 35px 5px;
    color: #ffffff;
    margin: 0;
  }
}

.main-wrapper {
  width: 100vw;
  height: 100vh;
  background: #f6f6f6;
}

main {
  margin-top: 10px;
  margin-left: auto;
  margin-right: auto;
  width: 300px;
}

section {
  &:not(:first-child) {
    margin-top: 15px;
  }

  label {
    display: block;
    font-size: 15px;
    margin-bottom: 5px;
  }

  input[type=text], input[type=password], select {
    width: 100%;
    font-size: 15px;
  }

  input[type=checkbox] {
    margin-right: 10px;
  }

  input[type=submit] {
    width: 300px;
    margin-top: 25px;
    border-radius: 5px;
    padding: 10px 25px;
    text-align: center;
    font-size: 15px;
    font-weight: 600;
    background: #1e87f0;
    color: #ffffff;
    border: 1px solid transparent;

    &:active {
      background: #0e6dcd;
    }

    &:hover {
      background: #0f7ae5;
      cursor: pointer;
    }
  }
}
</style>
