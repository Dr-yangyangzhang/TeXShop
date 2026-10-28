# Native command completion: first milestone

The editor offers commands automatically after a backslash and two ASCII letters,
with a 120 ms typing delay. Suggestions come from the existing command completion
list, cached per source view. A nonactivating AppKit panel keeps keyboard focus in
the editor. There is no language server, network access, or additional dependency.

Arrow keys select a suggestion. Return or Tab accepts it. Escape dismisses without
changing text. Acceptance puts the caret inside the first empty brace argument.
Backspace updates the suggestions. Cursor changes, scrolling through the editor,
and losing window focus dismiss the panel. Legacy manual completion remains available.

This milestone lists direct command templates, including starred and optional
argument variants. Aliases and environment expansion templates remain handled by
legacy completion. Internal template markers and placeholder text are removed from
popup entries. Automatic acceptance does not add template trailing newlines.
Reference/citation argument lookup, package/project indexing, custom macro discovery,
and verbatim-environment parsing are outside this milestone.

## Build with Xcode 27

From the repository directory:

```sh
DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer xcodebuild \
  -project TeXShop.xcodeproj -scheme TeXShop -configuration Debug \
  -derivedDataPath /private/tmp/texshop-autocomplete-build \
  CODE_SIGNING_ALLOWED=NO MACOSX_DEPLOYMENT_TARGET=12.0 \
  GCC_VERSION=com.apple.compilers.llvm.clang.1_0 build
open /private/tmp/texshop-autocomplete-build/Build/Products/Debug/TeXShop.app
python3 Tests/test_command_prefix.py
```

The deployment and compiler overrides apply only to this build. Xcode 27 rejects
the original 10.13 deployment target and the obsolete GCC compiler selection.
A separate build cleanup removes a resource-copy entry for an absent Manual.log.
This is a development build; release signing and older macOS compatibility are not verified.

Quit another running TeXShop before opening this development build to avoid testing
the installed app by mistake. Do not replace the installed app yet.

## Manual acceptance checklist

1. In a fresh TeX source window, type `\sec` one character at a time. After a short
   pause, expect `\section{}`, `\section*{}` and `\section[]{}`. No command should
   insert merely because the list appears.
2. Use arrows and Return to accept `\section{}`. Type a title; it belongs inside
   the braces. Undo the title, then undo completion: expect the original `\sec`.
   Redo should restore the completion.
3. Repeat using Tab for a starred command and mouse selection for another entry.
4. Type more letters and backspace: the list should update. Escape must leave the
   prefix unchanged. Move the caret or switch windows: the list should disappear.
5. Check `% \sec`, `\\sec`, ordinary prose, and active Chinese IME composition:
   no command popup. A new line after a comment and `\% \sec` should allow it.
6. Test two documents and split source views. Suggestions must stay with the active
   caret. Repeat in a large file and check typing responsiveness.
7. Exercise your existing manual completion shortcut, label completion, BibDesk
   citation completion if configured, and normal typesetting.

The build, 13 native prefix checks, automatic section popup, arrow/Return acceptance,
brace caret placement, completion undo, and Escape dismissal were checked during
development. Mouse/Tab acceptance, redo, real IME input, split views, large-file
performance, and BibDesk regression checks still need interactive acceptance testing.

## Disable or roll back

Automatic suggestions default on. To disable, quit TeXShop and run:

```sh
defaults write TeXShop DisableAutomaticCommandCompletion -bool YES
```

To re-enable, use `-bool NO`. This is an advanced defaults setting, not a new
Preferences control. The feature is isolated on its branch; do not merge until the
manual checks pass. Reverting the feature commit removes the popup independently
of the missing-resource build cleanup.
