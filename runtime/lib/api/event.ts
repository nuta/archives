export function publish(event, data) {
  process.send({ type: 'log', body: `@${event} ${data}` })
}
