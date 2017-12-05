export function println(message) {
    process.send({ type: "log", body: message });
}

export function error(message) {
    println(`!${message}`);
}
