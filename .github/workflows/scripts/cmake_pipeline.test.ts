import test from 'node:test';
import assert from 'node:assert/strict';
import { createCMakeLexer } from './cmake_lexer.ts';
import { createCMakeParser } from './cmake_parser.ts';
import { analyzeAst } from './cmake_analyzer.ts';
import { supportedPlatformsFromWorkflow } from './metadata_sources.ts';
import { loadMetadataConfig } from './metadata_config.ts';
import { serializeReport } from './toml_serializer.ts';
import { parse } from 'smol-toml';
import { compactInstallPaths } from './install_paths.ts';

test('CMake AST retains branches and excludes uncalled definitions', async () => {
  const parser = createCMakeParser(await createCMakeLexer());
  const ast = parser.parse(`
if(ENABLED)
  add_library(active INTERFACE)
else()
  add_library(other INTERFACE)
endif()
function(hidden)
  add_library(unpublished INTERFACE)
endfunction()
`, 'fixture/CMakeLists.txt');
  assert.equal(ast[0].kind, 'if');
  assert.equal(ast[1].kind, 'definition');
  assert.deepEqual(analyzeAst(ast).map(({ command, condition }) => [command.args[0], command.line, condition]), [
    ['active', 3, '(ENABLED)'],
    ['other', 5, 'NOT (ENABLED)'],
  ]);
});

test('CI runner versions map to lowercase operating system names', () => {
  const workflow = `jobs:
  linux-current:
    runs-on: ubuntu-latest
  linux-pinned:
    runs-on: ubuntu-24.04
  darwin:
    runs-on: macos-15
  windows:
    runs-on: windows-2022
`;
  assert.deepEqual(supportedPlatformsFromWorkflow(workflow), ['ubuntu', 'macos', 'windows']);
});

test('config supplies project prose and badge metadata without a version', () => {
  const config = loadMetadataConfig();
  assert.equal(config.components.length, 3);
  for (const component of config.components) {
    assert.equal(Object.hasOwn(component.badge, 'message'), false);
    assert.equal(typeof component.badge.label, 'string');
  }
  const custom = structuredClone(config);
  custom.project.description = 'Description from configuration';
  const report = serializeReport([{
    name: 'example', version: '1.2.3', major: 1, standard: '20', cmake: 'CMakeLists.txt',
    profiles: [], targets: [], dependencies: [], warnings: [],
  }], { repository: 'https://example.com/toolkit', download: 'git clone https://example.com/toolkit.git' }, ['ubuntu'], custom);
  assert.equal((parse(report).project as { description: string }).description, 'Description from configuration');
});

test('CMake-selected headers collapse only complete directories', () => {
  const all = [
    'include/jh/full/one.h', 'include/jh/full/two.h',
    'include/jh/partial/one.h', 'include/jh/partial/two.h',
  ];
  assert.deepEqual(compactInstallPaths(all, [all[0], all[1], all[2]], []), {
    files: ['include/jh/partial/one.h'],
    directories: ['include/jh/full/'],
  });
});
