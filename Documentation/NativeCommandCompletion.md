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
Common argument values and current-document labels are covered below. Citation keys,
installed package/project indexing, custom macro discovery and verbatim parsing
remain outside this milestone.

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

## Return after an environment opener

At the end of a line containing `\begin{document}`, press Return. The editor inserts:

```tex
\begin{document}

\end{document}
```

The caret stays on the blank line. Leading tabs/spaces are preserved. Named and
starred environments and simple optional/required arguments are supported. An
existing matching end is respected, including nested environments of the same name;
commented-out or escaped ends do not suppress completion. Lines containing other
source text, comments, active IME composition, or selected text use ordinary Return.
Parsing verbatim content and arbitrary TeX macro expansion is outside this feature.

Validate with document, itemize, align*, and an indented opener. Undo should restore
the opener; pressing Return before an existing matching end must not duplicate it.
Disable separately with `defaults write TeXShop DisableAutomaticEnvironmentCompletion -bool YES`.
The native test runner now checks 13 command-prefix and 19 environment cases.
Interactive Return insertion and caret placement have now been verified in the rebuilt app.

## Automatic multi-pass typesetting

TeXShop already supports this through its latexmk engines. Select **pdflatexmk**
in the program dropdown for pdfLaTeX, or **xelatexmk** for XeLaTeX, then Typeset.
This branch promotes the existing xelatexmk engine from Inactive to the active
bundled engines and installs it on launch only if an active engine with that name
is absent. Customized active scripts are preserved.

For a persistent per-document choice, add one of these as the first line:

```tex
% !TEX TS-program = pdflatexmk
```

or

```tex
% !TEX TS-program = xelatexmk
```

Latexmk runs the selected LaTeX engine, invokes BibTeX when needed, and repeats
LaTeX until citations and cross-references settle. It may skip unnecessary passes;
this is not a fixed four-command sequence. Choosing plain LaTeX or XeLaTeX still
performs its usual single run. TeXShop's existing root-file directives continue to
control multi-file projects.

Both actual bundled scripts were run against a fresh article with a BibTeX
bibliography and a section cross-reference. Both produced a PDF and .bbl with
resolved references in the final log. To validate in the UI, restart the development
app, select each engine on a small citation document, press Typeset once, and
confirm that the citation and section reference are resolved. A normal TeX Live
installation (including latexmk, BibTeX and the selected engine) is required.

## Contextual argument suggestions and Greek commands

The same popup now offers common values inside `\documentclass{}` (including
article/report/book), `\begin{}` and `\end{}`, `\usepackage{}` and
`\RequirePackage{}`, and `\bibliographystyle{}`. Package lists complete the token
after the last comma. Reference commands offer labels from the current document,
excluding commented labels. Optional command arguments are recognized. Accepting a
suggestion preserves the surrounding command and braces, including when editing
in the middle of a token. These are curated common catalogs, not a claim that a
particular class/package is installed. Citation keys and project-wide indexes are
not implemented yet.

Greek commands are present directly in the command catalog: typing `\al` offers
`\alpha`; shorthand aliases in the legacy completion file are no longer the only
way to reach these symbols. Ordinary prose `al` does not trigger LaTeX commands.

## Automatic workflow and opt-in Build on Save

Choose **Typeset > Use Automatic Workflow**, or **AutoLaTeX** in the program dropdown.
Then the normal Typeset button/shortcut builds using latexmk. No fixed four-pass
recipe is necessary: latexmk runs the selected engine, BibTeX/Biber and repeat passes
as dependencies require. A simple document may need fewer passes.

**Typeset > Build on Save** enables Command-S-triggered compilation and selects AutoLaTeX. It defaults off; once enabled, the setting persists across document windows and
application restarts. This is an explicit Save feature, not a continuous background watcher
or a trigger for system autosaves. Failed/cancelled saves do not build. Rapid saves
coalesce; queued saves wait for an existing job rather than repeatedly killing it.
Manual Typeset supersedes a queued save-triggered build. Disabling the toggle cancels
queued work; an already-running build is stopped using the normal Abort command.

AutoLaTeX defaults to pdfLaTeX, chooses XeLaTeX for recognized fontspec/xeCJK/ctex
requirements, and LuaLaTeX for recognized Lua commands/packages. It reads literal
input/include/subfile dependencies (bounded to 100 files and depth 16). Engine choice
is a heuristic, not a TeX interpreter. It cannot infer arbitrary macros or every
package's requirements. For an explicit choice within AutoLaTeX, add:

```tex
% !TEX TS-engine = xelatex
```

Allowed values are pdflatex, xelatex and lualatex. In AutoLaTeX mode, standard TS-program/program compiler directives (pdflatex,
xelatex, lualatex and their latexmk variants) select the compiler while retaining
latexmk and the hidden build directory. Custom engine directives retain their normal
precedence and can use a different output layout. Explicit legacy engine selections
remain available.

## Clean source folder and retained build files

AutoLaTeX writes build artifacts to `.texshop-build/<document-name>/` beside the
root source. On success it atomically publishes only the PDF beside the source.
The hidden folder retains logs, auxiliary files, latexmk's dependency database and
SyncTeX data for incremental builds and source/PDF synchronization. Intermediate
files are still generated and saved; they are isolated rather than deleted after
every compile. Existing auxiliary files are never removed or moved automatically.
Your source assets, bibliography and included files remain where you put them.

TeXShop's normal SyncTeX parser and log viewer can find the retained data. When
switching back to a conventional engine, newer conventional files take precedence.
Legacy custom SyncTeX parsers and unusually configured projects need further testing.
Documents relying on custom jobnames/output directories or tools that require
auxiliary files in the root should continue using a dedicated existing engine.

## Latest validation

`python3 Tests/test_command_prefix.py` passes 13 prefix, 19 environment, 16 argument
and 5 build-path checks, plus 16 automatic-build policy checks. `python3 Tests/test_autolatex.py` runs 11 engine-detection
checks and actual pdfLaTeX/BibTeX, XeLaTeX/BibTeX, LuaLaTeX and Biber builds. It checks
clean source folders, SyncTeX data, paths with spaces and preservation of the last
PDF after a failed build. TeX Live's font caches must be writable for LuaLaTeX.
The final Xcode Debug build uses the overrides above.

Interactive validation confirmed article completion with Tab, Greek alpha suggestions,
Return-triggered environment closing with correct caret placement, the automatic
workflow menu, and Command-S producing and opening a PDF after enabling Build on
Save. Further acceptance checks: repeated saves while compiling, cancelling a Save
panel, root/child projects, both directions of SyncTeX in complex projects, BibDesk,
real IME composition, large files and split source views.

## October 30 fixes and testing the correct application

Ordinary LaTeX documents now select AutoLaTeX when opened, instead of silently
returning to single-pass LaTeX. Explicit Plain TeX and custom default engine choices
are preserved. Build on Save remains opt-in, but its setting now persists. Enable
it once in Typeset > Build on Save; reopening the app should retain the checkmark.
Standard compiler directives no longer bypass the automatic recipe in AutoLaTeX mode.

The installer upgrades an exact copy of the previous bundled AutoLaTeX script;
customized scripts are preserved. This avoids leaving existing installations with
an outdated engine after rebuilding the app.

The installed `/Applications/TeXShop.app` is not replaced by a development build.
Close other TeXShop instances before testing the rebuilt preview and check that the
program dropdown says AutoLaTeX. Files produced by older builds are not deleted:
validate output placement in a fresh folder to distinguish new artifacts from old
ones. Screen sharing is not needed for automated tests or Git publication.
