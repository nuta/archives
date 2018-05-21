const path = require("path")
const CopyWebpackPlugin = require("copy-webpack-plugin")

module.exports = {
  entry: {
    app: ["./lib/index.ts"]
  },
  output: {
    path: path.resolve(__dirname, "docs"),
    publicPath: "/",
    filename: "esptool.js"
  },
  resolve: {
    extensions: [".ts", ".js"],
    alias: {
      './node': path.resolve('lib/node.web')
    }
  },
  devtool: "",
  module: {
    loaders: [
      {
        test: /\.ts$/,
        loader: "ts-loader"
      }
    ]
  },
  plugins: [
    new CopyWebpackPlugin([
      { from: './*.html' }
    ])
  ]
};
