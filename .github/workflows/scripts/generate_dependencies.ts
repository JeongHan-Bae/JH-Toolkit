#!/usr/bin/env node
/**
 * Generate JH-Toolkit release metadata and version badges with Node.js 24 or newer.
 * Install this directory's package.json dependencies with npm ci before running.
 * Uses Tree-sitter for CMake source, the CMake File API to verify generated build
 * metadata, and yaml for the CI workflow. Requires CMake 3.21 or newer.
 *
 * Usage:
 *   node .github/workflows/scripts/generate_dependencies.ts [--root PATH]
 *   node .github/workflows/scripts/generate_dependencies.ts --check
 *   node .github/workflows/scripts/generate_dependencies.ts --fetch-external
 *   node .github/workflows/scripts/generate_dependencies.ts --scan-cached
 *   node .github/workflows/scripts/generate_dependencies.ts --get-version
 *
 * CMake source scanning records conditional declarations. File API validation
 * configures the all-components profile with tests and examples disabled.
 */
import { readFileSync, writeFileSync, existsSync, mkdirSync } from 'node:fs';
import { resolve, dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { createCMakeLexer } from './cmake_lexer.ts';
import { createCMakeParser } from './cmake_parser.ts';
import { createMetadataAnalyzer } from './cmake_analyzer.ts';
import { serializeReport } from './toml_serializer.ts';
import { populateInstallProfiles } from './cmake_file_api.ts';
import { repositorySource, ciSupportedPlatforms } from './metadata_sources.ts';
import { loadMetadataConfig } from './metadata_config.ts';

const scriptRoot = resolve(dirname(fileURLToPath(import.meta.url)), '../../..');
const args = process.argv.slice(2);
const readArg = (key: string): string | undefined => { const i = args.indexOf(key); return i < 0 ? undefined : args[i + 1]; };
const root = resolve(readArg('--root') ?? scriptRoot);
const check = args.includes('--check');
const fetchExternal = args.includes('--fetch-external');
const scanCached = args.includes('--scan-cached');
const onlyVersion = args.includes('--get-version');
const metadata = loadMetadataConfig();
const astPort = createCMakeParser(await createCMakeLexer());
const analyzer = createMetadataAnalyzer({ root, astPort, fetchExternal, scanCached, metadata });
if (Number(process.versions.node.split('.')[0]) < 24) {
  throw new Error('Use Node.js 24 or newer to run this script.');
}

function syncFile(p: string, data: string) {
  const old = existsSync(p) ? readFileSync(p,'utf8') : '';
  if (old === data) return;
  if (check) { console.error(`OUT OF DATE: ${analyzer.rel(p)}`); process.exitCode = 1; return; }
  mkdirSync(dirname(p), { recursive: true }); writeFileSync(p, data); console.log(`Updated ${analyzer.rel(p)}`);
}
async function main() {
  const components = analyzer.analyze();
  if (onlyVersion) { console.log(components[0].version); return; }
  await populateInstallProfiles(root, components[0]);
  const target = join(root,'dependencies.toml');
  syncFile(target, serializeReport(components, repositorySource(root), ciSupportedPlatforms(root), metadata));
  analyzer.components.forEach((entry, i) => {
    const { file, schemaVersion, label, ...badgeFields } = entry.badge;
    const dest = join(root, 'assets', 'metadata', file);
    syncFile(dest, JSON.stringify({ schemaVersion, label, message: components[i].version, ...badgeFields }, null, 2) + '\n');
  });
  for (const c of components) {
    console.log(`${c.name} ${c.version}: ${c.targets.length} installed targets, ${c.dependencies.length} dependencies`);
    for (const warning of c.warnings) console.warn(`  WARNING: ${warning}`);
  }
}
await main();
