import { api } from '../api';

export async function list(args, opts, logger) {
  for (const app of await api.getApps()) {
    logger.info(app)
  }
}

export async function create(args, opts, logger) {
  await api.createApp(args.name, opts.api)
}

export async function delete_(args, opts, logger) {
  await api.deleteApp(args.name)
}
