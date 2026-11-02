"""Exercise the actual editor pairing block in an NSTextView test receiver."""
from pathlib import Path
import os
import subprocess
import tempfile
root = Path(__file__).resolve().parents[1]
source = (root / "Sources/TSTextView.m").read_text()
start = source.index("- (void)insertText:(id)aString")
end = source.index("\n\t// AutoCompletion", start)
method = source[start:end] + "\n    [super insertText:aString];\n}\n"
template = (root / "Tests/DelimiterEditorTests.m.in").read_text()
env = dict(os.environ)
env.setdefault("DEVELOPER_DIR", "/Applications/Xcode.app/Contents/Developer")
sdk = subprocess.check_output(["xcrun", "--sdk", "macosx", "--show-sdk-path"], env=env, text=True).strip()
with tempfile.TemporaryDirectory() as folder:
    path = Path(folder)
    (path / "test.m").write_text(template.replace("/* ACTUAL_EDITOR_INSERTION */", method))
    subprocess.run(["xcrun", "clang", "-isysroot", sdk, "-fobjc-arc", "-framework", "Cocoa", "-I", str(root / "Sources"), str(path / "test.m"), "-o", str(path / "test")], env=env, check=True)
    subprocess.run([str(path / "test")], env=env, check=True)
