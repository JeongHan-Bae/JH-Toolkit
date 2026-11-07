import { readFileSync, existsSync, mkdtempSync, rmSync } from 'node:fs';
import { resolve, dirname, join, relative, isAbsolute, sep } from 'node:path';
import { tmpdir } from 'node:os';
import { execFileSync } from 'node:child_process';
import type { CMakeAstPort } from './cmake_ast.ts';
import type { Evidence, Target, PackageDep, FetchDep, InstallProfile, Component, ScanCtx } from './report_types.ts';
import type { CMakeNode, CMakeCommand } from './cmake_ast.ts';
import type { MetadataConfig, ComponentConfig } from './metadata_config.ts';

export type AnalyzedCommand = { command: CMakeCommand; condition: string };

/** Interpret every branch symbolically while excluding uncalled definitions. */
export function analyzeAst(nodes: CMakeNode[], outer: string[] = []): AnalyzedCommand[] {
  const result: AnalyzedCommand[] = [];
  for (const node of nodes) {
    if (node.kind === 'command') {
      result.push({ command: node.command, condition: outer.join(' AND ') });
    } else if (node.kind === 'if') {
      const earlier: string[] = [];
      for (const branch of node.branches) {
        const predicate = earlier.length
          ? `NOT (${earlier.join(' OR ')}) AND (${branch.predicate})`
          : `(${branch.predicate})`;
        result.push(...analyzeAst(branch.body, [...outer, predicate]));
        earlier.push(branch.predicate);
      }
      if (node.elseBody)
        result.push(...analyzeAst(node.elseBody, [...outer, `NOT (${earlier.join(' OR ')})`]));
    } else if (node.kind === 'loop') {
      result.push(...analyzeAst(node.body, [...outer, `${node.name}(${node.args.join(' ')})`]));
    }
  }
  return result;
}

export function createMetadataAnalyzer(config: { root: string; astPort: CMakeAstPort; fetchExternal: boolean; scanCached: boolean; metadata: MetadataConfig }) {
  const { root, astPort, fetchExternal, scanCached, metadata } = config;
const COMPONENTS = metadata.components;

const externalSources: Array<{ root: string; name: string }> = [];
function rel(file: string): string {
  const found = externalSources.find(s => inside(s.root, file));
  if (found) return `external/${found.name}/${relative(found.root, file).split(sep).join('/')}`;
  return relative(root, file).split(sep).join('/') || '.';
}
function inside(base: string, p: string): boolean { const r = relative(base, p); return r === '' || (r !== '..' && !r.startsWith('..' + sep) && !isAbsolute(r)); }
function evidence(file: string, line: number, condition = ''): Evidence {
  return { file: rel(file), line, ...(condition ? { condition } : {}) };
}
class Scanner {
  component: Component;
  componentRoot: string;
  projectFound = false;
  targets = new Map<string, Target>();
  installedTargets = new Set<string>();
  packages = new Map<string, PackageDep>();
  fetches = new Map<string, FetchDep>();
  vars = new Map<string, string>();
  scanned = new Set<string>();
  active = new Set<string>();
  externalDirectories = new Set<string>();
  temps: string[] = [];
  warnings: string[] = [];
  entry: ComponentConfig;
  constructor(entry: ComponentConfig) {
    this.entry = entry;
    this.componentRoot = resolve(root, entry.dir);
    this.component = { name: '', version: '', major: 0, cmake: rel(join(this.componentRoot, 'CMakeLists.txt')), profiles: [], targets: [], dependencies: [], warnings: [] };
    this.vars.set('PROJECT_SOURCE_DIR', this.componentRoot);
    this.vars.set('CMAKE_SOURCE_DIR', root);
  }
  evidence(file: string, cmd: CMakeCommand, cond: string): Evidence { return evidence(file, cmd.line, cond); }
  warn(message: string) { if (!this.warnings.includes(message)) this.warnings.push(message); }
  replace(s: string, ctx: ScanCtx): string {
    let out = s;
    const vars = new Map(this.vars);
    vars.set('CMAKE_CURRENT_LIST_DIR', dirname(this.currentFile));
    vars.set('CMAKE_CURRENT_SOURCE_DIR', ctx.sourceDir);
    vars.set('PROJECT_SOURCE_DIR', ctx.projectDir);
    for (let i = 0; i < 4; i++) {
      const next = out.replace(/\$\{([A-Za-z_][A-Za-z0-9_]*)}/g, (full, key: string) => vars.has(key) ? vars.get(key)! : full);
      if (next === out) break;
      out = next;
    }
    return out;
  }
  currentFile = '';
  validFile(file: string): boolean { return inside(root, file) || [...this.externalDirectories].some(dir => inside(dir, file)); }
  scan(file: string, ctx: ScanCtx) {
    file = resolve(file);
    if (!existsSync(file) || !this.validFile(file)) return;
    const key = `${file}\0${ctx.extraConditions.join('&')}`;
    if (this.scanned.has(key) || this.active.has(file)) return;
    if (this.scanned.size >= 1500) throw new Error('CMake source scan limit exceeded (1500 files)');
    this.active.add(file); this.scanned.add(key);
    try {
      const cmds = analyzeAst(astPort.parse(readFileSync(file, 'utf8'), file));
      for (const { command: cmd, condition } of cmds) {
        this.currentFile = file;
        const cond = [...ctx.extraConditions, condition].filter(Boolean).join(' AND ');
        const ev = this.evidence(file, cmd, cond);
        const a = cmd.args.filter(x => x !== '(' && x !== ')');
        const value = (s: string) => this.replace(s, ctx);
        if (cmd.name === 'project' && file === join(this.componentRoot, 'CMakeLists.txt')) {
          if (a[0]) { this.component.name = value(a[0]); this.projectFound = true; }
          const i = a.findIndex(s => s.toUpperCase() === 'VERSION');
          if (i >= 0 && a[i + 1]) this.component.version = value(a[i + 1]);
        }
        if (cmd.name === 'set' && a[0] && a.length >= 2) {
          if (!cond && a[1] && !a.includes('PARENT_SCOPE')) this.vars.set(a[0], a.slice(1, a.indexOf('CACHE') > 0 ? a.indexOf('CACHE') : undefined).map(value).join(';'));
          if (file === join(this.componentRoot, 'CMakeLists.txt') && a[0] === 'PROJECT_VERSION' && !cond) this.component.version = value(a[1]);
          if (!ctx.external && a[0] === 'CMAKE_CXX_STANDARD' && !this.component.standard) this.component.standard = value(a[1]);
        }
        // A few safe, side-effect-free CMake variable operators needed by
        // upstream dependencies.cpm.cmake (e.g. Gherkin's pinned Messages tag).
        if (cmd.name === 'list' && a[0]?.toUpperCase() === 'GET' && a.length >= 4 && !cond) {
          const values = (this.vars.get(a[1]) ?? '').split(';');
          const index = Number(a[2]);
          if (Number.isInteger(index) && values[index] !== undefined) this.vars.set(a[3], values[index]);
        }
        if (cmd.name === 'string' && a[0]?.toUpperCase() === 'SUBSTRING' && a.length >= 5 && !cond) {
          const original = value(a[1]), begin = Number(a[2]), length = Number(a[3]);
          if (Number.isInteger(begin) && Number.isInteger(length) && begin >= 0)
            this.vars.set(a[4], length < 0 ? original.slice(begin) : original.slice(begin, begin + length));
        }
        if (cmd.name === 'add_library' || cmd.name === 'add_executable') {
          const name = a[0]; if (!name || name.includes('${') || name.includes('$<')) continue;
          const aliasI = a.indexOf('ALIAS');
          if (aliasI >= 0 && a[aliasI + 1]) {
            this.getTarget(a[aliasI + 1], 'UNKNOWN', ev, ctx.external).aliases.push(name);
          } else {
            const type = a.find(x => ['STATIC', 'SHARED', 'MODULE', 'INTERFACE', 'OBJECT', 'UNKNOWN', 'IMPORTED'].includes(x.toUpperCase()))?.toUpperCase()
              ?? (cmd.name === 'add_executable' ? 'EXECUTABLE' : 'DEFAULT');
            this.getTarget(name, type, ev, ctx.external);
          }
        }
        if (!ctx.external && cmd.name === 'install' && a[0]?.toUpperCase() === 'TARGETS') {
          for (const name of a.slice(1)) {
            if (['EXPORT', 'ARCHIVE', 'LIBRARY', 'RUNTIME', 'DESTINATION', 'COMPONENT', 'OPTIONAL', 'INCLUDES'].includes(name.toUpperCase())) break;
            if (!name.includes('$')) this.installedTargets.add(name);
          }
        }
        if (cmd.name === 'target_link_libraries' && a[0]) {
          const target = this.getTarget(a[0], 'UNKNOWN', ev, ctx.external);
          let scope = 'DEFAULT';
          for (const raw of a.slice(1)) {
            if (['PUBLIC', 'PRIVATE', 'INTERFACE', 'LINK_PUBLIC', 'LINK_PRIVATE', 'debug', 'optimized', 'general'].includes(raw)) { scope = raw; continue; }
            for (const item of this.expandLink(raw, ctx)) target.links.push({ name: item, scope, evidence: ev });
          }
        }
        if (cmd.name === 'target_compile_features' && a[0]) {
          const t = this.getTarget(a[0], 'UNKNOWN', ev, ctx.external);
          for (const f of a.slice(1)) {
            const standard = /^cxx_std_(\d+)$/.exec(f);
            if (standard) {
              t.standards.push(standard[1]);
              if (!ctx.external && !this.component.standard) this.component.standard = standard[1];
            }
          }
        }
        if (cmd.name === 'find_package' && a[0]) {
          const name = a[0];
          if (['Threads'].includes(name)) { /* keep as normal package below */ }
          const candidate = value(a[1] ?? '');
          const version = /^\d+(?:\.\d+)*(?:\.\.\.<\d+(?:\.\d+)*)?$/.test(candidate) ? candidate : undefined;
          this.packages.set(`${name}:${ev.file}:${ev.line}`, { name, ...(version ? { version } : {}), method: 'find_package', required: a.some(x => x.toUpperCase() === 'REQUIRED'), condition: cond, file: ev.file, line: ev.line });
        }
        if (cmd.name === 'fetchcontent_declare' && a[0]) {
          const depName = a[0]; const kv = this.kv(a.slice(1), ctx);
          this.fetches.set(depName.toLowerCase(), {
            name: depName, method: 'FetchContent', repository: kv.GIT_REPOSITORY ?? kv.URL,
            ref: kv.GIT_TAG, sourceSubdir: kv.SOURCE_SUBDIR,
            condition: cond, file: ev.file, line: ev.line, madeAvailable: false,
          });
        }
        if (cmd.name === 'fetchcontent_makeavailable') {
          for (const name of a) {
            const dep = this.fetches.get(name.toLowerCase());
            if (dep) {
              dep.madeAvailable = true;
              dep.condition = [dep.condition, cond].filter(Boolean).join(' AND ');
              if (fetchExternal) this.fetchAndScan(dep, ctx);
              else if (scanCached) this.scanCachedFetch(dep, ctx);
            } else this.warn(`${ev.file}:${ev.line}: FetchContent '${name}' has no visible Declare`);
          }
        }
        if (cmd.name === 'cpmaddpackage' || cmd.name === 'cpmfindpackage') {
          const kv = this.kv(a, ctx);
          const name = kv.NAME ?? kv.GITHUB_REPOSITORY?.split('/').at(-1) ?? kv.GIT_REPOSITORY?.split('/').at(-1)?.replace(/\.git$/, '') ?? /^gh:([^@]+)@/.exec(kv.URI ?? '')?.[1].split('/').at(-1);
          if (name) this.packages.set(`CPM:${name}:${ev.file}:${ev.line}`, {
            name, version: kv.VERSION ?? kv.GIT_TAG ?? /^gh:[^@]+@([^#]+)/.exec(kv.URI ?? '')?.[1], method: cmd.name === 'cpmaddpackage' ? 'CPMAddPackage' : 'CPMFindPackage', repository: kv.GIT_REPOSITORY ?? (kv.GITHUB_REPOSITORY ? `https://github.com/${kv.GITHUB_REPOSITORY}` : /^gh:([^@]+)@/.exec(kv.URI ?? '') ? `https://github.com/${/^gh:([^@]+)@/.exec(kv.URI ?? '')![1]}` : undefined),
            condition: cond, file: ev.file, line: ev.line
          });
        }
        if (cmd.name === 'add_subdirectory' && a[0]) {
          const p = value(a[0]);
          if (!p.includes('${') && !p.includes('$<')) {
            const target = resolve(ctx.sourceDir, p, 'CMakeLists.txt');
            const belongsToOtherComponent = !ctx.external && COMPONENTS.some(entry =>
              entry.dir !== '.' && entry.dir !== this.entry.dir && inside(resolve(root, entry.dir), target));
            const relativeDir = relative(this.componentRoot, dirname(target)).split(sep);
            const isNonReleaseTree = !ctx.external && relativeDir.some(part =>
              ['tests', 'tdd', 'static_tests', 'examples'].includes(part));
            if (existsSync(target) && !belongsToOtherComponent && !isNonReleaseTree)
              this.scan(target, { sourceDir: dirname(target), projectDir: ctx.projectDir, external: ctx.external, extraConditions: [...ctx.extraConditions, condition].filter(Boolean) });
          } else this.warn(`${ev.file}:${ev.line}: unresolved add_subdirectory(${a[0]})`);
        }
        if (cmd.name === 'include' && a[0] && !['GNUInstallDirs', 'CTest', 'CheckCXXCompilerFlag', 'CMakePackageConfigHelpers', 'FetchContent'].includes(a[0])) {
          const p = value(a[0]);
          if (!p.includes('${') && !p.includes('$<')) {
            const candidates = [resolve(dirname(file), p), resolve(ctx.sourceDir, p), resolve(this.componentRoot, p)];
            const found = candidates.flatMap(path => [path, path + '.cmake']).find(path => existsSync(path) && this.validFile(path));
            if (found) this.scan(found, { ...ctx, extraConditions: [...ctx.extraConditions, condition].filter(Boolean) });
          }
        }
      }
    } finally { this.active.delete(file); }
  }
  getTarget(name: string, type: string, ev: Evidence, external: boolean): Target {
    let t = this.targets.get(name);
    if (!t) {
      t = { name, type, aliases: [], links: [], standards: [], evidence: ev, external };
      this.targets.set(name, t);
    } else if (type !== 'UNKNOWN' && t.type === 'UNKNOWN') { t.type = type; t.evidence = ev; }
    return t;
  }
  kv(a: string[], ctx: ScanCtx): Record<string, string> {
    const out: Record<string, string> = {};
    const KEYS = new Set(['NAME','VERSION','GIT_REPOSITORY','GIT_TAG','GITHUB_REPOSITORY','URL','SOURCE_SUBDIR','SOURCE_DIR','URI','URL_HASH','DOWNLOAD_ONLY','OPTIONS','EXCLUDE_FROM_ALL','SYSTEM']);
    for (let i = 0; i < a.length - 1; i++) if (KEYS.has(a[i].toUpperCase())) out[a[i].toUpperCase()] = this.replace(a[++i], ctx);
    return out;
  }
  expandLink(raw: string, ctx: ScanCtx): string[] {
    let val = this.replace(raw, ctx);
    if (/^\$<(?:BUILD_INTERFACE|INSTALL_INTERFACE|LINK_ONLY):(.+)>$/.test(val)) val = val.replace(/^\$<[^:]+:(.*)>$/, '$1');
    if (/\$<|\$\{|^\$|^[-/]|^(?:\.|\d|::)/.test(val)) return [];
    return val.split(';').filter(s => s && s !== 'NOTFOUND');
  }
  scanCachedFetch(dep: FetchDep, ctx: ScanCtx): void {
    const name = dep.name.toLowerCase();
    const base = process.env[`FETCHCONTENT_SOURCE_DIR_${dep.name.toUpperCase()}`] || join(root, 'build', '_deps', `${name}-src`);
    if (existsSync(base) && inside(root, resolve(base))) {
      dep.source = resolve(base);
      this.externalDirectories.add(dep.source);
      externalSources.push({ root: dep.source, name: dep.name });
      const subdir = dep.sourceSubdir ?? '.';
      const cmake = resolve(dep.source, subdir, 'CMakeLists.txt');
      if (inside(dep.source, cmake)) this.scan(cmake, { sourceDir: dirname(cmake), projectDir: dirname(cmake), external: true, extraConditions: [...ctx.extraConditions, dep.condition || ''].filter(Boolean) });
    }
  }
  fetchAndScan(dep: FetchDep, ctx: ScanCtx): void {
    if (!dep.repository || !dep.ref || !/^https:\/\/github\.com\/[A-Za-z0-9_.-]+\/[A-Za-z0-9_.-]+(?:\.git)?$/.test(dep.repository)) {
      this.warn(`Cannot safely fetch ${dep.name}: missing supported GitHub HTTPS repository or pinned GIT_TAG`);
      return;
    }
    const tmp = mkdtempSync(join(tmpdir(), 'jh-cmake-'));
    this.temps.push(tmp);
    const dest = join(tmp, dep.name);
    const cmake = resolve(dest, dep.sourceSubdir ?? '.', 'CMakeLists.txt');
    if (!inside(dest, cmake)) throw new Error(`Invalid SOURCE_SUBDIR for ${dep.name}`);
    try {
      execFileSync('git', ['-c','protocol.file.allow=never', 'clone','--quiet','--depth','1','--branch',dep.ref, '--',dep.repository,dest],
        { stdio: 'pipe', timeout: 90000 });
    } catch (err) { throw new Error(`Fetch ${dep.name} failed: ${(err as Error).message.slice(0, 180)}`); }
    dep.source = dest;
    this.externalDirectories.add(dest);
    externalSources.push({ root: dest, name: dep.name });
    this.scan(cmake, { sourceDir: dirname(cmake), projectDir: dirname(cmake), external: true, extraConditions: [...ctx.extraConditions, dep.condition || ''].filter(Boolean) });
  }
  releaseProfiles(): InstallProfile[] {
    const file = join(root, 'cmake', 'jh-toolkit-components.cmake');
    const commands = analyzeAst(astPort.parse(readFileSync(file, 'utf8'), file)).map(item => item.command);
    const command = commands.find(cmd => cmd.name === 'set_property' &&
      cmd.args[0]?.toUpperCase() === 'CACHE' && cmd.args[1] === 'JH_TOOLKIT_COMPONENTS' &&
      cmd.args[2]?.toUpperCase() === 'PROPERTY' && cmd.args[3]?.toUpperCase() === 'STRINGS');
    if (!command) throw new Error(`Missing JH_TOOLKIT_COMPONENTS choices in ${file}`);
    return command.args.slice(4).filter(x => x !== '(' && x !== ')')
      .map(name => ({ name, files: [], directories: [] }));
  }
  build(): Component {
    const file = join(this.componentRoot, 'CMakeLists.txt');
    if (!existsSync(file)) throw new Error(`Required file missing: ${file}`);
    try {
      this.scan(file, { sourceDir: this.componentRoot, projectDir: this.componentRoot, external: false, extraConditions: [] });
      if (!this.projectFound) throw new Error(`Cannot find project() name in ${file}`);
      if (!/^\d+\.\d+(?:\.\d+)?(?:\.\d+)?$/.test(this.component.version))
        throw new Error(`Cannot find a valid version in ${file}; got '${this.component.version}'`);
      this.component.major = Number(this.component.version.split('.')[0]);
      this.component.targets = [...this.installedTargets].map(name => this.targets.get(name))
        .filter((t): t is Target => !!t && !t.external && t.type !== 'UNKNOWN')
        .sort((a,b) => a.name.localeCompare(b.name));
      if (this.component.targets.length !== this.installedTargets.size) throw new Error(`Unresolved installed target in ${file}`);
      if (this.entry.kind === 'toolkit') this.component.profiles = this.releaseProfiles();
      const deps: PackageDep[] = [];
      if (this.entry.kind !== 'toolkit') {
        const toolkitLink = this.component.targets.flatMap(target => target.links)
          .find(link => link.name === 'jh::jh-toolkit');
        if (!toolkitLink) throw new Error(`Installed targets in ${file} do not link jh::jh-toolkit`);
        deps.push({ name: 'jh-toolkit', method: 'linked_target',
          file: toolkitLink.evidence.file, line: toolkitLink.evidence.line,
          condition: toolkitLink.evidence.condition });
      }
      for (const pkg of this.packages.values()) {
        if (pkg.name.toLowerCase() !== 'jh-toolkit') deps.push(pkg);
      }
      for (const fetch of this.fetches.values()) if (fetch.madeAvailable) {
        deps.push({ name: fetch.name, method: fetch.method, repository: fetch.repository, ref: fetch.ref, sourceSubdir: fetch.sourceSubdir,
          condition: fetch.condition, file: fetch.file, line: fetch.line });
      }
      this.component.dependencies = deps.sort((a,b) => a.name.localeCompare(b.name) || a.method.localeCompare(b.method));
      this.component.warnings = this.warnings;
      return this.component;
    } finally { for (const dir of this.temps) rmSync(dir, { recursive: true, force: true }); }
  }
}


  return { components: COMPONENTS, analyze: () => COMPONENTS.map(entry => new Scanner(entry).build()), rel };
}
