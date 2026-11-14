import { spawnSync } from 'node:child_process';
import { mkdtempSync, mkdirSync, readFileSync, rmSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join, relative, resolve } from 'node:path';
import { pathToFileURL } from 'node:url';
import { renderDoxyfile } from './doxyfile.ts';
import { unescapeHeaders } from './unescape.ts';

export interface UndocumentedDeclaration {
  path: string;
  line: number;
  kind: 'type' | 'function';
  description: string;
}

export function undocumentedTypesAndFunctions(output: string, sourceRoot: string): UndocumentedDeclaration[] {
  const issues: UndocumentedDeclaration[] = [];
  for (const warning of output.split(/\r?\n/)) {
    const match = warning.match(/^(.*?):(\d+): warning: ((?:Compound|Member) .+ is not documented\.)$/);
    if (!match) continue;
    const kind = match[3].startsWith('Compound ') ? 'type'
      : /^Member .+ \((?:function|friend)\) of .+ is not documented\.$/.test(match[3]) ? 'function' : undefined;
    if (!kind) continue;
    const path = resolve(match[1]);
    if (!path.startsWith(sourceRoot + '/')) continue;
    issues.push({ path: relative(sourceRoot, path).split('\\').join('/'), line: Number(match[2]), kind, description: match[3] });
  }
  return issues;
}

/** Run the production Doxygen configuration without generating a site or changing .doxy-build. */
export function checkDoxygenDocumentation(root: string): UndocumentedDeclaration[] {
  const buildRoot = mkdtempSync(join(tmpdir(), 'jh-doxygen-check-'));
  try {
    unescapeHeaders(root, buildRoot);
    const pages = join(buildRoot, 'pages');
    mkdirSync(pages);
    for (const name of ['mainpage.md', 'toolkit_library.md', 'cucumber_library.md', 'tiny_test_library.md']) {
      writeFileSync(join(pages, name), `# ${name}\n`);
    }
    writeFileSync(join(buildRoot, 'footer.html'), '');
    const template = readFileSync(join(root, '.github/workflows/doxy_scripts/settings/Doxyfile'), 'utf8');
    const sourceRoot = join(buildRoot, 'source');
    const sourceRoots = [
      'include/jh', 'simple-cucumber/include/jh/test/cucumber',
      'simple-cucumber/gherkin/include/jh/test/cucumber', 'tiny-test/include/jh/test/tiny_test',
    ].map(path => join(sourceRoot, path)).join(' ');
    const doxyfile = renderDoxyfile(template.replaceAll('.doxy-build', buildRoot), {
      version: 'check', description: 'Documentation health check', html: 'NO', xml: 'NO', sourceRoots,
    });
    const config = join(buildRoot, 'Doxyfile');
    writeFileSync(config, doxyfile);
    const result = spawnSync('doxygen', [config], { cwd: root, encoding: 'utf8', maxBuffer: 32 * 1024 * 1024 });
    if (result.error) throw result.error;
    if (result.status !== 0) throw new Error(`Doxygen failed (${result.status}): ${result.stderr}`);
    return undocumentedTypesAndFunctions(result.stderr, sourceRoot);
  } finally {
    rmSync(buildRoot, { recursive: true, force: true });
  }
}

if (process.argv[1] && import.meta.url === pathToFileURL(resolve(process.argv[1])).href) {
  process.stdout.write(JSON.stringify(checkDoxygenDocumentation(resolve(process.cwd()))) + '\n');
}
