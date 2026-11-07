export type CMakeCommand = { name: string; args: string[]; line: number };
export type CMakeNode =
  | { kind: 'command'; command: CMakeCommand }
  | { kind: 'if'; branches: { predicate: string; body: CMakeNode[] }[]; elseBody?: CMakeNode[] }
  | { kind: 'loop'; name: 'foreach' | 'while'; args: string[]; body: CMakeNode[] }
  | { kind: 'definition'; body: CMakeNode[] };

export type CMakeAstPort = { parse(source: string, filename: string): CMakeNode[] };
