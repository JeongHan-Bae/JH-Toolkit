import { resolve } from 'node:path';
import { repositoryFiles } from './filesystem.ts';
import { missingDocumentation } from './checks.ts';
import { publish } from './summary.ts';

const missing = missingDocumentation(repositoryFiles(resolve(process.cwd())));
const lines = ['# Presence Check'];
if (missing.length) {
  lines.push('## ⚠️ Missing Documentation Detected', '',
    'The following public headers do not have corresponding documentation files:', '',
    ...missing.map(path => `- \`${path}\``), '',
    '> This check **does not** verify whether documentation content is up-to-date.\n> It only checks for **missing documentation files**.');
} else {
  lines.push('## ✅ Documentation Presence OK', '', 'All public headers have corresponding documentation files.');
}
publish(lines, missing.length ? [`Missing documentation detected (${missing.length} files)`] : []);
