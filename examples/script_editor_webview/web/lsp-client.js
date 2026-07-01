(function () {
  const LSP_CLIENT_VERSION = '1.0.0';

  function fromMonacoPosition(pos) {
    return { line: pos.lineNumber - 1, character: pos.column - 1 };
  }

  function toMonacoRange(range) {
    return {
      startLineNumber: range.start.line + 1,
      startColumn: range.start.character + 1,
      endLineNumber: range.end.line + 1,
      endColumn: range.end.character + 1,
    };
  }

  function severityToMonaco(monaco, severity) {
    switch (severity) {
      case 1: return monaco.MarkerSeverity.Error;
      case 2: return monaco.MarkerSeverity.Warning;
      case 3: return monaco.MarkerSeverity.Info;
      case 4: return monaco.MarkerSeverity.Hint;
      default: return monaco.MarkerSeverity.Info;
    }
  }

  function lspCompletionKindToMonaco(monaco, lspKind) {
    switch (lspKind) {
      case 2: return monaco.languages.CompletionItemKind.Method;
      case 3: return monaco.languages.CompletionItemKind.Function;
      case 4: return monaco.languages.CompletionItemKind.Constructor;
      case 5: return monaco.languages.CompletionItemKind.Field;
      case 6: return monaco.languages.CompletionItemKind.Variable;
      case 7: return monaco.languages.CompletionItemKind.Class;
      case 8: return monaco.languages.CompletionItemKind.Interface;
      case 9: return monaco.languages.CompletionItemKind.Module;
      case 10: return monaco.languages.CompletionItemKind.Property;
      case 11: return monaco.languages.CompletionItemKind.Unit;
      case 12: return monaco.languages.CompletionItemKind.Value;
      case 13: return monaco.languages.CompletionItemKind.Enum;
      case 14: return monaco.languages.CompletionItemKind.Keyword;
      case 15: return monaco.languages.CompletionItemKind.Snippet;
      case 16: return monaco.languages.CompletionItemKind.Color;
      case 17: return monaco.languages.CompletionItemKind.File;
      case 18: return monaco.languages.CompletionItemKind.Reference;
      case 19: return monaco.languages.CompletionItemKind.Folder;
      case 20: return monaco.languages.CompletionItemKind.EnumMember;
      case 21: return monaco.languages.CompletionItemKind.Constant;
      case 22: return monaco.languages.CompletionItemKind.Struct;
      case 23: return monaco.languages.CompletionItemKind.Event;
      case 24: return monaco.languages.CompletionItemKind.Operator;
      case 25: return monaco.languages.CompletionItemKind.TypeParameter;
      default: return monaco.languages.CompletionItemKind.Text;
    }
  }

  function ensureLuauLanguage(monaco) {
    const exists = monaco.languages.getLanguages().some(function (x) { return x.id === 'luau'; });
    if (!exists) {
      monaco.languages.register({ id: 'luau' });
    }

    monaco.languages.setMonarchTokensProvider('luau', {
      defaultToken: '',
      tokenPostfix: '.luau',
      keywords: [
        'and', 'break', 'continue', 'do', 'else', 'elseif', 'end', 'export', 'false', 'for', 'function', 'if', 'in',
        'local', 'nil', 'not', 'or', 'repeat', 'return', 'then', 'true', 'type', 'typeof', 'until', 'while'
      ],
      typeKeywords: ['any', 'boolean', 'buffer', 'never', 'number', 'string', 'thread', 'unknown', 'vector'],
      builtinGlobals: [
        'game', 'workspace', 'script', 'plugin', 'Enum', 'Instance', 'math', 'string', 'table', 'task', 'coroutine', 'utf8'
      ],
      operators: [
        '+', '-', '*', '/', '%', '^', '#', '==', '~=', '<=', '>=', '<', '>', '=', '+=', '-=', '*=', '/=', '..', '::', ':'
      ],
      symbols: /[=><!~?:&|+\-*\/\^%#.]+/,
      escapes: /\\(?:[abfnrtv\\\"']|x[0-9A-Fa-f]{2}|z\s*|u\{[0-9A-Fa-f]+\})/,
      tokenizer: {
        root: [
          [/[a-zA-Z_][\w]*/, {
            cases: {
              '@keywords': 'keyword',
              '@typeKeywords': 'type.identifier',
              '@builtinGlobals': 'variable.predefined',
              '@default': 'identifier'
            }
          }],
          { include: '@whitespace' },
          [/\d*\.\d+([eE][\-+]?\d+)?/, 'number.float'],
          [/0[xX][0-9a-fA-F_]+/, 'number.hex'],
          [/\d+/, 'number'],
          [/[{}\[\]()]/, '@brackets'],
          [/@symbols/, { cases: { '@operators': 'operator', '@default': '' } }],
          [/'/, { token: 'string.quote', bracket: '@open', next: '@stringSingle' }],
          [/\"/, { token: 'string.quote', bracket: '@open', next: '@stringDouble' }],
          [/`/, { token: 'string.quote', bracket: '@open', next: '@stringBacktick' }],
        ],
        whitespace: [
          [/\s+/, 'white'],
          [/--\[(=*)\[/, 'comment', '@comment.$1'],
          [/--.*$/, 'comment'],
        ],
        comment: [
          [/[^\]]+/, 'comment'],
          [/\](=*)\]/, { cases: { '$1==$S2': { token: 'comment', next: '@pop' }, '@default': 'comment' } }],
          [/./, 'comment']
        ],
        stringSingle: [
          [/[^\\']+/, 'string'],
          [/@escapes/, 'string.escape'],
          [/\\./, 'string.escape.invalid'],
          [/'/, { token: 'string.quote', bracket: '@close', next: '@pop' }]
        ],
        stringDouble: [
          [/[^\\\"]+/, 'string'],
          [/@escapes/, 'string.escape'],
          [/\\./, 'string.escape.invalid'],
          [/\"/, { token: 'string.quote', bracket: '@close', next: '@pop' }]
        ],
        stringBacktick: [
          [/[^\\`{]+/, 'string'],
          [/\{/, { token: 'delimiter.bracket', next: '@interpolation' }],
          [/@escapes/, 'string.escape'],
          [/\\./, 'string.escape.invalid'],
          [/`/, { token: 'string.quote', bracket: '@close', next: '@pop' }]
        ],
        interpolation: [
          [/\}/, { token: 'delimiter.bracket', next: '@stringBacktick' }],
          { include: '@root' }
        ]
      }
    });
  }

  function start(monaco, editor, options) {
    ensureLuauLanguage(monaco);

    const model = editor.getModel();
    if (model) {
      monaco.editor.setModelLanguage(model, 'luau');
    }

    const sendToServer = options.sendToServer;
    const rawUri = model && model.uri && model.uri.toString ? model.uri.toString() : '';
    const uri = rawUri.startsWith('file:///') ? rawUri : 'file:///rml/main.luau';

    let nextId = 1;
    let initialized = false;
    const pending = new Map();

    console.info('[RML LSP]', LSP_CLIENT_VERSION, 'starting over WebView host transport');

    function notify(method, params) {
      sendToServer({ jsonrpc: '2.0', method: method, params: params });
    }

    function request(method, params) {
      const id = nextId++;
      sendToServer({ jsonrpc: '2.0', id: id, method: method, params: params });
      return new Promise(function (resolve, reject) {
        pending.set(id, { resolve: resolve, reject: reject });
        setTimeout(function () {
          if (pending.has(id)) {
            pending.delete(id);
            reject(new Error('LSP timeout: ' + method));
          }
        }, 10000);
      });
    }

    function robloxSettings() {
      return {
        platform: { type: 'roblox' },
        sourcemap: { enabled: false, autogenerate: false },
        types: { roblox: true }
      };
    }

    async function handshake() {
      try {
        const init = await request('initialize', {
          processId: null,
          clientInfo: { name: 'RML Monaco', version: LSP_CLIENT_VERSION },
          rootUri: 'file:///rml',
          workspaceFolders: [{ uri: 'file:///rml', name: 'rml' }],
          initializationOptions: robloxSettings(),
          capabilities: {
            workspace: { configuration: false, workspaceFolders: true },
            textDocument: {
              synchronization: { didSave: true, dynamicRegistration: false },
              completion: { completionItem: { snippetSupport: true } },
              hover: { dynamicRegistration: false },
              definition: { dynamicRegistration: false }
            }
          }
        });

        notify('initialized', {});
        notify('workspace/didChangeConfiguration', { settings: robloxSettings() });

        notify('textDocument/didOpen', {
          textDocument: { uri: uri, languageId: 'luau', version: model.getVersionId(), text: model.getValue() }
        });

        initialized = true;
        console.info('[RML LSP] initialized', init && init.capabilities ? init.capabilities : {});
      } catch (err) {
        console.error('[RML LSP] initialize failed', err);
      }
    }

    function onServerMessage(msg) {
      if (!msg) return;

      if (typeof msg.id !== 'undefined' && (msg.result !== undefined || msg.error !== undefined)) {
        const p = pending.get(msg.id);
        if (p) {
          pending.delete(msg.id);
          if (msg.error) p.reject(msg.error); else p.resolve(msg.result);
        }
        return;
      }

      if (msg.method === 'textDocument/publishDiagnostics' && msg.params && msg.params.uri === uri) {
        const markers = (msg.params.diagnostics || []).map(function (d) {
          return Object.assign({}, toMonacoRange(d.range), {
            message: d.message || '',
            severity: severityToMonaco(monaco, d.severity),
            source: d.source || 'luau-lsp'
          });
        });
        monaco.editor.setModelMarkers(model, 'luau-lsp', markers);
      }
    }

    model.onDidChangeContent(function () {
      if (!initialized) return;
      notify('textDocument/didChange', {
        textDocument: { uri: uri, version: model.getVersionId() },
        contentChanges: [{ text: model.getValue() }],
      });
    });

    function registerCompletion(langId) {
      monaco.languages.registerCompletionItemProvider(langId, {
        triggerCharacters: ['.', ':'],
        provideCompletionItems: async function (_model, position, context) {
          if (!initialized) return { suggestions: [] };
          try {
            const triggerKind = context && context.triggerKind === monaco.languages.CompletionTriggerKind.TriggerCharacter ? 2 : 1;
            const triggerCharacter = context && context.triggerCharacter ? context.triggerCharacter : undefined;
            const result = await request('textDocument/completion', {
              textDocument: { uri: uri },
              position: fromMonacoPosition(position),
              context: { triggerKind: triggerKind, triggerCharacter: triggerCharacter },
            });

            const items = Array.isArray(result) ? result : (result && result.items) ? result.items : [];
            const suggestions = items.map(function (item) {
              let insertText = item.insertText || item.label;
              let range;
              if (item.textEdit && item.textEdit.newText) {
                insertText = item.textEdit.newText;
                if (item.textEdit.range) range = toMonacoRange(item.textEdit.range);
              }

              const isSnippet = item.insertTextFormat === 2;
              return {
                label: item.label,
                kind: lspCompletionKindToMonaco(monaco, item.kind),
                insertText: insertText,
                insertTextRules: isSnippet ? monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet : monaco.languages.CompletionItemInsertTextRule.None,
                range: range,
                filterText: item.filterText,
                sortText: item.sortText,
                preselect: item.preselect,
                detail: item.detail,
                documentation: item.documentation && item.documentation.value ? item.documentation.value : item.documentation,
              };
            });

            return { suggestions: suggestions };
          } catch (_) {
            return { suggestions: [] };
          }
        }
      });
    }

    function registerHover(langId) {
      monaco.languages.registerHoverProvider(langId, {
        provideHover: async function (_model, position) {
          if (!initialized) return null;
          try {
            const result = await request('textDocument/hover', {
              textDocument: { uri: uri },
              position: fromMonacoPosition(position),
            });
            if (!result || !result.contents) return null;
            const value = Array.isArray(result.contents)
              ? result.contents.map(function (c) { return c.value || c; }).join('\n\n')
              : (result.contents.value || result.contents);
            return { range: result.range ? toMonacoRange(result.range) : undefined, contents: [{ value: value }] };
          } catch (_) {
            return null;
          }
        }
      });
    }

    registerCompletion('luau');
    registerHover('luau');

    handshake();

    return { onServerMessage: onServerMessage };
  }

  window.RMLLsp = { start: start, version: LSP_CLIENT_VERSION };
})();
