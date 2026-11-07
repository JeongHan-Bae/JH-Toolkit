import { execFileSync } from 'node:child_process';
import { join } from 'node:path';

export type LibraryVersions = {
  'jh-toolkit': string;
  'jh-simple-cucumber': string;
  'jh-tiny-test': string;
};

/** Read all library versions through the metadata generator, which owns CMake parsing. */
export function libraryVersions(root: string): LibraryVersions {
  const script = join(root, '.github/workflows/metadata_scripts/get_version.ts');
  const output = execFileSync(process.execPath, [script, '--all'], {
    cwd: root,
    encoding: 'utf8',
    stdio: ['ignore', 'pipe', 'pipe'],
  });
  const versions: unknown = JSON.parse(output);
  if (!versions || typeof versions !== 'object' || Array.isArray(versions))
    throw new Error('Metadata generator did not return library versions');
  for (const name of ['jh-toolkit', 'jh-simple-cucumber', 'jh-tiny-test'] as const) {
    const version = (versions as Record<string, unknown>)[name];
    if (typeof version !== 'string' || !/^\d+\.\d+\.\d+$/.test(version))
      throw new Error(`Metadata generator returned no valid version for ${name}`);
  }
  return versions as LibraryVersions;
}
