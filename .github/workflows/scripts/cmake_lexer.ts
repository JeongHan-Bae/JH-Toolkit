import { Parser, Language } from 'web-tree-sitter';
import { getWasmPath } from 'tree-sitter-wasm';

export type SyntaxNode = {
  type: string;
  text: string;
  namedChildren: SyntaxNode[];
  startPosition: { row: number };
  hasError: boolean;
};

export type CMakeLexerPort = { lex(source: string, filename: string): SyntaxNode };

/** Initialize the npm WASM grammar and expose its syntax tree. */
export async function createCMakeLexer(): Promise<CMakeLexerPort> {
  await Parser.init();
  const parser = new Parser();
  parser.setLanguage(await Language.load(getWasmPath('cmake')));
  return {
    lex(source, filename) {
      const tree = parser.parse(source);
      if (!tree || tree.rootNode.hasError) throw new Error(`Invalid CMake syntax in ${filename}`);
      return tree.rootNode as SyntaxNode;
    },
  };
}
