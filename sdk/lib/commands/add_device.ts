import { api } from '../api';
import { loadAppYAML } from '../appdir';

export async function main(args, opts, logger) {
  const appName = loadAppYAML(opts.appDir).name

  await api.updateDevice(args.deviceName, {
    app: appName
  })
}
