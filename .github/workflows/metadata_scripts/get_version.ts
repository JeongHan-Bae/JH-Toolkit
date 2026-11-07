#!/usr/bin/env node
/**
 * Read library versions from their CMake sources through the metadata analyzer.
 * Usage: node .github/workflows/metadata_scripts/get_version.ts [--all] [--root PATH]
 * The default output is the toolkit version; --all writes a JSON object for all libraries.
 */
import { resolve, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
import { createCMakeLexer } from './cmake_lexer.ts';
import { createCMakeParser } from './cmake_parser.ts';
import { createMetadataAnalyzer } from './cmake_analyzer.ts';
import { loadMetadataConfig } from './metadata_config.ts';

if (Number(process.versions.node.split('.')[0]) < 24)
  throw new Error('Use Node.js 24 or newer to run this script.');
const args = process.argv.slice(2);
const rootIndex = args.indexOf('--root');
if (rootIndex >= 0 && !args[rootIndex + 1]) throw new Error('--root requires a path');
const root = resolve(rootIndex >= 0 ? args[rootIndex + 1] : resolve(dirname(fileURLToPath(import.meta.url)), '../../..'));
const astPort = createCMakeParser(await createCMakeLexer());
const components = createMetadataAnalyzer({
  root, astPort, fetchExternal: false, scanCached: false, metadata: loadMetadataConfig(),
}).analyze();
if (args.includes('--all')) console.log(JSON.stringify(Object.fromEntries(components.map(({ name, version }) => [name, version]))));
else console.log(components[0].version);
