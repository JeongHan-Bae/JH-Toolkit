export type Evidence = { file: string; line: number; condition?: string };
export type Target = {
  name: string;
  type: string;
  aliases: string[];
  links: { name: string; scope: string; evidence: Evidence }[];
  standards: string[];
  evidence: Evidence;
  external: boolean;
};
export type PackageDep = {
  name: string;
  version?: string;
  required?: boolean;
  method: string;
  repository?: string;
  ref?: string;
  sourceSubdir?: string;
  condition?: string;
  file: string;
  line: number;
};
export type FetchDep = PackageDep & { madeAvailable: boolean; source?: string };
export type InstallProfile = { name: string; files: string[]; directories: string[] };
export type Component = {
  name: string;
  version: string;
  major: number;
  standard?: string;
  cmake: string;
  profiles: InstallProfile[];
  targets: Target[];
  dependencies: PackageDep[];
  warnings: string[];
};
export type ScanCtx = { sourceDir: string; projectDir: string; external: boolean; extraConditions: string[] };
