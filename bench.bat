@echo off

SET INPUT=files\node_modules.tar
SET NAMES=-n "My.LZSS" -n "My.ROLZ" -n "LZSS.Brute" -n "LZSS.List" -n "LZSS.Hash" -n "LZSS.Tree" -n "LZSS.Kmp"

hyperfine -w 2 %NAMES% --export-markdown stats.md ^
  "compression e lzss %INPUT% -o out\node.lzss"^
  "compression e rolz %INPUT% -o out\node.rolz"^
  "lzss-brute -c -i %INPUT% -o out\node.lzss.brute"^
  "lzss-list -c -i %INPUT% -o out\node.lzss.list"^
  "lzss-hash -c -i %INPUT% -o out\node.lzss.hash"^
  "lzss-tree -c -i %INPUT% -o out\node.lzss.tree"^
  "lzss-kmp -c -i %INPUT% -o out\node.lzss.kmp"

echo Done!