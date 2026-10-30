"""Compile the actual editor prefix method against a minimal native test receiver."""
from pathlib import Path
import os
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
source = (root / "Sources/TSTextView.m").read_text()
start = source.index("- (NSRange)automaticCommandRange")
end = source.index("- (BOOL)insertAutomaticEnvironmentEnd", start)
template = (root / "Tests/CommandPrefixTests.m.in").read_text()
env = dict(os.environ)
env.setdefault("DEVELOPER_DIR", "/Applications/Xcode.app/Contents/Developer")
sdk = subprocess.check_output(["xcrun", "--sdk", "macosx", "--show-sdk-path"], env=env, text=True).strip()
with tempfile.TemporaryDirectory() as folder:
    path = Path(folder)
    (path / "test.m").write_text(template.replace("/* COMMAND_RANGE_IMPLEMENTATION */", source[start:end]))
    subprocess.run(["xcrun", "clang", "-isysroot", sdk, "-fobjc-arc", "-framework", "Cocoa", str(path / "test.m"), "-o", str(path / "test")], env=env, check=True)
    subprocess.run([str(path / "test")], check=True)

# Compile the same pure transformation used by Return in the source editor.
with tempfile.TemporaryDirectory() as folder:
    binary = str(Path(folder) / "environment-tests")
    subprocess.run(["xcrun", "clang", "-isysroot", sdk, "-fobjc-arc", "-framework", "Foundation", str(root / "Tests/EnvironmentCompletionTests.m"), "-o", binary], env=env, check=True)
    subprocess.run([binary], check=True)

with tempfile.TemporaryDirectory() as folder:
    binary = str(Path(folder) / "argument-tests")
    subprocess.run(["xcrun", "clang", "-isysroot", sdk, "-fobjc-arc", "-framework", "Foundation", str(root / "Tests/ArgumentCompletionTests.m"), "-o", binary], env=env, check=True)
    subprocess.run([binary], check=True)

with tempfile.TemporaryDirectory() as folder:
    binary = str(Path(folder) / "build-path-tests")
    subprocess.run(["xcrun", "clang", "-isysroot", sdk, "-fobjc-arc", "-framework", "Foundation", str(root / "Tests/BuildPathTests.m"), "-o", binary], env=env, check=True)
    subprocess.run([binary], check=True)

with tempfile.TemporaryDirectory() as folder:
    binary = str(Path(folder) / "automatic-policy-tests")
    subprocess.run(["xcrun", "clang", "-isysroot", sdk, "-fobjc-arc", "-framework", "Foundation", str(root / "Tests/AutomaticBuildPolicyTests.m"), "-o", binary], env=env, check=True)
    subprocess.run([binary], check=True)
