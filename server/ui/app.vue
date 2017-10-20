<template>
  <div id="app">
    <router-view></router-view>
  </div>
</template>

<script>
export default {
};
</script>

<style lang="scss">
$default-font-color: #183f18;
$ui-fonts: "Open Sans", sans-serif;

html, body {
  height:100%;
}

body {
  margin: 0;
  font-size: 15px;
  font-family: $ui-fonts;
  color: $default-font-color;
}

#app {
  height: 100%;
}

section {
  margin-bottom: 20px;
}

fieldset {
  border: none;
  padding: 0;
  margin-bottom: 60px;
}

input[type=text], input[type=url], input[type=password] {
  padding: 10px;
  border-radius: 5px;
  font-size: 15px;
  width: 100%;
  display: block;
  border-radius: 5px;
  font-size: 15px;
  background: #ffffff;
  border: 1px solid #cfcfcf;
  box-sizing: border-box;
}

@mixin button {
  color: #777777;
  font-size: 14px;
  font-weight: 700;
  border: 1px solid #999999;
  border-radius: 8px;
  background: #ffffff;
  padding: 10px 13px;

  &:hover {
    color: #555555;
    border: 1px solid #777777;
    background: #f3f3f3;
    transition: background-color 0.7s ease;
    cursor: pointer;
  }
}

@mixin green-button {
  color: #ffffff;
  background: #47a068;
  border: none;
  padding: 10px 13px;
  font-size: 14px;
  font-weight: 700;
  border-radius: 8px;

  &:hover {
    background: #478568;
    transition: background-color 0.7s ease;
    cursor: pointer;
  }
}

input[type=submit] {
  @include green-button;
  margin-left: auto;
  margin-top: 15px;
  display: block;
}

button {
  @include button;

  &:not(:first-child) {
    margin-left: 5px;
  }

  &.green {
    @include green-button;
  }
}

select {
  // TODO
  appearance: none;
  font-size: 15px;
  box-sizing: border-box;
}

.center {
  text-align: center;
}


label {
  display: block;
  font-weight: bold;
  margin-bottom: 7px;

  &:not(:first-child) {
    margin-top: 18px;
  }
}

.columns {
  display: flex;

  &.right-aligned {
    justify-content: flex-end;
  }

  & > :not(:last-child) {
    margin-right: 10px;
  }
}

.two-columns {
  display: flex;
  justify-content: space-between;
}

.rows {
  display: flex;
  flex-direction: column;

  & > :not(:first-child) {
    margin-top: 10px;
  }
}

.caption {
  color: #6a6a6a;
  font-size: 0.8rem;
}

.undecorated-link {
  &:link  { color: $default-font-color; }
  &:visited { color: $default-font-color; }
  &:hover   { color: $default-font-color; }
  &:active  { color: $default-font-color; }
}

.list {
  border: 1px solid #cccccc;
  border-radius: 4px;
  padding: 10px;

  .element {
    display: flex;
    justify-content: space-between;
    padding: 10px 0;

    .title {
      font-weight: bold;
      margin-left: 10px;
    }

    a {
      color: $default-font-color;
      text-decoration: none;

      &:hover {
        text-decoration: underline;
      }
    }

    .metadata {
      display: inline-block;
      margin-right: 50px;

      tr td:first-child {
        padding-right: 30px;
        color: #888;

        i {
          padding-right: 5px;
        }
      }
    }

    .buttons {
      height: 100%;
      display: inline-block;
      vertical-align: middle;
    }
  }
}
</style>

