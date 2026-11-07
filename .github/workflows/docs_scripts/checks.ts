import { posix } from 'node:path';
import type { CommentPort, FilePort, HeaderIssue } from './ports.ts';

export function missingDocumentation(files: FilePort): string[] {
  return files.publicHeaders().filter(path => {
    if (!path.endsWith('.h')) return false;
    if (path.startsWith('include/jh/detail/')) return false;
    if (path === 'include/jh/macros/header_begin.h' || path === 'include/jh/macros/header_end.h') return false;
    const doc = posix.join('docs', path.slice('include/jh/'.length).replace(/\.h$/, '.md'));
    return !files.exists(doc);
  });
}

export function headerIssues(files: FilePort, comments: CommentPort): HeaderIssue[] {
  return files.publicHeaders().map(path => {
    const data = files.read(path);
    const isHeader = path.endsWith('.h');
    return {
      path,
      nonAscii: data.some(byte => byte > 127),
      badNewline: data.length === 0 || data.at(-1) !== 10 || (data.length > 1 && data.at(-2) === 10),
      missingCopyright: isHeader && !comments.hasCopyright(data.toString('utf8')),
    };
  });
}
