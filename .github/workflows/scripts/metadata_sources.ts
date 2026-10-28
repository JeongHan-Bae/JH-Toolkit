import { readFileSync, existsSync } from 'node:fs';
import { join } from 'node:path';
import { execFileSync } from 'node:child_process';
import { parseDocument } from 'yaml';

export function repositorySource(root: string): { repository: string; download: string } {
  const supplied = process.env.JH_REPOSITORY_URL?.trim();
  if (!supplied && !existsSync(join(root, '.git')))
    throw new Error('No .git entry in the project root; set JH_REPOSITORY_URL');
  let remote: string;
  try {
    remote = supplied || execFileSync('git', ['-C', root, 'remote', 'get-url', 'origin'],
      { encoding: 'utf8', stdio: ['ignore', 'pipe', 'pipe'] }).trim();
  } catch {
    throw new Error('Cannot read origin remote; set JH_REPOSITORY_URL when running outside a Git checkout');
  }
  const ssh = /^(?:ssh:\/\/)?git@([^:\/]+)[:\/](\S+)$/.exec(remote);
  if (ssh) remote = `https://${ssh[1]}/${ssh[2]}`;
  const url = new URL(remote);
  if (!['https:', 'http:'].includes(url.protocol) || url.username || url.password || url.search || url.hash)
    throw new Error('Repository URL must be an HTTP(S) URL without credentials or query parameters');
  const repository = `${url.origin}${url.pathname.replace(/(?:\.git)?\/?$/, '')}`;
  if (repository === url.origin) throw new Error('Repository URL has no path');
  return { repository, download: `git clone ${repository}.git` };
}

export function ciSupportedPlatforms(root: string): string[] {
  return supportedPlatformsFromWorkflow(readFileSync(join(root, '.github', 'workflows', 'ci.yml'), 'utf8'));
}

export function supportedPlatformsFromWorkflow(source: string): string[] {
  const workflow = parseDocument(source, { uniqueKeys: true });
  if (workflow.errors.length) throw new Error(`Invalid CI workflow YAML: ${workflow.errors[0].message}`);
  const jobs = workflow.toJS()?.jobs;
  if (!jobs || typeof jobs !== 'object' || Array.isArray(jobs))
    throw new Error('CI workflow has no jobs mapping');
  const platforms: string[] = [];
  for (const [name, job] of Object.entries(jobs)) {
    const runner = (job as { 'runs-on'?: unknown })?.['runs-on'];
    if (typeof runner !== 'string') throw new Error(`CI job '${name}' has no literal runs-on runner`);
    const [platform, version] = runner.split('-');
    if (!version || !['ubuntu', 'macos', 'windows'].includes(platform))
      throw new Error(`Unsupported CI runner '${runner}' in .github/workflows/ci.yml`);
    if (!platforms.includes(platform)) platforms.push(platform);
  }
  if (platforms.length === 0) throw new Error('CI workflow has no supported runner jobs');
  return platforms;
}
