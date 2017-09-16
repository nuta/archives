export function describeAPI(key) {
  return {
    linux: "Full Linux Environment"
  }[key];
}

export function strftime(time) {
  // TODO
  let d = new Date(/^[-+]?\d*\.?\d*$/.test(time) ? parseFloat(time) * 1000 : time);
  let hours = (d.getHours() + 1).toString().padStart(2, "0");
  let mins = d.getMinutes().toString().padStart(2, "0");
  return `${hours}:${mins}`;
}