import { test } from 'node:test';
import { deepStrictEqual, strictEqual } from 'node:assert';
import { missingDocumentation, headerIssues } from './checks.ts';
import { cppComments } from './cpp_comments.ts';
import type { FilePort } from './ports.ts';

const contents = new Map([
  ['include/jh/widget.h', Buffer.from('/** @copyright */\n#pragma once\n')],
  ['include/jh/detail/internal.h', Buffer.from('#pragma once\n')],
  ['include/jh/macros/header_begin.h', Buffer.from('#pragma once\n')],
  ['include/jh/group', Buffer.from('#pragma once\n')],
]);
const files: FilePort = {
  publicHeaders: () => [...contents.keys()],
  exists: path => path === 'docs/detail/internal.md',
  read: path => contents.get(path)!,
};

test('presence maps public .h headers and excludes internal headers', () => {
  deepStrictEqual(missingDocumentation(files), ['include/jh/widget.h']);
});

test('hygiene reads syntax comments and checks bytes and trailing newline', async () => {
  const comments = await cppComments();
  strictEqual(comments.hasCopyright('const char *s = "@copyright";'), false);
  strictEqual(comments.hasCopyright('/** @copyright */\n#pragma once'), true);
  const issues = headerIssues(files, comments);
  strictEqual(issues.find(item => item.path === 'include/jh/widget.h')?.missingCopyright, false);
  strictEqual(issues.find(item => item.path === 'include/jh/detail/internal.h')?.missingCopyright, true);
  strictEqual(issues.find(item => item.path === 'include/jh/group')?.missingCopyright, false);
  contents.set('include/jh/widget.h', Buffer.from('/** @copyright */\n\u00e9\n\n'));
  const invalid = headerIssues(files, comments).find(item => item.path === 'include/jh/widget.h');
  strictEqual(invalid?.nonAscii, true);
  strictEqual(invalid?.badNewline, true);
});
