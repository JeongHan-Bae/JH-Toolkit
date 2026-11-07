import type { CMakeAstPort, CMakeCommand, CMakeNode } from './cmake_ast.ts';
import type { CMakeLexerPort, SyntaxNode } from './cmake_lexer.ts';

function unescapeArgument(value: string): string {
  return value.replace(/\\([;"()#\\ ])/g, '$1').replace(/\\\n/g, '');
}

function argumentsOf(node: SyntaxNode): string[] {
  const list = node.namedChildren.find(child => child.type === 'argument_list');
  return list?.namedChildren.filter(child => child.type === 'argument').map(argument => {
    const value = argument.namedChildren[0];
    if (!value) return '';
    if (value.type === 'quoted_argument') return unescapeArgument(value.text.slice(1, -1));
    if (value.type === 'bracket_argument')
      return value.namedChildren.find(child => child.type === 'bracket_argument_content')?.text.replace(/^\n/, '') ?? '';
    return unescapeArgument(value.text);
  }) ?? [];
}

function commandOf(node: SyntaxNode): CMakeCommand {
  const name = node.type === 'normal_command'
    ? node.namedChildren.find(child => child.type === 'identifier')?.text.toLowerCase()
    : node.type.slice(0, -'_command'.length);
  if (!name) throw new Error(`CMake command name missing at line ${node.startPosition.row + 1}`);
  return { name, args: argumentsOf(node), line: node.startPosition.row + 1 };
}

function lower(node: SyntaxNode): CMakeNode[] {
  if (node.type === 'if_condition') {
    const branches: { predicate: string; body: CMakeNode[] }[] = [];
    let elseBody: CMakeNode[] | undefined;
    for (let index = 0; index < node.namedChildren.length; index++) {
      const child = node.namedChildren[index];
      if (child.type === 'if_command' || child.type === 'elseif_command') {
        const body = node.namedChildren[index + 1];
        branches.push({ predicate: argumentsOf(child).join(' '), body: body?.type === 'body' ? lower(body) : [] });
      } else if (child.type === 'else_command') {
        const body = node.namedChildren[index + 1];
        elseBody = body?.type === 'body' ? lower(body) : [];
      }
    }
    return [{ kind: 'if', branches, ...(elseBody ? { elseBody } : {}) }];
  }
  if (node.type === 'foreach_loop' || node.type === 'while_loop') {
    const name = node.type === 'foreach_loop' ? 'foreach' : 'while';
    const command = node.namedChildren.find(child => child.type === `${name}_command`);
    const body = node.namedChildren.find(child => child.type === 'body');
    return [{ kind: 'loop', name, args: command ? argumentsOf(command) : [], body: body ? lower(body) : [] }];
  }
  if (node.type === 'function_def' || node.type === 'macro_def') {
    const body = node.namedChildren.find(child => child.type === 'body');
    return [{ kind: 'definition', body: body ? lower(body) : [] }];
  }
  if (node.type === 'normal_command') return [{ kind: 'command', command: commandOf(node) }];
  return node.namedChildren.flatMap(lower);
}

/** Map the lexer tree into command and control-flow AST nodes. */
export function createCMakeParser(lexer: CMakeLexerPort): CMakeAstPort {
  return { parse(source, filename) { return lower(lexer.lex(source, filename)); } };
}
