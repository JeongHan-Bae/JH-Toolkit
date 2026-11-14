import { spawnSync } from 'node:child_process';
import { resolve } from 'node:path';
import { publish } from './summary.ts';

interface UndocumentedDeclaration {
  path: string;
  line: number;
  kind: 'type' | 'function';
  description: string;
}

const root = resolve(process.cwd());
const result = spawnSync(process.execPath, [resolve(root, '.github/workflows/doxy_scripts/documentation_check.ts')], {
  cwd: root, encoding: 'utf8', maxBuffer: 32 * 1024 * 1024,
});
if (result.error) throw result.error;
if (result.status !== 0) throw new Error(`Doxygen documentation check failed: ${result.stderr}`);
const issues = JSON.parse(result.stdout) as UndocumentedDeclaration[];
const types = issues.filter(issue => issue.kind === 'type').length;
const functions = issues.length - types;
const lines = ['# Doxygen Type and Function Documentation Check'];
if (issues.length) {
  lines.push(`## ⚠️ ${types} undocumented type(s), ${functions} undocumented function(s)`, '',
    ...issues.map(issue => `- \`${issue.path}:${issue.line}\`: ${issue.description}`));
} else {
  lines.push('## ✅ Type and Function Documentation OK', 'Doxygen found no undocumented types or functions.');
}
publish(lines, issues.length ? [`Doxygen found ${types} undocumented type(s) and ${functions} undocumented function(s)`] : []);
