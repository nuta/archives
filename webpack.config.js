const path = require("path");
const { DefinePlugin } = require("webpack");
const VueLoaderPlugin = require('vue-loader/lib/plugin');
const HtmlWebpackPlugin = require("html-webpack-plugin");

module.exports = {
    mode: process.env.WEBPACK_MODE,
    entry: "./ui/index.js",
    output: {
        filename: "index.js",
        path: path.resolve(__dirname, "dist/ui")
    },
    resolve: {
        alias: {
            render: path.resolve(__dirname, "dist/render.js")
        }
    },
    module: {
        rules: [
            {
                test: /\.vue$/,
                loader: "vue-loader",
                options: {
                    loaders: {
                        scss: [
                            "vue-style-loader",
                            "css-loader",
                            "postcss-loader",
                            "sass-loader"
                        ]
                    }
                }
            },
            {
                test: /\.scss$/,
                use: [
                    { loader: "style-loader" },
                    { loader: "css-loader" },
                    { loader: "postcss-loader" },
                    { loader: "sass-loader" },
                ]
            },
            {
                test: /\.css$/,
                use: [
                    { loader: "style-loader" },
                    { loader: "css-loader" },
                ]
            }
        ]
    },
    plugins: [
        new DefinePlugin({
            WEBPACK_MODE: JSON.stringify(process.env.WEBPACK_MODE)
        }),
        new VueLoaderPlugin(),
        new HtmlWebpackPlugin({
            template: path.resolve(__dirname, "ui/index.html")
        })
    ],
    devServer: {
        contentBase: [
            path.join(__dirname, "dist/ui"),
            path.join(__dirname, "examples")
        ],
        compress: true,
        port: 9000,
    }
}
