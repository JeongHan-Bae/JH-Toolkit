import { mkdtempSync, rmSync, readFileSync } from 'node:fs';
import { join, relative, sep } from 'node:path';
import { tmpdir } from 'node:os';
import { execFileSync } from 'node:child_process';
import { createSharedStatelessQuery, findCurrentReplyIndexPath, readReplyIndex, readCache, readCodemodel } from 'cmake-file-api';
import type { Component } from './report_types.ts';
import { compactInstallPaths } from './install_paths.ts';

export async function populateInstallProfiles(root: string, project: Component): Promise<void> {
  const expectedChoices = project.profiles.map(profile => profile.name);
  for (const profile of project.profiles) {
    const build = mkdtempSync(join(tmpdir(), 'jh-metadata-file-api-'));
    try {
      await createSharedStatelessQuery(build, 'codemodel', '2');
      await createSharedStatelessQuery(build, 'cache', '2');
      const trace = join(build, 'cmake-trace.json');
      execFileSync('cmake', [
        '-S', root, '-B', build, '-DCMAKE_BUILD_TYPE=Release',
        '-DJH_ENABLE_TESTING=OFF', '-DJH_ENABLE_EXAMPLES=OFF',
        `-DJH_TOOLKIT_COMPONENTS=${profile.name}`, '-DUSE_JH_COMPILE_OPT=OFF',
        '--trace-format=json-v1', '--trace-expand',
        `--trace-source=${join(root, 'cmake', 'jh-toolkit-components.cmake')}`,
        `--trace-redirect=${trace}`,
      ], { stdio: 'pipe', timeout: 120000 });
      const replyDir = join(build, '.cmake', 'api', 'v1', 'reply');
      const index = await readReplyIndex(await findCurrentReplyIndexPath(replyDir));
      const reply = (kind: string) => {
        const entry = index.objects.find(object => object.kind === kind);
        if (!entry) throw new Error(`CMake File API has no ${kind} reply`);
        return join(replyDir, entry.jsonFile);
      };
      const model = await readCodemodel(reply('codemodel'));
      const configuration = model.configurations[0];
      if (configuration?.projects[0]?.name !== project.name)
        throw new Error('CMake File API project name differs from parsed CMake source');
      if (profile.name === 'all') {
        const concreteTargets = project.targets.filter(target => target.type !== 'INTERFACE').map(target => target.name);
        const modeledTargets = new Set(configuration.targets?.map(target => target.name) ?? []);
        if (concreteTargets.some(name => !modeledTargets.has(name)))
          throw new Error('CMake File API is missing an installed concrete target');
      }
      const cache = await readCache(reply('cache'), '2.0');
      const choice = cache.entries.find(entry => entry.name === 'JH_TOOLKIT_COMPONENTS');
      const allowed = choice?.properties.find(property => property.name === 'STRINGS')?.value.split(';');
      if (JSON.stringify(allowed) !== JSON.stringify(expectedChoices))
        throw new Error('CMake File API component choices differ from parsed CMake source');
      const files = new Set<string>();
      const directories = new Set<string>();
      const records = readFileSync(trace, 'utf8').split(/\r?\n/).filter(Boolean)
        .map(line => JSON.parse(line) as { cmd?: string; args?: string[] });
      const sourceHeaders = records.find(record =>
        record.cmd === 'set' && record.args?.[0] === 'JH_TOOLKIT_ALL_HEADERS')?.args?.[1];
      if (!sourceHeaders) throw new Error('CMake trace has no JH_TOOLKIT_ALL_HEADERS list');
      const sourceFiles = sourceHeaders.split(';').filter(Boolean).map(path => `include/${path}`);
      for (const record of records) {
        if (record.cmd !== 'install' || !record.args || !['FILES', 'DIRECTORY'].includes(record.args[0])) continue;
        const end = record.args.indexOf('DESTINATION');
        if (end < 0) continue;
        for (const source of record.args.slice(1, end)) {
          const path = relative(root, source).split(sep).join('/');
          if (record.args[0] === 'DIRECTORY' && (path === 'include' || path.startsWith('include/')))
            directories.add(path.endsWith('/') ? path : path + '/');
          else if (record.args[0] === 'FILES' && path.startsWith('include/'))
            files.add(path);
        }
      }
      if (files.size + directories.size === 0)
        throw new Error(`CMake install trace found no installable headers for '${profile.name}'`);
      const compact = compactInstallPaths(sourceFiles, [...files], [...directories]);
      profile.files = compact.files;
      profile.directories = compact.directories;
    } finally { rmSync(build, { recursive: true, force: true }); }
  }
}
