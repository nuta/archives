import * as fs from 'fs';
import * as path from 'path';
import * as nunjuck from 'nunjucks';
import { spawnSync } from 'child_process';
import { logger } from '../logger';
import { createFile } from '../helpers';
import { prepare } from '../prepare';

const DEFAULT_TEMPLATES = [
  {
    filepath: 'README.md',
    template: `\
{{ appName }}
=============
`
  },
  {
    filepath: 'app.yaml',
    template: `\
name: {{ appName }}
plugins: [ {{ plugins | join(", ") }} ]
`
  },
  {
    filepath: 'jsconfig.json',
    template: `\
{
  "compilerOptions": {
    "target": "es2017",
    "module": "commonjs",
    "lib": ["es2017"]
  }
}
`
  },
  {
    filepath: 'app.js',
    template: `\
Timer.interval(3, () => {
  println('Hello World!')
})
`
  }
]

export async function main(args, opts) {
  const appName = path.basename(args.dir)
  const plugins = opts.plugins.split(',')
  let templates = DEFAULT_TEMPLATES
  const context = {
    appName, plugins
  }

  for (const { filepath, template } of templates) {
    logger.progress(`Creating ${filepath}`)
    createFile(path.join(args.dir, filepath), nunjuck.renderString(template, context))
  }

  await prepare(args.dir)

  let appJS = fs.readFileSync(path.join(args.dir, 'app.js'), {
    encoding: 'utf-8'
  })

  for (const plugin of plugins) {
    const scaffoldPath = path.resolve(args.dir, `node_modules/@makestack/${plugin}/scaffold.js`)
    if (fs.existsSync(scaffoldPath)) {
      logger.progress(`Scaffolding ${plugin}`)
      const { scaffold } = require(scaffoldPath)
      const { header = '', footer = '' } = scaffold(context)
      appJS =
        header.replace(/\n+$/, '') + "\n\n" +
        appJS.replace(/\n+$/, '') + "\n\n" +
        footer.replace(/\n+$/, '') + "\n"
    }
  }

  if (appJS) {
    fs.writeFileSync(path.join(args.dir, 'app.js'), appJS)
  }
}
