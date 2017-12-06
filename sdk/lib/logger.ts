const chalk = require("chalk");
const { addColors, createLogger, format, transports } = require("winston");

const levels = {
    error: 1,
    warn: 2,
    progress: 3,
    success: 4,
    info: 5,
    recommend: 6,
    debug: 7,
};

const colors = {
    error: "red",
    warn: "yellow",
    progress: "blue",
    success: "green",
    recommend: "yellow",
    debug: "magenta",
};

addColors(colors);

export const logger = createLogger({
    level: process.env.LOG_LEVEL || "info",
    levels,
    transports: [
        new transports.Console(),
    ],
    format: format.combine(
        format.printf((info: any) => {
            return `${chalk.blue.bold(info.level.padStart(10) + ":")} ${info.message}`;
        }),
    ),
});
