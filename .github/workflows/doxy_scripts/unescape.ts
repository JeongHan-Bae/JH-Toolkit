import { cpSync, readFileSync, writeFileSync, readdirSync, rmSync } from 'node:fs';
import { join } from 'node:path';
import { decodeEntity } from 'html-entities';

// Decode only complete HTML character references. Doxygen expects source spelling
// after the repository's Doxygen-safe header annotations have been checked.
export function decodeEntities(source: string): string {
  return source.replace(/(?<!\\)&(#(?:x[0-9a-fA-F]+|[0-9]+)|[a-zA-Z][a-zA-Z0-9]+);/g,
    entity => decodeEntity(entity, { level: 'html5' }));
}

export function unescapeHeaders(root: string, buildRoot = join(root, '.doxy-build')): void {
  const stagedRoot = join(buildRoot, 'source');
  rmSync(stagedRoot, { recursive: true, force: true });
  const libraries = ['include', 'simple-cucumber/include', 'simple-cucumber/gherkin/include', 'tiny-test/include'];
  for (const library of libraries) cpSync(join(root, library), join(stagedRoot, library), { recursive: true });
  function walk(dir: string): void {
    for (const entry of readdirSync(dir, { withFileTypes: true })) {
      const path = join(dir, entry.name);
      if (entry.isDirectory()) walk(path);
      else if (entry.isFile() && (entry.name.endsWith('.h') || entry.name.endsWith('.hpp') || !entry.name.includes('.'))) {
        const source = readFileSync(path, 'utf8');
        const decoded = decodeEntities(source);
        if (decoded !== source) writeFileSync(path, decoded);
      }
    }
  }
  for (const library of libraries) walk(join(stagedRoot, library));
}

if (process.argv[1]?.endsWith('/unescape.ts')) unescapeHeaders(process.cwd());
