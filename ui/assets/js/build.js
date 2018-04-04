export async function buildApp(appCode) {
  // App image is either zip or plain JavaScript source code.
  const { code, map } = Babel.transform(appCode, {
    sourceMaps: true,
    minified: true,
    presets: [
      ['es2015', { modules: false }]
    ]
  })

  return { image: code, debug: JSON.stringify(map) }
}
