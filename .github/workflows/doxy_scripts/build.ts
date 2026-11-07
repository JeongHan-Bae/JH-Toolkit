import { appendFileSync, rmSync } from 'node:fs';
import { spawnSync } from 'node:child_process';
import { resolve, join } from 'node:path';
import { generateDoxyfile } from './doxyfile.ts';
import { unescapeHeaders } from './unescape.ts';

const root = resolve(process.cwd());
const args = process.argv.slice(2);
rmSync(join(root, '.doxy-build'), { recursive: true, force: true });
unescapeHeaders(root);
const version = await generateDoxyfile(root, {
  html: !args.includes('--no-html'),
  xml: args.includes('--use-xml'),
  repository: process.env.GITHUB_REPOSITORY,
  token: process.env.GITHUB_TOKEN,
});
const result = spawnSync('doxygen', [join(root, '.doxy-build', 'Doxyfile')], {
  cwd: root,
  stdio: 'inherit',
});
if (result.error) throw result.error;
if (result.status !== 0) process.exit(result.status ?? 1);
if (process.env.GITHUB_ENV) appendFileSync(process.env.GITHUB_ENV, `PROJECT_VERSION=${version}\n`);
