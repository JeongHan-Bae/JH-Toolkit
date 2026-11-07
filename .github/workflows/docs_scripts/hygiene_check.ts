import { resolve } from 'node:path';
import { repositoryFiles } from './filesystem.ts';
import { headerIssues } from './checks.ts';
import { cppComments } from './cpp_comments.ts';
import { publish } from './summary.ts';

const issues = headerIssues(repositoryFiles(resolve(process.cwd())), await cppComments());
const missing = issues.filter(item => item.missingCopyright).map(item => item.path);
const nonAscii = issues.filter(item => item.nonAscii).map(item => item.path);
const badNewline = issues.filter(item => item.badNewline).map(item => item.path);
const lines = ['# Hygiene Check'];
if (missing.length) lines.push('## ⚠️ Missing `@copyright`', 'The following public header files do not contain a `@copyright` notice:', '', ...missing.map(path => `- \`${path}\``), '');
if (nonAscii.length) lines.push('## ⚠️ Non-ASCII Public Headers', 'The following public headers contain non-ASCII characters. Public headers are required to be **pure ASCII** to ensure consistent cross-platform readability and tooling behavior:', '', ...nonAscii.map(path => `- \`${path}\``), '');
if (badNewline.length) lines.push('## ⚠️ Invalid EOF Newline', 'The following public headers must end with **exactly one `\\n`**. Files with no trailing newline or with multiple trailing newlines are invalid:', '', ...badNewline.map(path => `- \`${path}\``), '');
if (!missing.length && !nonAscii.length && !badNewline.length) lines.push('## ✅ Header Hygiene OK', 'All checked public headers passed hygiene checks.');
publish(lines, [
  ...(missing.length ? [`${missing.length} public header(s) missing @copyright`] : []),
  ...(nonAscii.length ? [`${nonAscii.length} public header(s) contain non-ASCII characters`] : []),
  ...(badNewline.length ? [`${badNewline.length} public header(s) have invalid EOF newline`] : []),
]);
