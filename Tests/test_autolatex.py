"""Integration checks using TeX Live; all generated files stay in temporary projects."""
from pathlib import Path
import os
import subprocess
import tempfile

engine = Path(__file__).resolve().parents[1] / "Resources/TeXShop/Engines/AutoLaTeX.engine"
env = dict(os.environ)
env["PATH"] = "/Library/TeX/texbin:" + env["PATH"]
base = Path(tempfile.mkdtemp(prefix="texshop-auto-tests-")).resolve()


def invoke(folder, detect=False):
    return subprocess.run([str(engine), *(["--detect"] if detect else []), str(folder / "sample.tex"), "", "/Library/TeX/texbin"], cwd=folder, env=env, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=120)


for index, (source, expected) in enumerate([
    (r"\documentclass{article}", "pdflatex"),
    (r"\usepackage{fontspec}", "xelatex"),
    (r"\usepackage{amsmath,fontspec}", "xelatex"),
    (r"\documentclass{ctexart}", "xelatex"),
    (r"\directlua{tex.print('test')}", "lualatex"),
    ("% !TEX TS-engine = lualatex\n\\usepackage{fontspec}", "lualatex"),
    ("% \\usepackage{fontspec}\n\\documentclass{article}", "pdflatex"),
    (r"\input{preamble}", "xelatex"),
]):
    folder = base / f"detect-{index}"
    folder.mkdir()
    (folder / "sample.tex").write_text(source)
    (folder / "preamble.tex").write_text(r"\usepackage{fontspec}\input{preamble}")
    result = invoke(folder, True)
    assert result.returncode == 0 and f"selected {expected};" in result.stdout, result.stdout
print("8 engine detection checks passed", flush=True)

for name, preamble, bibliography in [
    ("pdf with spaces", "", "bibtex"),
    ("xe", r"\usepackage{fontspec}", "bibtex"),
    ("lua", "% !TEX TS-engine = lualatex\n", "none"),
    ("biber", r"\usepackage[backend=biber]{biblatex}\addbibresource{references.bib}", "biber"),
]:
    folder = base / name
    folder.mkdir()
    bib = r"\bibliographystyle{plain}\bibliography{references}" if bibliography == "bibtex" else r"\printbibliography" if bibliography == "biber" else ""
    citation = r"\cite{knuth}" if bibliography != "none" else ""
    source = preamble if name == "lua" else ""
    source += "\\documentclass{article}\n" + (preamble if name != "lua" else "") + "\n\\begin{document}\n\\section{Intro}\\label{sec:intro}\nSee Section~\\ref{sec:intro}. " + citation + "\n" + bib + "\n\\end{document}\n"
    (folder / "sample.tex").write_text(source)
    (folder / "references.bib").write_text("@book{knuth, author={Donald Knuth}, title={The TeXbook}, year={1984}, publisher={Addison-Wesley}}\n")
    result = invoke(folder)
    (folder / ".texshop-build" / "test-output.txt").write_text(result.stdout)
    assert result.returncode == 0, str(folder) + "\n" + result.stdout[-3000:]
    assert {p.name for p in folder.iterdir()} == {"sample.tex", "references.bib", "sample.pdf", ".texshop-build"}, str(folder)
    build = folder / ".texshop-build" / "sample"
    log = (build / "sample.log").read_text(errors="replace")
    assert "undefined references" not in log and "There were undefined" not in log, str(folder)
    assert (build / "sample.synctex.gz").exists()
    if bibliography != "none":
        assert (build / "sample.bbl").exists()
    if name == "pdf with spaces":
        sync = subprocess.run(["synctex", "view", "-i", f"5:1:{folder / 'sample.tex'}", "-o", str(build / "sample.pdf")], env=env, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        assert sync.returncode == 0 and "Page:" in sync.stdout, sync.stdout
        old_pdf = (folder / "sample.pdf").read_bytes()
        (folder / "sample.tex").write_text(source.replace("See Section", r"\undefinedcommand See Section"))
        failed = invoke(folder)
        assert failed.returncode != 0 and (folder / "sample.pdf").read_bytes() == old_pdf, "failed build replaced published PDF"
    print(f"{name}: build, clean source folder and retained SyncTeX passed", flush=True)
print(f"Evidence: {base}", flush=True)
