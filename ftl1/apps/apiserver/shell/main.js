function toHumanTime(ms) {
    const MINUTE = 60 * 1000;
    const HOUR = 60 * MINUTE;
    const DAY = 24 * HOUR;

    if (ms >= DAY)
        return Math.floor(ms / DAY) + "d";
    else if (ms >= HOUR)
        return Math.floor(ms / HOUR) + "h";
    else if (ms >= MINUTE)
        return Math.floor(ms / MINUTE) + "m";
    else
        return Math.floor(ms / 1000) + "s";
}

async function main() {
    const resp = await fetch("/uptime");
    const body = await resp.text();
    const uptimeMs = parseInt(body);
    document.getElementById("uptime").textContent = toHumanTime(uptimeMs);
}

document.addEventListener("DOMContentLoaded", () => {
    main().catch(console.error);
});
