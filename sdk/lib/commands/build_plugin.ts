import { buildPlugin } from '../plugin_builder';

export function main(args, opts, logger) {
  buildPlugin(args.path, args.dest)
}
