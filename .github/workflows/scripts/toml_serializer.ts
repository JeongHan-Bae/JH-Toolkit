import { stringify } from 'smol-toml';
import type { Component } from './report_types.ts';
import type { MetadataConfig } from './metadata_config.ts';

function uniq<T>(values: T[]): T[] { return [...new Set(values)]; }

function componentReport(component: Component) {
  return {
    name: component.name,
    version: component.version,
    major_version: component.major,
    cmake: component.cmake,
    ...(component.profiles.length ? { selectable: component.profiles.map(profile => ({
      name: profile.name,
      ...(profile.files.length ? { files: profile.files } : {}),
      ...(profile.directories.length ? { directories: profile.directories } : {}),
    })) } : {}),
    targets: component.targets.map(target => ({
      name: target.name, type: target.type,
      ...(target.aliases.length ? { aliases: uniq(target.aliases).sort() } : {}),
      ...(target.standards.length ? { standards: uniq(target.standards).sort() } : {}),
      file: target.evidence.file, line: target.evidence.line,
      ...(target.evidence.condition ? { condition: target.evidence.condition } : {}),
      ...(target.links.length ? { links: target.links.map(link => ({
        name: link.name, scope: link.scope, file: link.evidence.file, line: link.evidence.line,
        ...(link.evidence.condition ? { condition: link.evidence.condition } : {}),
      })) } : {}),
    })),
    ...(component.dependencies.length ? { dependencies: component.dependencies.map(dep => ({
      name: dep.name, method: dep.method,
      ...(dep.version ? { version: dep.version } : {}),
      ...(dep.required !== undefined ? { required: dep.required } : {}),
      ...(dep.repository ? { repository: dep.repository } : {}),
      ...(dep.ref ? { ref: dep.ref } : {}),
      ...(dep.sourceSubdir ? { source_subdir: dep.sourceSubdir } : {}),
      ...(dep.condition ? { condition: dep.condition } : {}),
      file: dep.file, line: dep.line,
    })) } : {}),
  };
}

export function serializeReport(components: Component[], sourceInfo: { repository: string; download: string }, supportedPlatforms: string[], metadata: MetadataConfig): string {
  const project = components[0];
  if (!project.standard) throw new Error('Cannot find CMAKE_CXX_STANDARD in top-level CMakeLists.txt');
  return stringify({
    project: {
      name: project.name,
      version: project.version,
      cxx_standard: project.standard,
      description: metadata.project.description,
      supported_platforms: supportedPlatforms,
      source: sourceInfo,
    },
    components: components.map(componentReport),
  }).trimEnd() + '\n';
}
