const path = require("path");
const HtmlWebpackPlugin = require("html-webpack-plugin");
const HtmlWebpackInlineSourcePlugin = require("html-webpack-inline-source-plugin");

const config = {
    mode: process.env.WEBPACK_MODE,
    entry: "./ui/index.js",
    output: {
        filename: "index.js",
        path: path.resolve(__dirname, "dist/ui")
    },
    module: {
        rules: [
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
        new HtmlWebpackPlugin({
            inlineSource: ".(js|css)$",
            template: path.resolve(__dirname, "ui/index.html")
        }),
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

if (process.env.WEBPACK_MODE === "production") {
    config.plugins.push(new HtmlWebpackInlineSourcePlugin());
}

module.exports = config;
