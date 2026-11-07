import { existsSync, readFileSync, readdirSync } from 'node:fs';
import { join, relative } from 'node:path';
import type { FilePort } from './ports.ts';

export function repositoryFiles(root: string): FilePort {
  const headerRoot = join(root, 'include', 'jh');
  function walk(dir: string): string[] {
    return readdirSync(dir, { withFileTypes: true }).flatMap(entry => {
      const path = join(dir, entry.name);
      if (entry.isDirectory()) return walk(path);
      return entry.isFile() && (entry.name.endsWith('.h') || !entry.name.includes('.'))
        ? [relative(root, path).split('\\').join('/')]
        : [];
    });
  }
  return {
    publicHeaders: () => walk(headerRoot).sort(),
    exists: path => existsSync(join(root, path)),
    read: path => readFileSync(join(root, path)),
  };
}
