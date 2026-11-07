import { test } from 'node:test';
import { deepStrictEqual, strictEqual } from 'node:assert';
import { execFileSync } from 'node:child_process';
import { resolve, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';

const root = resolve(dirname(fileURLToPath(import.meta.url)), '../../..');
const script = resolve(dirname(fileURLToPath(import.meta.url)), 'get_version.ts');
const run = (...args: string[]) => execFileSync(process.execPath, [script, ...args], {
  cwd: root, encoding: 'utf8',
}).trim();

test('version CLI returns the toolkit version and all three named library versions', () => {
  const versions = JSON.parse(run('--all')) as Record<string, string>;
  deepStrictEqual(Object.keys(versions), ['jh-toolkit', 'jh-simple-cucumber', 'jh-tiny-test']);
  for (const version of Object.values(versions)) strictEqual(/^\d+\.\d+\.\d+$/.test(version), true);
  strictEqual(run(), versions['jh-toolkit']);
});
