import path from 'node:path';

export function resolveRepoPath(relativePath: string): string {
    return path.resolve(__dirname, '..', '..', '..', '..', relativePath);
}
