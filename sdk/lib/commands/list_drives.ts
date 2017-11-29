import * as chalk from 'chalk';
import { getAvailableDrives } from '../drive';

export async function main(args, opts, logger) {
  const drives = await getAvailableDrives()
  let i = 0
  for (const drive of Object.values(drives)) {
    if (i > 0) { console.log('') }

    console.log(`Drive: ${chalk.bold.green(drive.device)}`)
    console.log(`  name: ${drive.displayName}`)
    console.log(`  description: ${drive.description}`)
    console.log(`  size: ${drive.size / 1024 / 1024}MiB`)
    i++
  }
}
